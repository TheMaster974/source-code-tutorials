//========= Copyright Jorge "BSVino" Rodriguez, All rights reserved. Edited by WadDelz. ============//
//
// Purpose: Adds an animated background menu, thanks to WadDelZ for the improvements!
//
//==================================================================================================//

#include "cbase.h"
#include <cdll_client_int.h>
#include <ienginevgui.h>
#include <KeyValues.h>
#include "iclientmode.h"
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include "clienteffectprecachesystem.h"
#include "tier0/icommandline.h"
#include "engine/IEngineSound.h"
#include "fmtstr.h"
#include "baseviewport.h"

//mod info
#include "ModInfo.h"
#include "BikBackground.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//boolean used to check if the main menu background should use a bik video or the
//normal source engine real time background
bool g_PModBase_BikBackground_bUse = false;

//the background load type
ModBase_BikBackgroundLoadType g_PModBase_BikBackground_LoadType = ModBase_BikBackgroundLoadType::UseConvar;

//boolean used to check if the main menu background should use a texture as the logo
bool g_PModBase_BikBackground_bUseTextureLogo = false;
const char* g_PModBase_BikBackground_bUseTextureLogoName = nullptr;
const char* g_PModBase_BikBackground_bUseTextureLogoColor = nullptr;
int g_PModBase_BikBackground_bUseTextureLogoPosX = 0;
int g_PModBase_BikBackground_bUseTextureLogoPosY = 0;
int g_PModBase_BikBackground_bUseTextureLogoWide = 0;
int g_PModBase_BikBackground_bUseTextureLogoTall = 0;

//the convar name that's convar value will be used to check the background menu
const char* g_PModBase_BikBackground_szConvarName = nullptr;

//the array of bik background holders
CUtlVector<PModBase_BikBackgroundHolder*> g_PModBase_BikBackground_Holders;

bool g_PModBase_BIsGamepadUI = false;	//the gamepadui state

//-------------------------------------------------------------------------
// Purpose: background debug message
//-------------------------------------------------------------------------
void BikBackgroundDebugMsg(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);

	char buf[2048];
	Q_vsnprintf(buf, sizeof(buf), msg, args);
	ConColorMsg(Color(105, 250, 200, 255), "<Bik Background> %s", buf);

	va_end(args);
}

//ADVANCED STRING CRITERIA MATCHING FUNCTION


//-----------------------------------------------------------------------------------------
// 
// The StringMatchesCriteria function is a very advanced string matching utility.
//
// The first string is the string that you want the criteria to match.
// The second string is the criteria string.
// The bool inside the options struct specifies if the strings should do a case sensitive search or not.
//
// There are a couple of features the StringMatchesCriteria function has:
//
//  1. Wildcards '*' that can match any sequence of characters.
//       - For example, if your string is "testchamber" and your criteria is "test*", it will match because '*' can represent any characters after "test".
//       - You can also put '*' in the middle, like "test*er", and it will match "testchamber" because '*' matches "chamb".
//       - Multiple '*' in a row are treated the same as a single '*', so "te**st*" is equivalent to "test*".
//
//  2. Special wildcards '[...]' that can match either:
//       - Numeric ranges like [0-10] or [20-30] (NOTE that if you had, say testchmb_a_001 and you did testchmb_a_[0-10] as your criteria, it would work because it ignores the leading 0's)
//       - Alphabetic ranges like [a-z] or [A-Z]
//       - Multiple choices separated by '|' like [error|fail|1-5]
// 
//  3. Optional strings '{...}' to check for optional strings. The inside uses the same logic as inside the [] special wildcards.
//       - So say i have testchmb01 as my string but i didnt know if the string was testchmb01 or testchmb_01 then i could do testchmb{_}01. That would allow an _ to be or not to be bettween testchmb and 01.
// 
//  4. Case sensitivity control with the 4th parameter
// 
//	5. Multiple checks. If you want to do multiple checks for 1 string, Then seperate each criteria with a |. For example i could do 'testchmb_a_15|escape_[0-2]' and it will see if the string matches either one of the criteria's
//
// Here are some examples of (string) and (criteria) inputs that would return TRUE:
//
//	string:						criteria:
//	 testchamber				 testchamber
//	 testchamber				 test*
//	 testchamber				 test*er
//	 testchamber				 *er
//	 testchmb_a_01				 testchmb_a_[0-10]
//	 testchmb_a_01				 testchmb_a_[00|01]
//	 testchmb_a_05				 testchmb_a_[01-10]
//	 testchmb_a_1				 testchmb_a_{0-2}
//	 testchmb_b_1				 testchmb_{a-c}_1
//	 testchmb_c_1				 testchmb_{a-c}_1
//	 test_3					 test_[1-5]
//	 test_a					 test_[a-c]
//	 test01					 test[1-2]
//	 test02					 test{_01|_02}
//	 test_gamma				 test_{alpha|beta|gamma}
//
// Now here are some examples of (string) and (criteria) inputs that would return FALSE:
//
//	string:						criteria:
//	 testchamber				 testchambe
//	 testchamber				 *ers
//	 testchmb_a_01				 testchmb_a_[00|02]
//	 testchmb_a_11				 testchmb_a_[0-10]
//	 testchmb_d_1				 testchmb_{a-c}_1
//	 testchmb_z_1				 testchmb_{a-c}_1
//	 test_a_9					 test_a_{3-7}
//	 test_a_00					 test_a_{1-5}
//	 testa_6					 test[a-c]_{1-5}
//	 testb_9					 test[a-c]_{1-5}
//	 testdelta				 test_{alpha|beta|gamma}
//	 test011					 test[1-10]
//	 abc						 xyz*
//	 51						 [1-50]
//	 testA						 test[a-z]
//
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Compare two characters with optional case sensitivity
//-----------------------------------------------------------------------------------------
static bool CharEquals(char a, char b, bool caseSensitive)
{
	if (caseSensitive) return a == b;
	return tolower((unsigned char)a) == tolower((unsigned char)b);
}

//-----------------------------------------------------------------------------------------
// Check if input has a literal control character that should prevent special matching
//-----------------------------------------------------------------------------------------
static bool InputHasControlChar(const char* s)
{
	return *s == '[' || *s == ']' || *s == '{' || *s == '}' || *s == '*';
}

// Forward declaration
bool StringMatchesCriteria(const char* s1, const char* pattern, bool CaseSensitive = false);

//-----------------------------------------------------------------------------------------
// Helper: Match numeric or alphabetic range like 0-10 or a-z, returns chars consumed
//-----------------------------------------------------------------------------------------
static size_t MatchRangeAndAdvance(const char* s1, const char* token)
{
	const char* dash = strchr(token, '-');
	if (!dash) return 0;

	char startBuf[64] = { 0 };
	char endBuf[64] = { 0 };

	size_t startLen = dash - token;
	if (startLen >= sizeof(startBuf)) startLen = sizeof(startBuf) - 1;
	strncpy_s(startBuf, sizeof(startBuf), token, startLen);
	strcpy_s(endBuf, sizeof(endBuf), dash + 1);

	if (isdigit(startBuf[0]) && isdigit(endBuf[0]))
	{
		size_t i = 0;
		while (isdigit(s1[i])) i++;
		if (i == 0) return 0;

		char valBuf[32];
		strncpy_s(valBuf, sizeof(valBuf), s1, i);
		valBuf[i] = '\0';
		int val = atoi(valBuf);

		int start = atoi(startBuf);
		int end = atoi(endBuf);
		if (start > end) { int tmp = start; start = end; end = tmp; }

		// Handle leading zero length consistency
		size_t lenRangeMin = strlen(startBuf);
		size_t lenRangeMax = strlen(endBuf);

		if (val >= start && val <= end)
		{
			// Accept shorter/longer zero-padded values
			if (i >= lenRangeMin && i <= lenRangeMax)
				return i;
			// Or if within range regardless of leading zeros
			return i;
		}
		return 0;
	}
	else if (isalpha(startBuf[0]) && isalpha(endBuf[0]))
	{
		char val = s1[0];
		char start = startBuf[0];
		char end = endBuf[0];
		if (start > end) { char tmp = start; start = end; end = tmp; }
		if (val >= start && val <= end) return 1;
		return 0;
	}

	return 0;
}

//-----------------------------------------------------------------------------------------
// Helper: Match optional string {...}
//-----------------------------------------------------------------------------------------
static bool MatchOptionalString(const char* s1, const char** patternPtr, bool CaseSensitive = false)
{
	const char* pattern = *patternPtr + 1; // skip '{'
	char buffer[256] = { 0 };
	int bufPos = 0;

	while (*pattern && *pattern != '}')
	{
		if (bufPos < (int)(sizeof(buffer) - 1)) buffer[bufPos++] = *pattern++;
		else break;
	}
	buffer[bufPos] = 0;
	if (*pattern == '}') pattern++;

	char local[256];
	if (strcpy_s(local, sizeof(local), buffer) != 0) return false;

	char* context = NULL;
	char* token = strtok_s(local, "|", &context);

	while (token)
	{
		size_t len = strlen(token);
		size_t consumed = 0;

		if (strchr(token, '-') && !InputHasControlChar(s1))
			consumed = MatchRangeAndAdvance(s1, token);

		if (consumed > 0)
		{
			*patternPtr = pattern;
			if (StringMatchesCriteria(s1 + consumed, pattern, CaseSensitive))
				return true;
		}
		else
		{
			bool match = (CaseSensitive) ? (strncmp(s1, token, len) == 0) : (_strnicmp(s1, token, len) == 0);
			if (match)
			{
				*patternPtr = pattern;
				if (StringMatchesCriteria(s1 + len, pattern, CaseSensitive))
					return true;
			}
		}

		token = strtok_s(NULL, "|", &context);
	}

	*patternPtr = pattern;
	return StringMatchesCriteria(s1, pattern, CaseSensitive);
}

//-----------------------------------------------------------------------------------------
// Helper: Match special wildcard [...]
//-----------------------------------------------------------------------------------------
static bool MatchSpecialWildcard(const char* s1, const char** patternPtr, bool CaseSensitive)
{
	const char* pattern = *patternPtr + 1; // skip '['
	char buffer[256] = { 0 };
	int bufPos = 0;

	while (*pattern && *pattern != ']')
	{
		if (bufPos < (int)(sizeof(buffer) - 1)) buffer[bufPos++] = *pattern++;
		else break;
	}
	buffer[bufPos] = 0;
	if (*pattern == ']') pattern++;

	char local[256];
	if (strcpy_s(local, sizeof(local), buffer) != 0) return false;

	char* context = NULL;
	char* token = strtok_s(local, "|", &context);

	while (token)
	{
		size_t len = strlen(token);
		size_t consumed = 0;

		if (strchr(token, '-') && !InputHasControlChar(s1))
			consumed = MatchRangeAndAdvance(s1, token);

		if (consumed > 0)
		{
			*patternPtr = pattern;
			if (StringMatchesCriteria(s1 + consumed, pattern, CaseSensitive))
				return true;
		}
		else
		{
			bool match = (CaseSensitive) ? (strncmp(s1, token, len) == 0) : (_strnicmp(s1, token, len) == 0);
			if (match)
			{
				*patternPtr = pattern;
				if (StringMatchesCriteria(s1 + len, pattern, CaseSensitive))
					return true;
			}
		}

		token = strtok_s(NULL, "|", &context);
	}

	return false;
}

//-----------------------------------------------------------------------------------------
// Main matcher
//-----------------------------------------------------------------------------------------
bool StringMatchesCriteria(const char* s1, const char* pattern, bool CaseSensitive)
{
	// Top-level | operator handling for both s1 and pattern
	const char* pipe1 = strchr(s1, '|');
	const char* pipe2 = strchr(pattern, '|');

	if (pipe2)
	{
		char left[512], right[512];
		size_t leftLen = pipe2 - pattern;
		strncpy_s(left, sizeof(left), pattern, leftLen);
		strcpy_s(right, sizeof(right), pipe2 + 1);

		if (StringMatchesCriteria(s1, left, CaseSensitive) || StringMatchesCriteria(s1, right, CaseSensitive))
			return true;
		return false;
	}

	if (pipe1)
	{
		char leftS1[512], rightS1[512];
		size_t leftLen = pipe1 - s1;
		strncpy_s(leftS1, sizeof(leftS1), s1, leftLen);
		strcpy_s(rightS1, sizeof(rightS1), pipe1 + 1);

		if (StringMatchesCriteria(leftS1, pattern, CaseSensitive) || StringMatchesCriteria(rightS1, pattern, CaseSensitive))
			return true;
		return false;
	}

	while (*pattern)
	{
		if (*pattern == '*' && !InputHasControlChar(s1))
		{
			while (*pattern == '*') pattern++;
			if (*pattern == 0) return true;
			for (size_t i = 0; s1[i]; i++)
				if (StringMatchesCriteria(s1 + i, pattern, CaseSensitive))
					return true;
			return false;
		}

		if (*pattern == '{' && !InputHasControlChar(s1))
			return MatchOptionalString(s1, &pattern, CaseSensitive);

		if (*pattern == '[' && !InputHasControlChar(s1))
			return MatchSpecialWildcard(s1, &pattern, CaseSensitive);

		if (!CharEquals(*s1, *pattern, CaseSensitive))
			return false;

		s1++;
		pattern++;
	}

	return *s1 == 0;
}




//What this is is a convar that saves the previous map that was played. This can be used for the ComparePreviousMap or ComparePreviousMapRandom bik background load types.
ConVar modbase_previous_map_played("modbase_previous_map_played", "", FCVAR_ARCHIVE, "The stores the previous map played. It is Used for bik background loading.");

//the actuall system
class CSetPreviousPlayedMapSystem : public CAutoGameSystem
{
public:
	virtual void LevelShutdownPreEntity()
	{
		if (!engine->IsLevelMainMenuBackground())
			modbase_previous_map_played.SetValue(MapName());
	}
};
static CSetPreviousPlayedMapSystem s_SetPrevMapSystem;




//for map_background command
static bool bOverrideVideo = false;
static const char* bOverrideVideoBuf = nullptr;
static const char* bOverrideVideoSongBuf = nullptr;
static float flVolume = nullptr;

//current temp value for Random mode
static int CurrTempRandValue = 0;

//-----------------------------------------------------------------------------
// Purpose: Checks which chapter is unlocked by checking the sv_unlockedchapters convar
//			and get the mainmenu background name from the BackgroundNames array using
//			the minvalue of the background item and the convar value.
//-----------------------------------------------------------------------------
static const char* GetBackgroundName(const int& value)
{
	if (bOverrideVideo && bOverrideVideoBuf)
		return bOverrideVideoBuf;

	if (g_PModBase_BikBackground_Holders.Count() <= 0)
		return nullptr;

	//check the load type
	if (g_PModBase_BikBackground_LoadType == ModBase_BikBackgroundLoadType::Random)
	{
		if (CurrTempRandValue < 0 || CurrTempRandValue >= g_PModBase_BikBackground_Holders.Count())
			return nullptr;

		return g_PModBase_BikBackground_Holders[CurrTempRandValue]->bik_video_name;
	}
	else if (g_PModBase_BikBackground_LoadType == ModBase_BikBackgroundLoadType::FullyRandom)
	{
		return g_PModBase_BikBackground_Holders[random->RandomInt(0, g_PModBase_BikBackground_Holders.Count() - 1)]->bik_video_name;
	}
	else if (g_PModBase_BikBackground_LoadType == ModBase_BikBackgroundLoadType::ComparePreviousMap)
	{
		//use CurrTempRandValue as a default index
		CurrTempRandValue = random->RandomInt(0, g_PModBase_BikBackground_Holders.Count() - 1);

		//is the previous map name empty
		bool IsPrevMapEmpty = Q_strlen(modbase_previous_map_played.GetString()) <= 0;

		//find the array item
		for (int i = 0; i < g_PModBase_BikBackground_Holders.Count(); i++)
		{
			//get the bik background holder
			PModBase_BikBackgroundHolder* holder = g_PModBase_BikBackground_Holders[i];
			if (!holder)
				continue;

			//check this first
			if (IsPrevMapEmpty)
			{
				//check default
				if (holder->IsDefaultIfPrevMapIsEmpty)
					CurrTempRandValue = i;

				continue;
			}

			//check criteria
			if (StringMatchesCriteria(modbase_previous_map_played.GetString(), holder->MapCriteria))
			{
				//set CurrTempRandValue then return
				CurrTempRandValue = i;
				return holder->bik_video_name;
			}

			//is default
			if (holder->minvalue)
				CurrTempRandValue = i;
		}

		//check CurrTempRandValue
		if (CurrTempRandValue >= 0 && CurrTempRandValue < g_PModBase_BikBackground_Holders.Count())
			return g_PModBase_BikBackground_Holders[CurrTempRandValue]->bik_video_name;

		return nullptr;
	}
	else
	{
		//index
		int index = -1;
		int minvalue = INT_MIN;		//safe number

		//find the array item
		for (int i = 0; i < g_PModBase_BikBackground_Holders.Count(); i++)
		{
			//get the bik background holder
			PModBase_BikBackgroundHolder* holder = g_PModBase_BikBackground_Holders[i];
			if (!holder)
				continue;

			if (value >= holder->minvalue && holder->minvalue > minvalue)
			{
				minvalue = holder->minvalue;
				index = i;
			}
		}

		//find the holder
		PModBase_BikBackgroundHolder* holder = nullptr;

		//get the holder
		if (index != -1)
			holder = g_PModBase_BikBackground_Holders[index];
		else if (g_PModBase_BikBackground_Holders.Count() > 0)
			holder = g_PModBase_BikBackground_Holders[0];

		//check the holder
		if (!holder)
			return nullptr;

		return holder->bik_video_name;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Gets the name and volume of the song that should play for this background
//-----------------------------------------------------------------------------
static BikBackground_SongInfo_t* GetBackgroundSongInfo(const int& value)
{
	//return this
	if (g_PModBase_BikBackground_Holders.Count() <= 0)
		return nullptr;

	//check the load type
	if (g_PModBase_BikBackground_LoadType == ModBase_BikBackgroundLoadType::Random)
	{
		//bounds checking
		if (CurrTempRandValue < 0 || CurrTempRandValue >= g_PModBase_BikBackground_Holders.Count())
			return nullptr;

		//get the song info
		CUtlVector<BikBackground_SongInfo_t*>& Songs = g_PModBase_BikBackground_Holders[CurrTempRandValue]->Songs;

		//check songs size
		if (Songs.Count() <= 0)
			return nullptr;

		//return random song
		return Songs[random->RandomInt(0, Songs.Count() - 1)];
	}
	else if (g_PModBase_BikBackground_LoadType == ModBase_BikBackgroundLoadType::FullyRandom)
	{
		//get the song info
		CUtlVector<BikBackground_SongInfo_t*>& Songs = g_PModBase_BikBackground_Holders[random->RandomInt(0, g_PModBase_BikBackground_Holders.Count() - 1)]->Songs;

		//check songs size
		if (Songs.Count() <= 0)
			return nullptr;

		//return random song
		return Songs[random->RandomInt(0, Songs.Count() - 1)];
	}
	else if (g_PModBase_BikBackground_LoadType == ModBase_BikBackgroundLoadType::ComparePreviousMap)
	{
		//check CurrTempRandValue
		if (CurrTempRandValue < 0 || CurrTempRandValue >= g_PModBase_BikBackground_Holders.Count())
			return nullptr;

		//get the song info
		CUtlVector<BikBackground_SongInfo_t*>& Songs = g_PModBase_BikBackground_Holders[CurrTempRandValue]->Songs;

		//check songs size
		if (Songs.Count() <= 0)
			return nullptr;

		//return a random song
		return Songs[random->RandomInt(0, Songs.Count() - 1)];
	}
	else
	{
		//index
		int index = -1;
		int minvalue = INT_MIN;		//safe number

		//find the array item
		for (int i = 0; i < g_PModBase_BikBackground_Holders.Count(); i++)
		{
			//get the bik background holder
			PModBase_BikBackgroundHolder* holder = g_PModBase_BikBackground_Holders[i];
			if (!holder)
				continue;

			if (value >= holder->minvalue && holder->minvalue > minvalue)
			{
				minvalue = holder->minvalue;
				index = i;
			}
		}

		//find the holder
		PModBase_BikBackgroundHolder* holder = nullptr;

		//get the holder
		if (index != -1)
			holder = g_PModBase_BikBackground_Holders[index];
		else if (g_PModBase_BikBackground_Holders.Count() > 0)
			holder = g_PModBase_BikBackground_Holders[0];

		//check the holder
		if (!holder)
			return nullptr;

		//get the song info
		CUtlVector<BikBackground_SongInfo_t*>& Songs = holder->Songs;

		//check songs size
		if (Songs.Count() <= 0)
			return nullptr;

		return Songs[random->RandomInt(0, Songs.Count() - 1)];
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
ModBase_BikMenu::ModBase_BikMenu(vgui::Panel* parent, const char* pElementName) : vgui::Panel(NULL, "ModBase_BikMenu")
{
	SetParent(enginevgui->GetPanel(PANEL_GAMEUIDLL));
	SetBuildModeEditable(false);
	SetVisible(false);
	SetPaintEnabled(false);
	SetProportional(true);
	SetKeyBoardInputEnabled(false);
	SetPaintBorderEnabled(false);
	m_VideoMaterial = NULL;
	m_nPlaybackWidth = 0;
	m_nPlaybackHeight = 0;

	m_bToolsMode = false;
	m_bLoaded = false;

	//for the main menu text
	m_DrawLogoX1 = 0;
	m_DrawLogoY1 = 0;

	m_DrawLogoX2 = 0;
	m_DrawLogoY2 = 0;

	m_DrawLogoText1 = nullptr;
	m_DrawLogoText2 = nullptr;

	m_DrawLogoColor1 = color32{ 255, 255, 255, 255 };
	m_DrawLogoColor2 = color32{ 255, 255, 255, 128 };

	m_DrawLogoFont = INVALID_FONT;

	//for the optional main menu text
	m_ImageLogoTextureId = 0;

	m_bInit = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ModBase_BikMenu::~ModBase_BikMenu()
{
	//reset CurrTempRandValue
	CurrTempRandValue = 0;

	//delete the background image
	if (m_ImageLogoTextureId)
		vgui::surface()->DeleteTextureByID(m_ImageLogoTextureId);

	//delete the video
	ReleaseVideo();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ModBase_BikMenu::ApplySchemeSettings(IScheme* pScheme)
{
	//this function gets called like 10000000 times a second. so fix it
	if (m_bInit)
		return;

	m_bInit = true;

	SetPos(-1, -1);
	SetSize(ScreenWidth() + 2, ScreenHeight() + 2);
	SetZPos(100);

	//1stly see if the image as the icon is being used
	if (g_PModBase_BikBackground_bUseTextureLogo)
	{
		//if so then set the texture
		m_ImageLogoTextureId = vgui::surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile(m_ImageLogoTextureId, g_PModBase_BikBackground_bUseTextureLogoName, true, false);

		//get the right offset
		//just use these m_DrawLogo*1 variables to save memory because otherwise they would be unused
		m_DrawLogoX1 = vgui::scheme()->GetProportionalScaledValue(g_PModBase_BikBackground_bUseTextureLogoPosX);
		m_DrawLogoY1 = vgui::scheme()->GetProportionalScaledValue(g_PModBase_BikBackground_bUseTextureLogoPosY);

		//just use these m_DrawLogo*1 variables to save memory because otherwise they would be unused
		m_DrawLogoX2 = vgui::scheme()->GetProportionalScaledValue(g_PModBase_BikBackground_bUseTextureLogoWide);
		m_DrawLogoY2 = vgui::scheme()->GetProportionalScaledValue(g_PModBase_BikBackground_bUseTextureLogoTall);

		//just use these m_DrawLogoColor1 variable to save memory because otherwise they would be unused
		UTIL_StringToColor32(&m_DrawLogoColor1, g_PModBase_BikBackground_bUseTextureLogoColor);

		return;
	}

	//get the client scheme
	vgui::IScheme* scheme = vgui::scheme()->GetIScheme(vgui::scheme()->GetScheme("ClientScheme"));
	if (!scheme)
	{
		BikBackgroundDebugMsg("Background BIK Video Error: Failed to get ClientScheme for '" __FUNCSIG__ "' Main menu logo will NOT Show\n");
		return;
	}

	//get x and y values for the logo's
	m_DrawLogoX1 = vgui::scheme()->GetProportionalScaledValue(atoi(scheme->GetResourceString("Main.Title1.X")));
	m_DrawLogoY1 = vgui::scheme()->GetProportionalScaledValue(atoi(scheme->GetResourceString("Main.Title1.Y")));
	m_DrawLogoX2 = vgui::scheme()->GetProportionalScaledValue(atoi(scheme->GetResourceString("Main.Title2.X")));
	m_DrawLogoY2 = vgui::scheme()->GetProportionalScaledValue(atoi(scheme->GetResourceString("Main.Title2.Y")));

	//get the color for the logos
	UTIL_StringToColor32(&m_DrawLogoColor1, scheme->GetResourceString("Main.Title1.Color"));
	UTIL_StringToColor32(&m_DrawLogoColor2, scheme->GetResourceString("Main.Title2.Color"));

	//get the font
	m_DrawLogoFont = scheme->GetFont("ClientTitleFont", true);

	//get the texts
	m_DrawLogoText1 = ModInfo().GetGameTitle();
	m_DrawLogoText2 = ModInfo().GetGameTitle2();

	//if the texts are empty then make them nullptr to save having to draw them in tha Paint function.
	m_DrawLogoText1 = wcslen(m_DrawLogoText1) == 0 ? nullptr : m_DrawLogoText1;
	m_DrawLogoText2 = wcslen(m_DrawLogoText2) == 0 ? nullptr : m_DrawLogoText2;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ModBase_BikMenu::IsVideoPlaying()
{
	return m_bPaintVideo;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ModBase_BikMenu::StartVideo()
{
	m_bToolsMode = (IsPC() && (CommandLine()->CheckParm("-tools") != NULL)) ? true : false;

	SetVisible(true);
	SetPaintEnabled(true);

	ConVarRef gConvar(g_PModBase_BikBackground_szConvarName);

	if (BeginPlayback(CFmtStr("%s.bik", GetBackgroundName(gConvar.GetInt()))))
		m_bLoaded = true;

	//debug message
	BikBackgroundDebugMsg("Background BIK Video: Starting main menu video!\n");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ModBase_BikMenu::StopVideo()
{
	//SetVisible( false ); 
	//SetPaintEnabled( false );
	ReleaseVideo();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ModBase_BikMenu::GetPanelPos(int& xpos, int& ypos)
{
	vgui::ipanel()->GetAbsPos(GetVPanel(), xpos, ypos);
}

//if you want to play a video using the playvideo command and the video material for this is still active then it
//will cause the video to look AWFULL. so do this
bool bDidReleaseVideo = false;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ModBase_BikMenu::Paint(void)
{
	if (m_bToolsMode)
		return;

	if (engine->IsConnected())
	{
		if (!bDidReleaseVideo)
		{
			ReleaseVideo();
			bDidReleaseVideo = true;
		}

		return;
	}

	bDidReleaseVideo = false;

	if (!m_bLoaded)
		return;

	if (m_bPaintVideo)
	{
		m_nPlaybackHeight = ScreenHeight() + 2;
		m_nPlaybackWidth = ScreenWidth() + 2;

		// No video to play, so do nothing
		if (m_VideoMaterial == NULL)
			return;

		// Update our frame
		if (m_VideoMaterial->Update() == false)
			return;

		// Sit in the "center"
		int xpos, ypos;
		GetPanelPos(xpos, ypos);

		// Draw the polys to draw this out
		CMatRenderContextPtr pRenderContext(materials);

		pRenderContext->MatrixMode(MATERIAL_VIEW);
		pRenderContext->PushMatrix();
		pRenderContext->LoadIdentity();

		pRenderContext->MatrixMode(MATERIAL_PROJECTION);
		pRenderContext->PushMatrix();
		pRenderContext->LoadIdentity();

		pRenderContext->Bind(m_pMaterial, NULL);

		CMeshBuilder meshBuilder;
		IMesh* pMesh = pRenderContext->GetDynamicMesh(true);
		meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

		float flLeftX = xpos;
		float flRightX = xpos + (m_nPlaybackWidth - 1);

		float flTopY = ypos;
		float flBottomY = ypos + (m_nPlaybackHeight - 1);

		// Map our UVs to cut out just the portion of the video we're interested in
		float flLeftU = 0.0f;
		float flTopV = 0.0f;

		// We need to subtract off a pixel to make sure we don't bleed
		float flRightU = m_flU - (1.0f / (float)m_nPlaybackWidth);
		float flBottomV = m_flV - (1.0f / (float)m_nPlaybackHeight);

		// Get the current viewport size
		int vx, vy, vw, vh;
		pRenderContext->GetViewport(vx, vy, vw, vh);

		// Map from screen pixel coords to -1..1
		flRightX = FLerp(-1, 1, 0, vw, flRightX);
		flLeftX = FLerp(-1, 1, 0, vw, flLeftX);
		flTopY = FLerp(1, -1, 0, vh, flTopY);
		flBottomY = FLerp(1, -1, 0, vh, flBottomY);

		float alpha = ((float)GetFgColor()[3] / 255.0f);

		for (int corner = 0; corner < 4; corner++)
		{
			bool bLeft = (corner == 0) || (corner == 3);
			meshBuilder.Position3f((bLeft) ? flLeftX : flRightX, (corner & 2) ? flBottomY : flTopY, 0.0f);
			meshBuilder.Normal3f(0.0f, 0.0f, 1.0f);
			meshBuilder.TexCoord2f(0, (bLeft) ? flLeftU : flRightU, (corner & 2) ? flBottomV : flTopV);
			meshBuilder.TangentS3f(0.0f, 1.0f, 0.0f);
			meshBuilder.TangentT3f(1.0f, 0.0f, 0.0f);
			meshBuilder.Color4f(1.0f, 1.0f, 1.0f, alpha);
			meshBuilder.AdvanceVertex();
		}

		meshBuilder.End();
		pMesh->Draw();

		pRenderContext->MatrixMode(MATERIAL_VIEW);
		pRenderContext->PopMatrix();

		pRenderContext->MatrixMode(MATERIAL_PROJECTION);
		pRenderContext->PopMatrix();

		//dont draw whilst loading a level or if the gamepadui is enabled
		if (g_PModBase_BIsGamepadUI || engine->IsDrawingLoadingImage())
			return;

		//if using texture logos then draw the texture logo at the offset
		if (g_PModBase_BikBackground_bUseTextureLogo)
		{
			surface()->DrawSetColor(m_DrawLogoColor1.r, m_DrawLogoColor1.g, m_DrawLogoColor1.b, m_DrawLogoColor1.a);
			surface()->DrawSetTexture(m_ImageLogoTextureId);
			surface()->DrawTexturedRect(m_DrawLogoX1, m_DrawLogoY1, m_DrawLogoX1 + m_DrawLogoX2, m_DrawLogoY1 + m_DrawLogoY2);
			return;
		}

		//draw the 1st logo text if there is one
		if (m_DrawLogoText1)
		{
			surface()->DrawSetTextFont(m_DrawLogoFont);
			surface()->DrawSetTextColor(m_DrawLogoColor1.r, m_DrawLogoColor1.g, m_DrawLogoColor1.b, m_DrawLogoColor1.a);
			surface()->DrawSetTextPos(m_DrawLogoX1, m_DrawLogoY1);
			surface()->DrawPrintText(m_DrawLogoText1, wcslen(m_DrawLogoText1));
		}

		//draw the 2nd logo text if there is one
		if (m_DrawLogoText2)
		{
			surface()->DrawSetTextFont(m_DrawLogoFont);
			surface()->DrawSetTextColor(m_DrawLogoColor2.r, m_DrawLogoColor2.g, m_DrawLogoColor2.b, m_DrawLogoColor2.a);
			surface()->DrawSetTextPos(m_DrawLogoX2, m_DrawLogoY2);
			surface()->DrawPrintText(m_DrawLogoText2, wcslen(m_DrawLogoText2));
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ModBase_BikMenu::BeginPlayback(const char* pFilename)
{
	// Need working video services
	if (g_pVideo == NULL)
		return false;

	// Destroy any previously allocated video
	if (m_VideoMaterial != NULL)
	{
		g_pVideo->DestroyVideoMaterial(m_VideoMaterial);
		m_VideoMaterial = NULL;
	}

	// Create new Video material
	m_VideoMaterial = g_pVideo->CreateVideoMaterial("VideoMaterial", pFilename, "GAME",
		VideoPlaybackFlags::DEFAULT_MATERIAL_OPTIONS,
		VideoSystem::DETERMINE_FROM_FILE_EXTENSION, m_bAllowAlternateMedia);
	if (m_VideoMaterial == NULL)
		return false;

	m_bPaintVideo = true;

	m_VideoMaterial->SetLooping(true);

	int nWidth, nHeight;
	m_VideoMaterial->GetVideoImageSize(&nWidth, &nHeight);
	m_VideoMaterial->GetVideoTexCoordRange(&m_flU, &m_flV);
	m_pMaterial = m_VideoMaterial->GetMaterial();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ModBase_BikMenu::ReleaseVideo()
{
	m_bPaintVideo = false;
	m_bLoaded = false;

	// Destroy any previously allocated video
	// Shut down this video, destroy the video material
	if (g_pVideo->IsVideoSystemAvailable(VideoSystem_t::BINK) && g_pVideo != NULL && m_VideoMaterial != NULL)
	{
		g_pVideo->DestroyVideoMaterial(m_VideoMaterial);
		m_VideoMaterial = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Find the disconnect command, and rename it...
//-----------------------------------------------------------------------------
CON_COMMAND(__disconnect, "Disconnect game from server.")
{
	//check the +map parameter
	static bool bFirstTime = true;
	if (bFirstTime && CommandLine()->CheckParm("+map") != nullptr && !g_PModBase_BIsGamepadUI)
	{
		bFirstTime = false;
		return;
	}

	bFirstTime = false;

	//we will probably need to set the modbase_previous_map_played convar.
	if (!engine->IsLevelMainMenuBackground() && engine->IsConnected())
		modbase_previous_map_played.SetValue(CAutoGameSystem::MapName());

	engine->ClientCmd_Unrestricted("__real_disconnect");

	//check to see if we are using the Random load type and if so set CurrTempRandValue to a random number.
	if (g_PModBase_BikBackground_LoadType == ModBase_BikBackgroundLoadType::Random)
		CurrTempRandValue = random->RandomInt(0, g_PModBase_BikBackground_Holders.Count() - 1);

	//start the main menu video
	CBaseViewport* pViewPort = dynamic_cast<CBaseViewport*>(g_pClientMode->GetViewport());
	if (pViewPort)
		pViewPort->StartMainMenuVideo();

	//play the background song if there is one
	ConVarRef gConvar(g_PModBase_BikBackground_szConvarName);

	//stop all sounds
	enginesound->StopAllSounds(true);

	//get the background song info
	BikBackground_SongInfo_t* info = GetBackgroundSongInfo(gConvar.GetInt());

	//play the bg song
	if (info && info->songname && !bOverrideVideo)
		engine->ClientCmd(CFmtStr("playvol %s %f", info->songname, info->songvolume));
	else
		engine->ClientCmd(CFmtStr("playvol %s %f", bOverrideVideoSongBuf, flVolume));
}

//-----------------------------------------------------------------------------
// Purpose: the function to override the map_background command
//-----------------------------------------------------------------------------
void CC_MapBackground(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		ConMsg("  Command Usage: %s <background video (REQUIRES MEDIA EXTENTION)> <override song> <override volume> <override>\n    If the <override> parameter is 1 then it will override the background video to play to the inputted background video at the 2nd argument,\n    it will also play the song of the 3rd parameter with the volume of the 4th parameter. else it will just play the normal background video (the one that would play if you disconnected)\n", args.Arg(0));
		return;
	}

	if (atoi(args.Arg(4)) != 0)
	{
		bOverrideVideo = true;
		bOverrideVideoBuf = args.Arg(1);
		bOverrideVideoSongBuf = args.Arg(2);
		flVolume = atof(args.Arg(3));
	}

	__disconnect(CCommand{});

	bOverrideVideoBuf = bOverrideVideoSongBuf = nullptr;
	bOverrideVideo = false;
}

FnCommandCallback_t map_background_completion = nullptr;

//-----------------------------------------------------------------------------
// Purpose: Find the disconnect command, and rename it...
//-----------------------------------------------------------------------------
void SwapDisconnectCommand()
{
	//DevMsg("SwapDisconnectCommand\n");
	ConCommand* _realDisconnectCommand = dynamic_cast<ConCommand*>(g_pCVar->FindCommand("disconnect"));
	ConCommand* _DisconnectCommand = dynamic_cast<ConCommand*>(g_pCVar->FindCommand("__disconnect"));
	ConCommand* _Startupmenu = dynamic_cast<ConCommand*>(g_pCVar->FindCommand("startupmenu"));
	ConCommand* map_background = dynamic_cast<ConCommand*>(g_pCVar->FindCommand("map_background"));

	if (!_realDisconnectCommand)
		return;

	if (!_DisconnectCommand)
		return;

	_realDisconnectCommand->Shutdown();
	_realDisconnectCommand->Create("__real_disconnect", "");
	_realDisconnectCommand->Init();

	_DisconnectCommand->Shutdown();
	_DisconnectCommand->Create("disconnect", "Disconnect game from server.");
	_DisconnectCommand->Init();

	//change the startupmenu command
	if (_Startupmenu)
	{
		_Startupmenu->m_bHasCompletionCallback = false;
		_Startupmenu->m_fnCommandCallback = __disconnect;
	}

	if (map_background)
	{
		map_background->Shutdown();
		map_background->Create("map_background", "Plays the background video that would play when you disconnect/goto the main menu. execute this command with no args for command explination\n");
		map_background->Init();

		if (!map_background_completion)
			map_background_completion = map_background->m_fnCommandCallback;

		map_background->m_bHasCompletionCallback = false;
		map_background->m_fnCommandCallback = CC_MapBackground;
	}
}

//from cdll_client_int.cpp
void ModBase_DeleteBackground();
void ModBase_InitBackground();

//--------------------------------------------------------------------------------------------
// Purpose: resets the background bik thingy and reloads the file
//--------------------------------------------------------------------------------------------
CON_COMMAND(modbase_bik_background_reset, "Resets the bik background file (located in modbase/config/background_menu.txt). NOTE this doesnt revert the console commands changed (like startupmenu or map_background) so you're gonna have to restart!")
{
	CBaseViewport* pViewPort = dynamic_cast<CBaseViewport*>(g_pClientMode->GetViewport());
	if (!pViewPort)
	{
		BikBackgroundDebugMsg("Background BIK Video Error: Failed to find the viewport for modbase_bik_background_reset\n");
		return;
	}

	//delete the current BikBackground menu
	pViewPort->DeleteBIKMenu();

	//init the bik menu.
	pViewPort->InitializeBIKMenu(false);

	ModBase_DeleteBackground();
	ModBase_InitBackground();

	//swap SwapDisconnectCommand if needed
	if (g_PModBase_BikBackground_bUse)
	{
		SwapDisconnectCommand();
		__disconnect_command.Dispatch(CCommand{});
	}
	else if (map_background_completion)
	{
		CCommand cc;
		cc.Tokenize("map_background background1");

		//open the background map
		map_background_completion(cc);
	}
	else
		engine->ClientCmd("map_background background1");
}

//concommand for bik background
static ConCommand modbase_bik_background_play("modbase_bik_background_play", CC_MapBackground, "Plays a background video. input no parameters for help on how to use this command!");