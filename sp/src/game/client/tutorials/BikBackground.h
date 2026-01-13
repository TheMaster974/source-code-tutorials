//========= Copyright Jorge "BSVino" Rodriguez, All rights reserved. Edited by WadDelz. ============//
//
// Purpose: Adds an animated background menu, thanks to WadDelZ for the improvements!
//
//==================================================================================================//
#ifndef MODBASE_BACKGROUND_H
#define MODBASE_BACKGROUND_H

#ifdef _WIN32
#pragma once
#endif

#include <vguitextwindow.h>
#include "video/ivideoservices.h"
#include <materialsystem/MaterialSystemUtil.h>

//--------------------------------------------------------------------------------------------------------------------
//													BIK BACKGROUND STUFF
//--------------------------------------------------------------------------------------------------------------------

//background load type
enum class ModBase_BikBackgroundLoadType
{
	Random,
	FullyRandom,
	UseConvar,
	ComparePreviousMap,
};

//bik background song info
struct BikBackground_SongInfo_t
{
	const char* songname = nullptr;
	float songvolume = 0.0f;
};

//boolean used to check if the main menu background should use a bik video or the
//normal source engine real time background
extern bool g_PModBase_BikBackground_bUse;

//the background load type
extern ModBase_BikBackgroundLoadType g_PModBase_BikBackground_LoadType;

//boolean used to check if the main menu background should use a texture as the logo
extern bool g_PModBase_BikBackground_bUseTextureLogo;
extern const char* g_PModBase_BikBackground_bUseTextureLogoName;
extern const char* g_PModBase_BikBackground_bUseTextureLogoColor;
extern int g_PModBase_BikBackground_bUseTextureLogoPosX;
extern int g_PModBase_BikBackground_bUseTextureLogoPosY;
extern int g_PModBase_BikBackground_bUseTextureLogoWide;
extern int g_PModBase_BikBackground_bUseTextureLogoTall;

//the convar name that's convar value will be used to check the background menu
extern const char* g_PModBase_BikBackground_szConvarName;

//the structure that holds all the menu background items
struct PModBase_BikBackgroundHolder
{
	const char* bik_video_name = nullptr;			//background bik video name
	CUtlVector<BikBackground_SongInfo_t*> Songs;	//chooses a random song out of all these songs.
	int minvalue = 0;								//min value convar needs to be for this background to play
	const char* MapCriteria = nullptr;				//ComparePreviousMap ONLY: The criteria the previous map name must have for this background to play.
	bool IsDefaultIfPrevMapIsEmpty = false;
};

//the array of bik background holders
extern CUtlVector<PModBase_BikBackgroundHolder*> g_PModBase_BikBackground_Holders;

extern bool g_PModBase_BIsGamepadUI;

void BikBackgroundDebugMsg(const char* msg, ...);

//--------------------------------------------------------------------------------------------------------------------
//													BIK BACKGROUND CLASS
//--------------------------------------------------------------------------------------------------------------------

class ModBase_BikMenu : public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE(ModBase_BikMenu, vgui::Panel);

	ModBase_BikMenu(vgui::Panel* parent, const char* pElementName);
	~ModBase_BikMenu();

public:
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

	bool IsVideoPlaying();
	void StartVideo();
	void StopVideo();

	void GetPanelPos(int& xpos, int& ypos);

	void Paint();
	bool BeginPlayback(const char* pFilename);
	void ReleaseVideo();
	void DoModal();

private:
	bool m_bLoaded;
	bool m_bToolsMode;
	bool m_bPaintVideo;

private:
	//for drawing the menu text
	int m_DrawLogoX1, m_DrawLogoY1;
	int m_DrawLogoX2, m_DrawLogoY2;
	color32 m_DrawLogoColor1;
	color32 m_DrawLogoColor2;
	const wchar_t* m_DrawLogoText1;
	const wchar_t* m_DrawLogoText2;
	unsigned long long m_DrawLogoFont;

	//for optionally drawing the main menu image icon
	int m_ImageLogoTextureId;

public:
	bool m_bInit;

protected:
	IVideoMaterial* m_VideoMaterial;
	IMaterial* m_pMaterial;

	int m_nPlaybackHeight; // Calculated to address ratio changes
	int m_nPlaybackWidth;
	char m_szExitCommand[MAX_PATH]; // This call is fired at the engine when the video finishes or is interrupted

	float m_flU; // UV ranges for video on its sheet
	float m_flV;

	bool m_bAllowAlternateMedia;
};

#endif // MODBASE_BACKGROUND_H