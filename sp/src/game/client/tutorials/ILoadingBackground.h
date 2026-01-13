//======= Maestra Fenix, 2024 ==================================================//
//======= Obsidian Conflict Team, 2024 =========================================//
//======= Cvoxalury, 2024 ======================================================//
// Purpose: Map load background panel
//
//==============================================================================//
#ifndef MAPLOAD_BACKGROUND_H
#define MAPLOAD_BACKGROUND_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui/ISurface.h"
#include "vgui/IVGui.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ProgressBar.h>
#include "vgui_controls/CircularProgressBar.h"
#include <vgui_controls/Label.h>
#include "ienginevgui.h"
#include <KeyValues.h>
// OC Team code
#include <string>
#include <vector>
#include <unordered_map>
#include "filesystem.h"
//
#define ENABLE_LOADING_TIP // if defined, adds an element for tip text
#define ENABLE_CUSTOM_LOADING_BAR // if defined, overrides the loading bar, hides the original one
#define ENABLE_CUSTOM_LOADING_WHEEL // if defined, a circular progress bar will appear in bottom right corner

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CMapLoadBG : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CMapLoadBG, vgui::EditablePanel);
public:

	// OC
	using string_list_t = std::vector<std::string>;
	using string_image_list_t = std::unordered_map<std::string, string_list_t>;
	//
	// Construction
	CMapLoadBG(char const* panelName);
	~CMapLoadBG();

	// OC
	void SetMap(const std::string& sMap = "");
	void Reset();
	//

	bool FindLoadingDialogBarHandle(void);
	int iLoadingBarHandle;
	float progress;

	void OnMessage(const KeyValues* params, vgui::VPANEL fromPanel);
	void OnThink(void);

protected:
	void ApplySchemeSettings(vgui::IScheme* pScheme);

private:
	vgui::ImagePanel* m_pBackground;
#ifdef ENABLE_CUSTOM_LOADING_BAR
	vgui::ContinuousProgressBar* m_pProgressBar;
	vgui::Label* m_pProgressPercentage;
	//#if HL2MP
	//	vgui::Label *m_pProgressStatusMessage; // the mp-specific message about connecting, downloading
	//#endif
#endif
#ifdef ENABLE_LOADING_TIP
	vgui::Label* m_pLoadingTip;
	// OC
	int iNumberTips;
	//
#endif
#ifdef ENABLE_CUSTOM_LOADING_WHEEL
	vgui::CircularProgressBar* m_pProgressWheel;
#endif
	// OC
	string_image_list_t m_hImages;
	//
};

/*
ProgressBar directions are:
0 - left to right (or 'east')
1 - right to left ('west')
2 - bottom to top ('north')
3 - top to bottom ('south')
*/

#endif	// !MAPLOAD_BACKGROUND_H
