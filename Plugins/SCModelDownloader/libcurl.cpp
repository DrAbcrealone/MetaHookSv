#include <metahook.h>

#include "libcurl.h"

#pragma region CURL Methods

void* (*curl_easy_init)(void) = nullptr;
CURLcode(*curl_easy_setopt)(void* curl, CURLoption option, ...) = nullptr;
CURLcode(*curl_easy_getinfo)(void* curl, CURLINFO info, ...) = nullptr;
CURLcode(*curl_easy_perform)(void* curl) = nullptr;
void (*curl_easy_cleanup)(void* curl) = nullptr;
struct curl_slist* (*curl_slist_append)(struct curl_slist*, const char*) = nullptr;
void (*curl_slist_free_all)(struct curl_slist* list) = nullptr;

#pragma endregion

static HINTERFACEMODULE g_hLibCurl = nullptr;

void CURL_Init() {
	g_hLibCurl = Sys_LoadModule("libcurl.dll");

	if (!g_hLibCurl)
	{
		g_pMetaHookAPI->SysError("Could not load libcurl.dll");
		return;
	}

#define LIBCURL_FUNCTION_DEFINE(name) name = (decltype(name))(GetProcAddress((HMODULE)g_hLibCurl, #name))
	LIBCURL_FUNCTION_DEFINE(curl_easy_init);
	LIBCURL_FUNCTION_DEFINE(curl_slist_append);
	LIBCURL_FUNCTION_DEFINE(curl_easy_setopt);
	LIBCURL_FUNCTION_DEFINE(curl_easy_getinfo);
	LIBCURL_FUNCTION_DEFINE(curl_easy_perform);
	LIBCURL_FUNCTION_DEFINE(curl_slist_free_all);
	LIBCURL_FUNCTION_DEFINE(curl_easy_cleanup);
#undef LIBCURL_FUNCTION_DEFINE
}

void CURL_Shutdown() {
	if (g_hLibCurl)
	{
		Sys_FreeModule(g_hLibCurl);
		g_hLibCurl = nullptr;
	}
}

