#pragma once

#include <string>

class CHTTPPayload
{
public:
	~CHTTPPayload();

	CHTTPPayload(const char* d, size_t l);
	char* m_pData = nullptr;
	size_t m_iSize = 0;
};

typedef void (*fnHTTPContinueWith)(class CHTTPRespond* payload, void* user);

class CHTTPRespond
{
public:
	~CHTTPRespond();

	CHTTPRespond(CHTTPPayload* payload, int code);

	int m_iHTTPCode = -1;
	CHTTPPayload* m_pPayload = nullptr;

	const fnHTTPContinueWith* m_pfnContinueWith = nullptr;
	void* m_pUser = nullptr;
};

typedef void (*fnHTTPRequestFailed)(void* user);
typedef void (*fnHTTPRequestCompleted)(CHTTPPayload* payload, void* user);

class CHTTPRequest
{
public:
	static CHTTPRequest* Create(const char* url);

	CHTTPRequest* OnFailed(const fnHTTPRequestFailed& pfn, void* user);
	CHTTPRequest* OnResponed(const fnHTTPRequestCompleted& pfn, void* user);

	//in main thread
	CHTTPRequest* ContinueWith(const fnHTTPContinueWith& pfn, void* user);

	void Start();


	const fnHTTPRequestCompleted* m_pfnOnCompleted = nullptr;
	void* m_pCompletedUserData = nullptr;

	const fnHTTPRequestFailed* m_pfnOnFailed = nullptr;
	void* m_pFailedUserData = nullptr;

	const fnHTTPContinueWith* m_pfnContinueWith = nullptr;
	void* m_pContinueUserData = nullptr;

	std::string m_szUrl;
};

void UtilHTTPClient_Init();
void UtilHTTPClient_RunFrame();
void UtilHTTPClient_Shutdown();