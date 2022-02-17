#include <metahook.h>
#include <studio.h>
#include <r_studioint.h>
#include <capstone.h>
#include "cl_entity.h"
#include "com_model.h"
#include "triangleapi.h"
#include "cvardef.h"
#include "exportfuncs.h"
#include "entity_types.h"
#include "parsemsg.h"
#include "privatehook.h"
#include "plugins.h"
#include <steam_api.h>
#include <functional>
#include <set>
#include <unordered_map>
#include <rapidjson/document.h>

cvar_t *scmodel_autodownload = NULL;

typedef struct
{
	char		name[260];
	char		modelname[260];
	model_t		*model;
} player_model_t;

player_model_t (*DM_PlayerState)[MAX_CLIENTS];

cl_enginefunc_t gEngfuncs;
engine_studio_api_t IEngineStudio;
r_studio_interface_t **gpStudioInterface;

class CHttpRequest;

using HttpCallback = std::function<void(CHttpRequest *pthis, HTTPRequestCompleted_t *pResult, bool bError) > ;

class CHttpRequest
{
public:
	CHttpRequest(EHTTPMethod eMethod, const char *szUrl, const HttpCallback &callback) : m_Callback(callback)
	{
		m_RequestHandle = SteamHTTP()->CreateHTTPRequest(eMethod, szUrl);
		m_bFinished = false;
	}

	~CHttpRequest()
	{
		m_CallResult.Cancel();

		if (m_RequestHandle != INVALID_HTTPREQUEST_HANDLE)
		{
			SteamHTTP()->ReleaseHTTPRequest(m_RequestHandle);
			m_RequestHandle = INVALID_HTTPREQUEST_HANDLE;
		}
	}

public:

	bool IsFinished()
	{
		return m_bFinished;
	}

	bool IsValid()
	{
		if (m_RequestHandle == INVALID_HTTPREQUEST_HANDLE)
			return false;

		return true;
	}

	bool GetResponseBodySize(uint32 *bodySize)
	{
		return SteamHTTP()->GetHTTPResponseBodySize(m_RequestHandle, bodySize);
	}

	bool GetResponseBody(uint8 *buffer, uint32 maxBuffer)
	{
		return SteamHTTP()->GetHTTPResponseBodyData(m_RequestHandle, buffer, maxBuffer);
	}

	bool Send(void)
	{
		SteamAPICall_t apiCall;
		bool bRet = SteamHTTP()->SendHTTPRequest(m_RequestHandle, &apiCall);

		if (bRet)
		{
			m_CallResult.Set(apiCall, this, &CHttpRequest::OnHttpRequestCompleted);
		}

		return bRet;
	}

private:
	void OnHttpRequestCompleted(HTTPRequestCompleted_t *pResult, bool bError)
	{
		m_Callback(this, pResult, bError);

		m_bFinished = true;

		if (m_RequestHandle != INVALID_HTTPREQUEST_HANDLE)
		{
			SteamHTTP()->ReleaseHTTPRequest(m_RequestHandle);
			m_RequestHandle = INVALID_HTTPREQUEST_HANDLE;
		}
	}

private:
	HTTPRequestHandle m_RequestHandle;

	CCallResult<CHttpRequest, HTTPRequestCompleted_t> m_CallResult;

	HttpCallback m_Callback;

	bool m_bFinished;
};

typedef struct scmodel_s
{
	int size;
	int flags;
	int polys;
}scmodel_t;

int SCModel_Hash(const std::string &name)
{
	int hash = 0;

	for (size_t i = 0; i < name.length(); i++) {
		char ch = (char)name[i];
		hash = ((hash << 5) - hash) + ch;
		hash = hash % 15485863; // prevent hash ever increasing beyond 31 bits
	}
	return hash;
}

class CDownloadList
{
public:
	CDownloadList(const std::string &n) : name(n)
	{
		repoId = SCModel_Hash(n) % 32;
	}

public:
	int repoId;
	std::string name;
	std::vector<CHttpRequest *> requests;
};

std::unordered_map<std::string, scmodel_t> g_scmodels;

std::unordered_map<std::string, CDownloadList *> g_download_models;

CHttpRequest *g_scmodel_json = NULL;

void SCModel_ReloadModel(const std::string &name)
{
	//Reload models for those players
	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		if (!strcmp((*DM_PlayerState)[i].name, name.c_str()))
		{
			(*DM_PlayerState)[i].name[0] = 0;
		}
	}
}

bool SCModel_IsAllRequestFinished(CDownloadList *list, CHttpRequest *req)
{
	for (auto r : list->requests)
	{
		if (r != req && !r->IsFinished())
			return false;
	}

	return true;
}

void SCModel_ModelFileAcquired(CDownloadList *list, CHttpRequest *req)
{
	if (SCModel_IsAllRequestFinished(list, req))
	{
		SCModel_ReloadModel(list->name);
	}
}

void SCModel_RequestForModelFile(CDownloadList *list, const char *url, const char *localpath)
{
	std::string local(localpath);

	auto request = new CHttpRequest(
		EHTTPMethod::k_EHTTPMethodGET,
		url,
		[list, local](CHttpRequest *pthis, HTTPRequestCompleted_t *pResult, bool bError) {
		if (!bError && pResult->m_eStatusCode == 200)
		{
			uint32_t bodySize = 0;
			if (pthis->GetResponseBodySize(&bodySize))
			{
				PUCHAR buffer = (PUCHAR)malloc(bodySize);
				if (pthis->GetResponseBody(buffer, bodySize))
				{
					auto FileHandle = g_pFileSystem->Open(local.c_str(), "wb", "GAMEDOWNLOAD");
					if (FileHandle)
					{
						g_pFileSystem->Write(buffer, bodySize, FileHandle);
						g_pFileSystem->Close(FileHandle);
					}

					SCModel_ModelFileAcquired(list, pthis);
				}
				else
				{
					gEngfuncs.Con_Printf("SCModelDownloader: Failed to GetHTTPResponseBodyData!\n");
				}
				free(buffer);
			}
			else
			{
				gEngfuncs.Con_Printf("SCModelDownloader: Failed to GetHTTPResponseBodySize!\n");
			}
		}
		else
		{
			gEngfuncs.Con_Printf("SCModelDownloader: Failed the http response for model file!\n");
		}
	});

	if (!request->IsValid())
	{
		gEngfuncs.Con_Printf("SCModelDownloader: Failed to create http request for model file!\n");
		delete request;
		request = NULL;
		return;
	}

	if (!request->Send())
	{
		gEngfuncs.Con_Printf("SCModelDownloader: Failed to send http request for model file!\n");
		delete request;
		request = NULL;
		return;
	}

	list->requests.emplace_back(request);
}

void SCModel_ModelJsonAcquired(CDownloadList *list, bool bHasTModel)
{
	g_pFileSystem->CreateDirHierarchy("models", "GAMEDOWNLOAD");
	g_pFileSystem->CreateDirHierarchy("models/player", "GAMEDOWNLOAD");

	char localdir[1024];
	sprintf_s(localdir, sizeof(localdir), "models/player/%s", list->name.c_str());
	g_pFileSystem->CreateDirHierarchy(localdir, "GAMEDOWNLOAD");

	if (1)
	{
		char url[1024];
		sprintf_s(url, sizeof(url), "https://wootdata.github.io/scmodels_data_%d/models/player/%s/%s.mdl", list->repoId, list->name.c_str(), list->name.c_str());

		char localpath[1024];
		sprintf_s(localpath, sizeof(localpath), "models/player/%s/%s.mdl", list->name.c_str(), list->name.c_str());

		SCModel_RequestForModelFile(list, url, localpath);
	}

	if (bHasTModel)
	{
		char url[1024];
		sprintf_s(url, sizeof(url), "https://wootdata.github.io/scmodels_data_%d/models/player/%s/%sT.mdl", list->repoId, list->name.c_str(), list->name.c_str());
		
		char localpath[1024];
		sprintf_s(localpath, sizeof(localpath), "models/player/%s/%sT.mdl", list->name.c_str(), list->name.c_str());
		
		SCModel_RequestForModelFile(list, url, localpath);
	}

	//Assume ?
	if (1)
	{
		char url[1024];
		sprintf_s(url, sizeof(url), "https://wootdata.github.io/scmodels_data_%d/models/player/%s/%s.bmp", list->repoId, list->name.c_str(), list->name.c_str());

		char localpath[1024];
		sprintf_s(localpath, sizeof(localpath), "models/player/%s/%s.bmp", list->name.c_str(), list->name.c_str());

		SCModel_RequestForModelFile(list, url, localpath);
	}
}

void SCModel_RequestForModelJson(CDownloadList *list)
{
	char url[1024];
	sprintf_s(url, sizeof(url), "https://wootdata.github.io/scmodels_data_%d/models/player/%s/%s.json", list->repoId, list->name.c_str(), list->name.c_str());

	auto request = new CHttpRequest(
		EHTTPMethod::k_EHTTPMethodGET,
		url,
		[list](CHttpRequest *pthis, HTTPRequestCompleted_t *pResult, bool bError) {
		if (!bError && pResult->m_eStatusCode == 200)
		{
			uint32_t bodySize = 0;
			if (pthis->GetResponseBodySize(&bodySize))
			{
				PUCHAR buffer = (PUCHAR)malloc(bodySize);
				if (pthis->GetResponseBody(buffer, bodySize))
				{
					rapidjson::Document doc;

					doc.Parse((const char *)buffer, bodySize);

					if (!doc.HasParseError())
					{
						auto obj = doc.GetObj();

						bool bHasTModel = false;

						auto m_t_model = doc.FindMember("t_model");
						if (m_t_model != doc.MemberEnd() && m_t_model->value.IsBool())
						{
							bHasTModel = m_t_model->value.GetBool();
						}

						SCModel_ModelJsonAcquired(list, bHasTModel);
					}
					else
					{
						gEngfuncs.Con_Printf("SCModelDownloader: Failed to parse model json!\n");
					}
				}
				else
				{
					gEngfuncs.Con_Printf("SCModelDownloader: Failed to GetHTTPResponseBodyData!\n");
				}
				free(buffer);
			}
			else
			{
				gEngfuncs.Con_Printf("SCModelDownloader: Failed to GetHTTPResponseBodySize!\n");
			}
		}
		else
		{
			gEngfuncs.Con_Printf("SCModelDownloader: Failed the http response for model json!\n");
		}
	});

	if (!request->IsValid())
	{
		gEngfuncs.Con_Printf("SCModelDownloader: Failed to create http request for model json!\n");
		delete request;
		request = NULL;
		return;
	}

	if (!request->Send())
	{
		gEngfuncs.Con_Printf("SCModelDownloader: Failed to send http request for model json!\n");
		delete request;
		request = NULL;
		return;
	}

	list->requests.emplace_back(request);
}

void SCModel_DatabaseAcquired(void)
{
	gEngfuncs.Con_Printf("SCModelDownloader: scmodel database acquired!\n");

	for (auto &m : g_download_models)
	{
		auto itor = g_scmodels.find(m.first);
		if (itor != g_scmodels.end())
		{
			gEngfuncs.Con_Printf("SCModelDownloader: Found missing model \"%s\" in scmodel database.\n", m.first.c_str());

			SCModel_RequestForModelJson(m.second);
		}
	}
}

void SCModel_RequestForDatabase(void)
{
	if (g_scmodels.size())
	{
		SCModel_DatabaseAcquired();
		return;
	}

	if (!g_scmodel_json)
	{
		gEngfuncs.Con_Printf("SCModelDownloader: Requesting for scmodel database...\n");

		g_scmodel_json = new CHttpRequest(
			EHTTPMethod::k_EHTTPMethodGET,
			"https://raw.githubusercontent.com/wootguy/scmodels/master/database/models.json",
			[](CHttpRequest *pthis, HTTPRequestCompleted_t *pResult, bool bError) {
			if (!bError && pResult->m_eStatusCode == 200)
			{
				uint32_t bodySize = 0;
				if (pthis->GetResponseBodySize(&bodySize))
				{
					PUCHAR buffer = (PUCHAR)malloc(bodySize);
					if (pthis->GetResponseBody(buffer, bodySize))
					{
						rapidjson::Document doc;

						doc.Parse((const char *)buffer, bodySize);

						if (!doc.HasParseError())
						{
							for (auto itor = doc.MemberBegin(); itor != doc.MemberEnd(); ++itor)
							{
								std::string name = itor->name.GetString();

								auto obj = itor->value.GetObj();

								scmodel_t m = { 0 };

								auto m_size = obj.FindMember("size");
								if (m_size != obj.MemberEnd() && m_size->value.IsInt())
								{
									m.size = m_size->value.GetInt();
								}

								auto m_flags = obj.FindMember("flags");
								if (m_flags != obj.MemberEnd() && m_flags->value.IsInt())
								{
									m.flags = m_flags->value.GetInt();
								}

								auto m_polys = obj.FindMember("polys");
								if (m_polys != obj.MemberEnd() && m_polys->value.IsInt())
								{
									m.polys = m_polys->value.GetInt();
								}

								g_scmodels[name] = m;
							}

							SCModel_DatabaseAcquired();
						}
						else
						{
							gEngfuncs.Con_Printf("SCModelDownloader: Failed to parse database!\n");
						}
					}
					else
					{
						gEngfuncs.Con_Printf("SCModelDownloader: Failed to GetHTTPResponseBodyData!\n");
					}
					free(buffer);
				}
				else
				{
					gEngfuncs.Con_Printf("SCModelDownloader: Failed to GetHTTPResponseBodySize!\n");
				}
			}
			else
			{
				gEngfuncs.Con_Printf("SCModelDownloader: Failed the http response for scmodel database!\n");
			}
		});

		if (!g_scmodel_json->IsValid())
		{
			gEngfuncs.Con_Printf("SCModelDownloader: Failed to create http request for scmodel database!\n");
			delete g_scmodel_json;
			g_scmodel_json = NULL;
			return;
		}

		if (!g_scmodel_json->Send())
		{
			gEngfuncs.Con_Printf("SCModelDownloader: Failed to send http request for scmodel database!\n");
			delete g_scmodel_json;
			g_scmodel_json = NULL;
			return;
		}
	}
}

void R_StudioChangePlayerModel(void)
{
	gPrivateFuncs.R_StudioChangePlayerModel();

	int index = IEngineStudio.GetCurrentEntity()->index;
	if (index >= 1 && index <= 32)
	{
		if ((*DM_PlayerState)[index - 1].model == IEngineStudio.GetCurrentEntity()->model)
		{
			if ((*DM_PlayerState)[index - 1].name[0])
			{
				std::string name = (*DM_PlayerState)[index - 1].name;
				
				gEngfuncs.Con_Printf("SCModelDownloader: Missing model \"%s\".\n", name.c_str());

				if (scmodel_autodownload->value)
				{
					auto itor = g_download_models.find(name);
					if (itor == g_download_models.end())
					{
						g_download_models[name] = new CDownloadList(name);
						SCModel_RequestForDatabase();
					}
				}
			}
		}
	}
}

int HUD_VidInit(void)
{
	if (g_scmodel_json)
	{
		delete g_scmodel_json;
		g_scmodel_json = NULL;
	}

	for (auto m : g_download_models)
	{
		for (auto r : m.second->requests)
		{
			delete r;
		}
		delete m.second;
	}

	g_download_models.clear();

	return gExportfuncs.HUD_VidInit();	
}

void HUD_Init(void)
{
	gExportfuncs.HUD_Init();

	scmodel_autodownload = gEngfuncs.pfnRegisterVariable("scmodel_autodownload", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
}

int HUD_GetStudioModelInterface(int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio)
{
	memcpy(&IEngineStudio, pstudio, sizeof(IEngineStudio));
	gpStudioInterface = ppinterface;

	gPrivateFuncs.studioapi_SetupPlayerModel = IEngineStudio.SetupPlayerModel;

	if (1)
	{
		g_pMetaHookAPI->DisasmRanges(pstudio->SetupPlayerModel, 0x500, [](void *inst, PUCHAR address, size_t instLen, int instCount, int depth, PVOID context)
		{
			auto pinst = (cs_insn *)inst;

			if (address[0] == 0xE8)
			{
				PVOID imm = (PVOID)pinst->detail->x86.operands[0].imm;

				if (imm == gPrivateFuncs.R_StudioChangePlayerModel)
				{
					PVOID ptarget = R_StudioChangePlayerModel;
					int rva = (PUCHAR)ptarget - (address + 5);
					g_pMetaHookAPI->WriteMemory(address + 1, &rva, 4);
				}
			}

			if (!DM_PlayerState)
			{
				if (pinst->id == X86_INS_LEA &&
					pinst->detail->x86.op_count == 2 &&
					pinst->detail->x86.operands[0].type == X86_OP_REG &&
					pinst->detail->x86.operands[1].type == X86_OP_MEM &&
					(PUCHAR)pinst->detail->x86.operands[1].mem.disp > (PUCHAR)g_dwEngineDataBase &&
					(PUCHAR)pinst->detail->x86.operands[1].mem.disp < (PUCHAR)g_dwEngineDataBase + g_dwEngineDataSize &&
					pinst->detail->x86.operands[1].mem.base != 0 &&
					pinst->detail->x86.operands[1].mem.scale == 1 || pinst->detail->x86.operands[1].mem.scale == 4)
				{
					DM_PlayerState = (decltype(DM_PlayerState))pinst->detail->x86.operands[1].mem.disp;
				}
			}

			if (address[0] == 0xCC)
				return TRUE;

			if (pinst->id == X86_INS_RET)
				return TRUE;

			return FALSE;
		}, 0, NULL);
	}

	int result = gExportfuncs.HUD_GetStudioModelInterface ? gExportfuncs.HUD_GetStudioModelInterface(version, ppinterface, pstudio) : 1;

	return result;
}