//======= Maestra Fenix, 2024 ==================================================//
//======= Obsidian Conflict Team, 2024 =========================================//
//======= Cvoxalury, 2024 ======================================================//
//
// Purpose: Map load background panel
// Todo: The MP side of things. Secondary bar for when external files are needed,
// status text related to that, status text for the first bar (connecting, etc)...
// The secondary bar in vanilla appears to be called "Progress2", but I don't
// know how to extract its text messages, and I don't have the ability to
// test downloading external files to verify its working.
//==============================================================================//

#include "cbase.h"
#include "ILoadingBackground.h"
#include "GameUI/IGameUI.h"
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

IMaterial* mapBackground;

CMapLoadBG::CMapLoadBG(char const* panelName) : EditablePanel(NULL, panelName)
{
	VPANEL toolParent = enginevgui->GetPanel(PANEL_GAMEUIDLL);
	SetParent(toolParent);
	// OC Team Code
	std::string sImagePath = "materials" CORRECT_PATH_SEPARATOR_S "vgui" CORRECT_PATH_SEPARATOR_S "loading" CORRECT_PATH_SEPARATOR_S;

	FileFindHandle_t hImage = FILESYSTEM_INVALID_FIND_HANDLE;

	m_hImages["_"] = string_list_t();
	for (
		const char* sName = g_pFullFileSystem->FindFirstEx((sImagePath + "*.vmt").c_str(), "MOD", &hImage);
		sName;
		sName = g_pFullFileSystem->FindNext(hImage)
		)
	{
		if (sName[0] != '.')
		{
			std::string sImage = sName;
			sImage = sImage.substr(0, sImage.length() - 4);

			m_hImages["_"].push_back("loading/" + sImage);
		}
	}

	g_pFullFileSystem->FindClose(hImage);
	hImage = FILESYSTEM_INVALID_FIND_HANDLE;

	for (
		const char* sName = g_pFullFileSystem->FindFirstEx((sImagePath + "maps" + CORRECT_PATH_SEPARATOR_S + "*.vmt").c_str(), "MOD", &hImage);
		sName;
		sName = g_pFullFileSystem->FindNext(hImage)
		)
	{
		if (sName[0] != '.')
		{
			std::string sImage = sName;
			sImage = sImage.substr(0, sImage.length() - 4);

			// ToDo: find method to get base map name, preferably a common one so we end up with one list per category
			// ex: GetChapterForMap( sImage ) == "route_kanal"; // sImage == "d1_canals_01"
			std::string sCategory = sImage;

			if (!m_hImages.count(sCategory)) m_hImages[sCategory] = string_list_t();

			m_hImages[sCategory].push_back("loading/maps/" + sImage);
		}
	}

	g_pFullFileSystem->FindClose(hImage);
	//
	m_pBackground = new ImagePanel(this, "Background");

	if (m_pBackground)
	{
		m_pBackground->SetSize(ScreenWidth(), ScreenHeight());
		m_pBackground->SetShouldScaleImage(true);
		Reset();
	}

#ifdef ENABLE_CUSTOM_LOADING_BAR
	m_pProgressBar = new ContinuousProgressBar(this, "ProgressBar");

	if (m_pProgressBar) m_pProgressBar->SetProgressDirection(0);

	m_pProgressPercentage = new vgui::Label(this, "ProgressPercentage", "   ");

	//#if HL2MP
	//	m_pProgressStatusMessage = new vgui::Label(this, "ProgressStatusMessage", "   ");
	//#endif
#endif

#ifdef ENABLE_CUSTOM_LOADING_WHEEL	
	m_pProgressWheel = new CircularProgressBar(this, "ProgressWheel");

	if (m_pProgressWheel) m_pProgressWheel->SetProgressDirection(vgui::CircularProgressBar::PROGRESS_CW);
#endif

#ifdef ENABLE_LOADING_TIP
	m_pLoadingTip = new vgui::Label(this, "LoadingTip", " ");
	// OC
	iNumberTips = 0;

	//Fenix: This scans through all loaded strings of the mod. It's fast, but perhaps it could be even faster if we could tell it where exactly to look?
	for (int i = g_pVGuiLocalize->GetFirstStringIndex(); i != INVALID_LOCALIZE_STRING_INDEX; i = g_pVGuiLocalize->GetNextStringIndex(i))
	{
		const char* strName = g_pVGuiLocalize->GetNameByIndex(i);

		if (V_strncmp("loadingscreen_Tip_", strName, 18) == 0)
			iNumberTips++;
	}
	//
#endif

	progress = 0;
	iLoadingBarHandle = 0;

	LoadControlSettings("resource/ui/loadingscreen.res");
}

CMapLoadBG::~CMapLoadBG()
{
	// None
}
// OC Team code
//-----------------------------------------------------------------------------
// Purpose: Loads a random default or a map defined loading screen texture.
//-----------------------------------------------------------------------------
void CMapLoadBG::SetMap(const std::string& sMap)
{
	std::string sCurrentImage = (m_pBackground->GetImageName() ? m_pBackground->GetImageName() : "");

	if (!sMap.empty())
	{
		// ToDo: map category: sCategory = GetChapterForMap( sMap );
		std::string sCategory = sMap;

		if (sCurrentImage.find(sCategory) != std::string::npos) return;

		for (const auto& itImageCategory : m_hImages)
		{
			if (!std::equal(sCategory.begin(), sCategory.end(), itImageCategory.first.begin())) continue;

			return m_pBackground->SetImage(itImageCategory.second[random->RandomInt(0, itImageCategory.second.size() - 1)].c_str());
		}

		// Note: optimization, don't bother setting a default image when we already have one, excluding the default fallback ("loading/default" == 15 chars)
		// Note: this effectively makes the random default background be chosen once per session, it will persist
		// across map loads unless a map-specific one overrides it.
		if (sCurrentImage.find("loading/default") != std::string::npos && sCurrentImage.length() > 15) return;
		return m_pBackground->SetImage(m_hImages["_"][random->RandomInt(0, m_hImages["_"].size() - 1)].c_str());
	}

	return m_pBackground->SetImage("loading/default");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMapLoadBG::Reset()
{
	SetMap();
}
//
void CMapLoadBG::ApplySchemeSettings(IScheme* pScheme)
{
	// OC
	BaseClass::ApplySchemeSettings(pScheme);
	SetBounds(0, 0, ScreenWidth(), ScreenHeight());
	//
	IScheme* pClientScheme = vgui::scheme()->GetIScheme(vgui::scheme()->GetScheme("ClientScheme"));

#ifdef ENABLE_CUSTOM_LOADING_BAR

	if (m_pProgressBar)
	{
		m_pProgressBar->SetAlpha(100);
		m_pProgressBar->SetPaintBorderEnabled(false);
		m_pProgressBar->SetSize(max(256, ScreenWidth() * 0.66f), max(8, (ScreenHeight() / 100)));
		m_pProgressBar->SetPos(ScreenWidth() / 6, ScreenHeight() * 0.9f);

		if (m_pProgressPercentage)
		{
			m_pProgressPercentage->SetFont(pClientScheme->GetFont("Default", true));
			m_pProgressPercentage->SetSize(32, 32);
			m_pProgressPercentage->SetPos(ScreenWidth() / 6 + m_pProgressBar->GetWide() + m_pProgressPercentage->GetWide() / 2, (ScreenHeight() * 0.9f) - m_pProgressBar->GetTall() / 2);
			m_pProgressPercentage->SetContentAlignment(vgui::Label::a_southwest);
		}
		//#if HL2MP
		//		if (m_pProgressStatusMessage)
		//		{
		//			m_pProgressStatusMessage->SetFont(pClientScheme->GetFont("Default", true));
		//			m_pProgressStatusMessage->SetSize(32, 32);
		//			m_pProgressStatusMessage->SetPos(ScreenWidth() / 2 - m_pProgressStatusMessage->GetWide() / 2, ScreenHeight() * 0.95);
		//			m_pProgressStatusMessage->SetContentAlignment(vgui::Label::a_southwest);
		//		}
		//#endif
	}
#endif
#ifdef ENABLE_CUSTOM_LOADING_WHEEL
	if (m_pProgressWheel)
	{
		m_pProgressWheel->SetAlpha(100);
		m_pProgressWheel->SetPaintBorderEnabled(false);
		m_pProgressWheel->SetSize(min(64, ScreenWidth() * 0.1), min(64, ScreenWidth() * 0.1));
		m_pProgressWheel->SetPos((ScreenWidth()) - (m_pProgressWheel->GetWide() * 1.5f), (ScreenHeight()) - (m_pProgressWheel->GetTall() * 1.5f));
		m_pProgressWheel->SetBgImage("loading/ui/loading_wheel_bg");
		m_pProgressWheel->SetFgImage("loading/ui/loading_wheel_fg");
	}
#endif

#ifdef ENABLE_LOADING_TIP
	if (m_pLoadingTip)
	{
		m_pLoadingTip->SetFont(pClientScheme->GetFont("Default", true));
		m_pLoadingTip->SetContentAlignment(vgui::Label::a_center);
		m_pLoadingTip->SizeToContents();
		m_pLoadingTip->SetPos(ScreenWidth() / 2 - m_pLoadingTip->GetWide() / 2, ScreenHeight() * 0.85f);
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Scan from the root panel down its children to find the handle
// to the loading bar panel.
//-----------------------------------------------------------------------------
bool CMapLoadBG::FindLoadingDialogBarHandle(void)
{
	int iGameuipanel, iBasegameuipanel, iLoadingdialog;
	iGameuipanel = iBasegameuipanel = iLoadingdialog = 0;
	VPANEL searchpanel = vgui::ivgui()->HandleToPanel(1); // 1 is the parent of gameuipanel
	if (searchpanel)
	{
		for (int i = 0; i < vgui::ipanel()->GetChildCount(searchpanel); i++)
		{
			VPANEL tmppanel = vgui::ipanel()->GetChild(searchpanel, i);

			if (!Q_strcmp(vgui::ipanel()->GetName(tmppanel), "GameUI Panel"))
				iGameuipanel = vgui::ivgui()->PanelToHandle(tmppanel);
		}
	}

	if (iGameuipanel != 0) // the gameui panel, which hosts the parent of loading dialogue
	{
		//	AssertMsg(0, "iGameuipanel is %i", iGameuipanel);
		searchpanel = vgui::ivgui()->HandleToPanel(iGameuipanel);
		if (searchpanel)
		{
			for (int i = 0; i < vgui::ipanel()->GetChildCount(searchpanel); i++)
			{
				VPANEL tmppanel = vgui::ipanel()->GetChild(searchpanel, i);

				if (!Q_strcmp(vgui::ipanel()->GetName(tmppanel), "BaseGameUIPanel"))
				{
					iBasegameuipanel = vgui::ivgui()->PanelToHandle(tmppanel);
				}
			}
		}
	}

	if (iBasegameuipanel != 0) // the parent of loading dialogue
	{
		//	AssertMsg(0, "iBasegameuipanel is %i", iBasegameuipanel);
		searchpanel = vgui::ivgui()->HandleToPanel(iBasegameuipanel);
		if (searchpanel)
		{
			for (int i = 0; i < vgui::ipanel()->GetChildCount(searchpanel); i++)
			{
				VPANEL tmppanel = vgui::ipanel()->GetChild(searchpanel, i);

				if (!Q_strcmp(vgui::ipanel()->GetName(tmppanel), "LoadingDialog"))
				{
					iLoadingdialog = vgui::ivgui()->PanelToHandle(tmppanel);
					vgui::ipanel()->SetPos(tmppanel, -99999, -99999); // HACK: for some reason SetVisible doesn't work?.. push it out into the cosmic void.
				}
			}
		}
	}

	if (iLoadingdialog != 0) // the loading dialogue, parent of loading bar element
	{
		//	AssertMsg(0, "iLoadingdialog is %i", iLoadingdialog);
		searchpanel = vgui::ivgui()->HandleToPanel(iLoadingdialog);
		if (searchpanel)
		{
			for (int i = 0; i < vgui::ipanel()->GetChildCount(searchpanel); i++)
			{
				VPANEL tmppanel = vgui::ipanel()->GetChild(searchpanel, i);

				if (!Q_strcmp(vgui::ipanel()->GetName(tmppanel), "Progress"))
					iLoadingBarHandle = vgui::ivgui()->PanelToHandle(tmppanel);

				vgui::ipanel()->SetVisible(tmppanel, false); // hide the loading dialog's elements
			}
		}
	}

	if (iLoadingBarHandle != 0) // the loading bar itself
	{
		vgui::ipanel()->SetVisible(vgui::ivgui()->HandleToPanel(iLoadingBarHandle), false);
	}

	return iLoadingBarHandle != 0;
}

//-----------------------------------------------------------------------------
// Purpose: Catch the activation message to choose one of random default 
// loading screens (this is guaranteed to happen before we know the name of the
// map we're loading, as that info cannot be made readily available, and we
// may not even have/want a separate texture for it.
//-----------------------------------------------------------------------------
void CMapLoadBG::OnMessage(const KeyValues* params, VPANEL fromPanel)
{
#ifdef ENABLE_LOADING_TIP
	if (m_pLoadingTip)
	{
		char tipText[256];

		Q_snprintf(tipText, sizeof(tipText), "#loadingscreen_Tip_%i", RandomInt(1, iNumberTips)); // OC
		if (g_pVGuiLocalize->Find(tipText))
		{
			m_pLoadingTip->SetText(tipText);
		}
		else // retry with at least #1 string. Again, if we just enumerated the strings, we wouldn't need this...
		{
			if (g_pVGuiLocalize->Find("#loadingscreen_Tip_1"))
				m_pLoadingTip->SetText("#loadingscreen_Tip_1");
		}

		m_pLoadingTip->SizeToContents();
		m_pLoadingTip->SetPos(ScreenWidth() / 2 - m_pLoadingTip->GetWide() / 2, ScreenHeight() * 0.85f); // FIXME: without it, the labels is misaligned even though it should be a_center?
	}
#endif

	BaseClass::OnMessage(params, fromPanel);
}

void CMapLoadBG::OnThink(void)
{
	BaseClass::OnThink();

	if (FindLoadingDialogBarHandle())
	{
		VPANEL panel = vgui::ivgui()->HandleToPanel(iLoadingBarHandle);
		if (panel)
		{
			ProgressBar* tempBar = dynamic_cast<vgui::ProgressBar*>(vgui::ipanel()->GetPanel(panel, "GAMEUI"));
			if (tempBar)
			{
				progress = tempBar->GetProgress();
			}
			else
			{
				Assert(0);
			}
		}
		else
		{
			Assert(0);
		}
	}

#ifdef ENABLE_CUSTOM_LOADING_BAR
	if (m_pProgressBar)
	{
		m_pProgressBar->SetProgress(progress);

		if (m_pProgressPercentage)
		{
			char percentage[10]; //itoa does not exist in gcc so it needs to be replaced with Q_snprintf
			Q_snprintf(percentage, ARRAYSIZE(percentage), "%i", (int)(100 * progress));
			m_pProgressPercentage->SetText(percentage);
			m_pProgressPercentage->SizeToContents();
		}

		//#ifdef HL2MP
		//		if (m_pProgressStatusMessage)
		//		{
		//			char statusText[256];
		//			itoa((int)100 * progress, statusText, 10);
		//			m_pProgressStatusMessage->SetText(statusText);
		//			m_pProgressStatusMessage->SizeToContents();
		//			m_pProgressStatusMessage->SetPos(ScreenWidth() / 2 - m_pProgressStatusMessage->GetWide() / 2, ScreenHeight() * 0.95f); // FIXME: without it, the labels is misaligned even though it should be a_center?
		//		}
		//#endif
	}
#endif
#ifdef ENABLE_CUSTOM_LOADING_WHEEL
	if (m_pProgressWheel)
	{
		m_pProgressWheel->SetProgress(progress);
	}
#endif
	// OC Team code
	static std::string sMap = UTIL_GetCurrentMap();

	std::string sCurrentMap = UTIL_GetCurrentMap();
	if (sMap == sCurrentMap) return;
	else sMap = sCurrentMap;

	sCurrentMap = V_GetFileName(sCurrentMap.c_str());
	if (sCurrentMap.find(".bsp") != std::string::npos) sCurrentMap = sCurrentMap.substr(0, sCurrentMap.length() - 4);

	SetMap(sCurrentMap);
	//
}