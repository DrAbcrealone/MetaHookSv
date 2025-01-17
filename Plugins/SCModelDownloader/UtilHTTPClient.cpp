#include <vector>
#include <future>
#include <thread>

#include <metahook.h>
#include "plugins.h"

#include "libcurl.h"
#include "UtilHTTPClient.h"

static std::vector<std::future<CHTTPRespond*>> s_aryPromise{};

void UtilHTTPClient_Init()
{
	CURL_Init();
}

void UtilHTTPClient_RunFrame()
{
	for (auto iter = s_aryPromise.begin(); iter != s_aryPromise.end();)
	{
		auto status = iter->wait_for(std::chrono::seconds(0));
		if (status == std::future_status::ready)
		{
			auto rep = iter->get();
			if (rep)
			{
				auto fnContinue = rep->m_pfnContinueWith;
				(*fnContinue)(rep, rep->m_pUser);
			}
			delete rep;
			iter = s_aryPromise.erase(iter);
		}
		else
			iter++;
	}
}

void UtilHTTPClient_Shutdown()
{
	CURL_Shutdown();
}



CHTTPPayload::~CHTTPPayload()
{
	if (m_pData)
		delete[] m_pData;
}

CHTTPPayload::CHTTPPayload(const char* d, size_t l)
{
	m_iSize = l;
	m_pData = new char[m_iSize];
	memcpy(m_pData, d, m_iSize);

}

CHTTPRequest* CHTTPRequest::Create(const char* url)
{
	CHTTPRequest* req = new CHTTPRequest();
	req->m_szUrl = url;
	return req;
}

CHTTPRequest* CHTTPRequest::OnFailed(const fnHTTPRequestFailed& pfn, void* user)
{
	this->m_pfnOnFailed = &pfn;
	this->m_pFailedUserData = user;
	return this;
}

CHTTPRequest* CHTTPRequest::OnResponed(const fnHTTPRequestCompleted& pfn, void* user)
{
	this->m_pfnOnCompleted = &pfn;
	this->m_pCompletedUserData = user;
	return this;
}

CHTTPRequest* CHTTPRequest::ContinueWith(const fnHTTPContinueWith& pfn, void* user)
{
	this->m_pfnContinueWith = &pfn;
	this->m_pContinueUserData = user;
	return this;
}

static size_t append(void* ptr, size_t size, size_t nmemb, void* user) {
	std::vector<char>* p = (std::vector<char>*)user;
	auto cs = p->size();
	p->resize(cs + size * nmemb);
	memcpy(p->data() + cs, ptr, size * nmemb);
	return size * nmemb;
}

void CHTTPRequest::Start()
{
	auto future = std::async([](CHTTPRequest* req) -> CHTTPRespond* {
		void* curl = curl_easy_init();
		if (curl)
		{
			std::vector<char> readBuffer;
			curl_easy_setopt(curl, CURLOPT_URL, req->m_szUrl.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
			CURLcode res = curl_easy_perform(curl);
			if (res != CURLE_OK)
			{
				auto fnFailed = req->m_pfnOnFailed;
				if (fnFailed)
					(*fnFailed)(req->m_pFailedUserData);
				curl_easy_cleanup(curl);
				delete req;
				return nullptr;
			}
			else
			{
				long http_code = 0;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
				CHTTPPayload* payload = new CHTTPPayload(readBuffer.data(), readBuffer.size());
				auto fnComplete = req->m_pfnOnCompleted;
				if (fnComplete)
				{
					(*fnComplete)(payload, req->m_pCompletedUserData);
				}
				curl_easy_cleanup(curl);
				CHTTPRespond* respond = new CHTTPRespond(payload, (int)http_code);
				respond->m_pfnContinueWith = req->m_pfnContinueWith;
				respond->m_pUser = req->m_pContinueUserData;
				delete req;
				return respond;
			}
		}
		else
		{
			auto fnFailed = req->m_pfnOnFailed;
			if (fnFailed)
				(*fnFailed)(req->m_pFailedUserData);
			return nullptr;
		}
		}, this);
	s_aryPromise.push_back(future);
}

CHTTPRespond::~CHTTPRespond()
{
	if (m_pPayload)
		delete m_pPayload;
}

CHTTPRespond::CHTTPRespond(CHTTPPayload* payload, int code)
{
	m_pPayload = payload;
	m_iHTTPCode = code;
}
