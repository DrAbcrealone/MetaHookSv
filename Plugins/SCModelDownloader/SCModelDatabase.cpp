#include <metahook.h>
#include "plugins.h"

#include "cvardef.h"

#include "UtilHTTPClient.h"
#include "UtilAssetsIntegrity.h"
#include "SCModelDatabase.h"

#include <string>
#include <unordered_map>
#include <algorithm>
#include <format>

#include <rapidjson/document.h>

#pragma region Marco Def
static constexpr void LOG_DEVELOPER(const char* str) {
	std::string temp = "[SCModelDownloader] ";
	temp += str;
	temp += "\n";
	gEngfuncs.Con_DPrintf(temp.c_str());
}
template <typename... Args>
static constexpr void LOG_DEVELOPER(const char* str, Args&&... args) {
	std::string_view fmt = str;
	std::string msg = std::vformat(fmt, std::make_format_args(args...));
	std::string temp = "[SCModelDownloader] ";
	temp += msg;
	temp += "\n";
	gEngfuncs.Con_DPrintf(temp.c_str());
}

constexpr int DATA_REPO_COUNT = 32;
#pragma endregion

#pragma region External
extern bool SCModel_ShouldDownloadLatest();
extern void SCModel_ReloadModel(const char* name);
extern cvar_t* scmodel_maxretry;
#pragma endregion

#pragma region Struct Def
using scmodel_t = struct scmodel_s
{
	int size;
	int flags;
	int polys;
};

#pragma endregion

#pragma region Local Var
static std::unordered_map<std::string, scmodel_t> g_dicDatabase;
#pragma endregion

static inline int SCModel_Hash(const std::string& name)
{
	int hash = 0;

	for (size_t i = 0; i < name.length(); i++) {
		char ch = (char)name[i];
		hash = ((hash << 5) - hash) + ch;
		hash = hash % 15485863; // prevent hash ever increasing beyond 31 bits
	}
	return hash;
}

class BaseQueryTask {
public:
	int m_iRetry = 0;
	virtual void StartQuery() = 0;
};

class CSCModelMetadataQueryTask : public BaseQueryTask {
public:
	void PraseJson(const char* data, size_t size) {
		if (g_dicDatabase.size() > 0)
			return;
		if (size == 0)
			return;
		rapidjson::Document doc;
		doc.Parse(data, size);
		if (doc.HasParseError()){
			LOG_DEVELOPER("Failed to parse database response!");
			return;
		}
		for (auto itor = doc.MemberBegin(); itor != doc.MemberEnd(); ++itor){
			std::string name = itor->name.GetString();
			auto obj = itor->value.GetObj();
			scmodel_t m = { 0 };
			auto m_size = obj.FindMember("size");
			if (m_size != obj.MemberEnd() && m_size->value.IsInt())
				m.size = m_size->value.GetInt();
			auto m_flags = obj.FindMember("flags");
			if (m_flags != obj.MemberEnd() && m_flags->value.IsInt())
				m.flags = m_flags->value.GetInt();
			auto m_polys = obj.FindMember("polys");
			if (m_polys != obj.MemberEnd() && m_polys->value.IsInt())
				m.polys = m_polys->value.GetInt();
			g_dicDatabase[name] = m;
		}
	}
	virtual void StartQuery() override {
		CHTTPRequest::Create("https://raw.githubusercontent.com/wootguy/scmodels/master/database/models.json")->
			OnResponed([](CHTTPPayload* payload, void* user) {
				CSCModelMetadataQueryTask* pThis = reinterpret_cast<CSCModelMetadataQueryTask*>(user);
				pThis->PraseJson(payload->m_pData, payload->m_iSize);
			}, this)->
			OnFailed([](void* user) {
				CSCModelMetadataQueryTask* pThis = reinterpret_cast<CSCModelMetadataQueryTask*>(user);
				pThis->m_iRetry++;
				if (pThis->m_iRetry >= scmodel_maxretry->value) {
					LOG_DEVELOPER("Attempts to fetch the model Metadata have reached the maximum number of retries and will be aborted.");
					return;
				}
				else {
					LOG_DEVELOPER("Attempts to fetch the model Metadata failed, started retry. {}/{}", pThis->m_iRetry, (int)scmodel_maxretry->value);
					pThis->StartQuery();
				}
			}, this)->
			Start();
	}
};

enum DOWNLOAD_TASK_TYPE
{
	INVALID,
	MDL,
	BMP
};
class CSCMDLDownlaodTask : public BaseQueryTask {
public:
	std::string m_szUrl = "";
	std::string m_szSaveFileName = "";
	std::string m_szModelName = "";

	DOWNLOAD_TASK_TYPE m_iType = INVALID;

	CSCMDLDownlaodTask* m_pNext = nullptr;

	void SaveToDisk(const char* data, size_t size) {
		LOG_DEVELOPER("File \"{}\" acquired!", m_szModelName);
		switch (m_iType)
		{
		case MDL: {
			if (UtilAssetsIntegrityCheckReason::OK != UtilAssetsIntegrity()->CheckStudioModel(data, size, NULL)) {
				LOG_DEVELOPER("File \"{}\" is corrupted!\n", m_szModelName);
				return;
			}
			break;
		}
		case BMP: {
			if (UtilAssetsIntegrityCheckReason::OK != UtilAssetsIntegrity()->Check8bitBMP(data, size, NULL)) {
				LOG_DEVELOPER("File \"{}\" is corrupted!\n", m_szModelName);
				return;
			}
		}
		case INVALID:
		default: {
			LOG_DEVELOPER("Invalid Request in \"{}\"!\n", m_szModelName);
			return;
		}
		}

		FILESYSTEM_ANY_CREATEDIR("models", "GAMEDOWNLOAD");
		FILESYSTEM_ANY_CREATEDIR("models/player", "GAMEDOWNLOAD");

		std::string filePathDir = std::format("models/player/{0}", m_szModelName);
		FILESYSTEM_ANY_CREATEDIR(filePathDir.c_str(), "GAMEDOWNLOAD");

		std::string filePath = std::format("models/player/{0}/{1}", m_szModelName, m_szSaveFileName);
		auto FileHandle = FILESYSTEM_ANY_OPEN(filePath.c_str(), "wb", "GAMEDOWNLOAD");

		if (FileHandle)
		{
			FILESYSTEM_ANY_WRITE(data, size, FileHandle);
			FILESYSTEM_ANY_CLOSE(FileHandle);
		}
	}

	virtual void StartQuery() override {
		CHTTPRequest::Create(m_szUrl.c_str())->
			OnFailed([](void* user) {
			CSCMDLDownlaodTask* pThis = reinterpret_cast<CSCMDLDownlaodTask*>(user);
			pThis->m_iRetry++;
			if (pThis->m_iRetry >= scmodel_maxretry->value) {
				LOG_DEVELOPER("Attempts to fetch model ({}) have reached the maximum number of retries and will be aborted.", pThis->m_szModelName);
				return;
			}
			else {
				LOG_DEVELOPER("Attempts to fetch model ({}) Metadata failed, started retry. {}/{}", pThis->m_szModelName, pThis->m_iRetry, (int)scmodel_maxretry->value);
				pThis->StartQuery();
			}
				}, this)->
			OnResponed([](CHTTPPayload* payload, void* user) {
					CSCMDLDownlaodTask* pThis = reinterpret_cast<CSCMDLDownlaodTask*>(user);
					if (pThis->m_pNext != nullptr)
						pThis->m_pNext->StartQuery();
				}, this)->
					ContinueWith([](CHTTPRespond* payload, void* user) {
					CSCMDLDownlaodTask* pThis = reinterpret_cast<CSCMDLDownlaodTask*>(user);
					pThis->SaveToDisk(payload->m_pPayload->m_pData, payload->m_pPayload->m_iSize);
						}, this)->
					Start();
	}
};

class CSCModelJsonQueryTask : public BaseQueryTask {
public:
	int m_iRepoId = 0;
	std::string m_szModel = "";
	std::string m_szUrl = "";
	std::string m_szLowerName = "";
	bool m_bHasTModel = false;

	std::string m_szNetworkFileNameBase = "";

	bool IsAllRequiredFilesPresent() {
		std::string mdl = std::format("models/player/{0}/{0}.mdl", m_szModel);

		if (!FILESYSTEM_ANY_FILEEXISTS(mdl.c_str()))
			return false;

		if (m_bHasTModel) {
			std::string Tmdl = std::format("models/player/{0}/{0}T.mdl", m_szModel);
			std::string tmdl = std::format("models/player/{0}/{0}t.mdl", m_szModel);

			if (!FILESYSTEM_ANY_FILEEXISTS(Tmdl.c_str()) && !FILESYSTEM_ANY_FILEEXISTS(tmdl.c_str()))
				return false;
		}

		return true;
	}

	void PraseJson(const char* data, size_t size) {
		rapidjson::Document doc;
		doc.Parse(data, size);

		if (doc.HasParseError()){
			LOG_DEVELOPER("Failed to parse model json response!");
			return;
		}

		auto obj = doc.GetObj();
		bool bHasTModel = false;
		auto json_t_model = doc.FindMember("t_model");
		if (json_t_model != doc.MemberEnd() && json_t_model->value.IsBool())
			bHasTModel = json_t_model->value.GetBool();

		std::string networkFileNameBase = m_szLowerName;
		auto json_name = doc.FindMember("name");
		if (json_name != doc.MemberEnd() && json_name->value.IsString()){
			std::string name = json_name->value.GetString();
			if (name.size() > 4 &&
				name[name.size() - 4] == '.' &&
				name[name.size() - 3] == 'm' &&
				name[name.size() - 2] == 'd' &&
				name[name.size() - 1] == 'l')
				name = name.erase(name.size() - 4, 4);

			if (0 == stricmp(networkFileNameBase.c_str(), name.c_str()))
				networkFileNameBase = name;
		}

		m_szNetworkFileNameBase = networkFileNameBase;
		m_bHasTModel = bHasTModel;

		LOG_DEVELOPER(" Json for model \"{}\" acquired!", m_szModel);

		//Start download mdl
		if (!m_szNetworkFileNameBase.empty())
		{
			//MDL

			//T MDL

			//BMP
		}
	}
	virtual void StartQuery() override{
		m_szLowerName = m_szModel;
		std::transform(m_szLowerName.begin(), m_szLowerName.end(), m_szLowerName.begin(), std::tolower);
		m_iRepoId = SCModel_Hash(m_szLowerName) % DATA_REPO_COUNT;
		m_szUrl = std::format("https://wootdata.github.io/scmodels_data_{0}/models/player/{1}/{1}.json", m_iRepoId, m_szLowerName);

		CHTTPRequest::Create(m_szUrl.c_str())->
			OnFailed([](void* user) {
				CSCModelJsonQueryTask* pThis = reinterpret_cast<CSCModelJsonQueryTask*>(user);
				pThis->m_iRetry++;
				if (pThis->m_iRetry >= scmodel_maxretry->value) {
					LOG_DEVELOPER("Attempts to fetch model ({}) have reached the maximum number of retries and will be aborted.", pThis->m_szModel);
					return;
				}
				else {
					LOG_DEVELOPER("Attempts to fetch model ({}) Metadata failed, started retry. {}/{}", pThis->m_szModel, pThis->m_iRetry, (int)scmodel_maxretry->value);
					pThis->StartQuery();
				}
			}, this)->
			OnResponed([](CHTTPPayload* payload, void* user) {
				CSCModelJsonQueryTask* pThis = reinterpret_cast<CSCModelJsonQueryTask*>(user);
				pThis->PraseJson(payload->m_pData, payload->m_iSize);
			}, this)->
			Start();
	}
};




ISCModelDatabase* SCModelDatabase()
{
	return &s_SCModelDatabase;
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CSCModelDatabase, ISCModelDatabase, SCMODEL_DATABASE_INTERFACE_VERSION, s_SCModelDatabase)