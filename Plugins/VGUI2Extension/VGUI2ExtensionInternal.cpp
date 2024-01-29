#include <metahook.h>
#include "VGUI2ExtensionInternal.h"

#include <vector>
#include <algorithm>

const char* GetBaseDirectory();
const char* GetCurrentLanguage();

class CVGUI2Extension : public IVGUI2ExtensionInternal
{
private:
	std::vector<IVGUI2Extension_BaseUICallbacks*> m_BaseUICallbacks;
	std::vector<IVGUI2Extension_GameUICallbacks*> m_GameUICallbacks;
	std::vector<IVGUI2Extension_ClientVGUICallbacks*> m_ClientVGUICallbacks;

public:

	void RegisterBaseUICallbacks(IVGUI2Extension_BaseUICallbacks* pCallbacks) override
	{
		m_BaseUICallbacks.emplace_back(pCallbacks);

		std::sort(m_BaseUICallbacks.begin(), m_BaseUICallbacks.end(),
			[](const IVGUI2Extension_BaseUICallbacks* a, const IVGUI2Extension_BaseUICallbacks* b) -> bool {
				return a->GetAltitude() > b->GetAltitude();
			});
	}

	void UnregisterBaseUICallbacks(IVGUI2Extension_BaseUICallbacks* pCallbacks) override
	{
		for (auto it = m_BaseUICallbacks.begin(); it != m_BaseUICallbacks.end(); ++it)
		{
			if (*it == pCallbacks)
			{
				m_BaseUICallbacks.erase(it);
				return;
			}
		}
	}

	void RegisterGameUICallbacks(IVGUI2Extension_GameUICallbacks* pCallbacks) override
	{
		m_GameUICallbacks.emplace_back(pCallbacks);

		std::sort(m_GameUICallbacks.begin(), m_GameUICallbacks.end(),
			[](const IVGUI2Extension_GameUICallbacks* a, const IVGUI2Extension_GameUICallbacks* b) -> bool {
				return a->GetAltitude() > b->GetAltitude();
			});
	}

	void UnregisterGameUICallbacks(IVGUI2Extension_GameUICallbacks* pCallbacks) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			if (*it == pCallbacks)
			{
				m_GameUICallbacks.erase(it);
				return;
			}
		}
	}

	void RegisterClientVGUICallbacks(IVGUI2Extension_ClientVGUICallbacks* pCallbacks) override
	{
		m_ClientVGUICallbacks.emplace_back(pCallbacks);

		std::sort(m_ClientVGUICallbacks.begin(), m_ClientVGUICallbacks.end(),
			[](const IVGUI2Extension_ClientVGUICallbacks* a, const IVGUI2Extension_ClientVGUICallbacks* b) -> bool {
				return a->GetAltitude() > b->GetAltitude();
			});
	}

	void UnregisterClientVGUICallbacks(IVGUI2Extension_ClientVGUICallbacks* pCallbacks) override
	{
		for (auto it = m_ClientVGUICallbacks.begin(); it != m_ClientVGUICallbacks.end(); ++it)
		{
			if (*it == pCallbacks)
			{
				m_ClientVGUICallbacks.erase(it);
				return;
			}
		}
	}

	const char* GetBaseDirectory() const override
	{
		return GetBaseDirectory();
	}

	const char* GetCurrentLanguage() const override
	{
		return GetCurrentLanguage();
	}

public:

	//BaseUI

	void BaseUI_Initialize(CreateInterfaceFn* factories, int count) override
	{
		for (auto it = m_BaseUICallbacks.begin(); it != m_BaseUICallbacks.end(); ++it)
		{
			(*it)->Initialize(factories, count);
		}
	}

	void BaseUI_Start(struct cl_enginefuncs_s* engineFuncs, int interfaceVersion) override
	{
		for (auto it = m_BaseUICallbacks.begin(); it != m_BaseUICallbacks.end(); ++it)
		{
			(*it)->Start(engineFuncs, interfaceVersion);
		}
	}

	void BaseUI_Shutdown() override
	{
		for (auto it = m_BaseUICallbacks.begin(); it != m_BaseUICallbacks.end(); ++it)
		{
			(*it)->Shutdown();
		}
	}

	void BaseUI_Key_Event(int &down, int &keynum, const char* &pszCurrentBinding, VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_BaseUICallbacks.begin(); it != m_BaseUICallbacks.end(); ++it)
		{
			(*it)->Key_Event(down, keynum, pszCurrentBinding, CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void BaseUI_CallEngineSurfaceProc(void* &pevent, void* &userData, VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_BaseUICallbacks.begin(); it != m_BaseUICallbacks.end(); ++it)
		{
			(*it)->CallEngineSurfaceProc(pevent, userData, CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void BaseUI_Paint(int &x, int &y, int &right, int &bottom, VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_BaseUICallbacks.begin(); it != m_BaseUICallbacks.end(); ++it)
		{
			(*it)->Paint(x, y, right, bottom, CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void BaseUI_HideGameUI(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_BaseUICallbacks.begin(); it != m_BaseUICallbacks.end(); ++it)
		{
			(*it)->HideGameUI(CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void BaseUI_ActivateGameUI(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_BaseUICallbacks.begin(); it != m_BaseUICallbacks.end(); ++it)
		{
			(*it)->ActivateGameUI(CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void BaseUI_IsGameUIVisible(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_BaseUICallbacks.begin(); it != m_BaseUICallbacks.end(); ++it)
		{
			(*it)->IsGameUIVisible(CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void BaseUI_HideConsole(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_BaseUICallbacks.begin(); it != m_BaseUICallbacks.end(); ++it)
		{
			(*it)->HideConsole(CallbackContext);
			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void BaseUI_ShowConsole(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_BaseUICallbacks.begin(); it != m_BaseUICallbacks.end(); ++it)
		{
			(*it)->ShowConsole(CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}
	//GameUI

	void GameUI_Initialize(CreateInterfaceFn* factories, int count) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->Initialize(factories, count);
		}
	}

	void GameUI_Start(struct cl_enginefuncs_s* engineFuncs, int interfaceVersion, void* system) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->Start(engineFuncs, interfaceVersion, system);
		}
	}

	void GameUI_Shutdown() override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->Shutdown();
		}
	}

	void GameUI_ActivateGameUI(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->ActivateGameUI(CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_ActivateDemoUI(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->ActivateDemoUI(CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_HasExclusiveInput(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->HasExclusiveInput(CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_RunFrame(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->RunFrame(CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_ConnectToServer(const char*& game, int& IP, int& port, VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->ConnectToServer(game, IP, port, CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_DisconnectFromServer(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->DisconnectFromServer(CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}
	void GameUI_HideGameUI(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->HideGameUI(CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_IsGameUIActive(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->IsGameUIActive(CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_LoadingStarted(const char*& resourceType, const char*& resourceName, VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->LoadingStarted(resourceType, resourceName, CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_LoadingFinished(const char*& resourceType, const char*& resourceName, VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->LoadingFinished(resourceType, resourceName, CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_StartProgressBar(const char*& progressType, int& progressSteps, VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->StartProgressBar(progressType, progressSteps, CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_ContinueProgressBar(int& progressPoint, float& progressFraction, VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->ContinueProgressBar(progressPoint, progressFraction, CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_StopProgressBar(bool& bError, const char*& failureReason, const char*& extendedReason, VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->StopProgressBar(bError, failureReason, extendedReason, CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_SetProgressBarStatusText(const char*& statusText, VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->SetProgressBarStatusText(statusText, CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_SetSecondaryProgressBar(float& progress, VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->SetSecondaryProgressBar(progress, CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	void GameUI_SetSecondaryProgressBarText(const char*& statusText, VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_GameUICallbacks.begin(); it != m_GameUICallbacks.end(); ++it)
		{
			(*it)->SetSecondaryProgressBarText(statusText, CallbackContext);

			if (CallbackContext->Result >= VGUI2Extension_Result::HANDLED)
			{
				return;
			}
		}
	}

	//ClientVGUI

	void ClientVGUI_Initialize(CreateInterfaceFn* factories, int count) override
	{
		for (auto it = m_ClientVGUICallbacks.begin(); it != m_ClientVGUICallbacks.end(); ++it)
		{
			(*it)->Initialize(factories, count);
		}
	}

	void ClientVGUI_Shutdown() override
	{
		for (auto it = m_ClientVGUICallbacks.begin(); it != m_ClientVGUICallbacks.end(); ++it)
		{
			(*it)->Shutdown();
		}
	}

	void ClientVGUI_Start() override
	{
		for (auto it = m_ClientVGUICallbacks.begin(); it != m_ClientVGUICallbacks.end(); ++it)
		{
			(*it)->Start();
		}
	}

	void ClientVGUI_SetParent(vgui::VPANEL parent) override
	{
		for (auto it = m_ClientVGUICallbacks.begin(); it != m_ClientVGUICallbacks.end(); ++it)
		{
			(*it)->SetParent(parent);
		}
	}

	void ClientVGUI_UseVGUI1(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_ClientVGUICallbacks.begin(); it != m_ClientVGUICallbacks.end(); ++it)
		{
			(*it)->UseVGUI1(CallbackContext);
		}
	}

	void ClientVGUI_HideScoreBoard(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_ClientVGUICallbacks.begin(); it != m_ClientVGUICallbacks.end(); ++it)
		{
			(*it)->HideScoreBoard(CallbackContext);
		}
	}

	void ClientVGUI_HideAllVGUIMenu(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_ClientVGUICallbacks.begin(); it != m_ClientVGUICallbacks.end(); ++it)
		{
			(*it)->HideAllVGUIMenu(CallbackContext);
		}
	}

	void ClientVGUI_ActivateClientUI(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_ClientVGUICallbacks.begin(); it != m_ClientVGUICallbacks.end(); ++it)
		{
			(*it)->ActivateClientUI(CallbackContext);
		}
	}

	void ClientVGUI_HideClientUI(VGUI2Extension_CallbackContext* CallbackContext) override
	{
		for (auto it = m_ClientVGUICallbacks.begin(); it != m_ClientVGUICallbacks.end(); ++it)
		{
			(*it)->HideClientUI(CallbackContext);
		}
	}
};

static CVGUI2Extension s_VGUI2Extension;

IVGUI2ExtensionInternal* VGUI2ExtensionInternal()
{
	return &s_VGUI2Extension;
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CVGUI2Extension, IVGUI2Extension, VGUI2_EXTENSION_INTERFACE_VERSION, s_VGUI2Extension);