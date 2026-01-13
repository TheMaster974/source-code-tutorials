//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Adds Hit Markers. Thanks to Ferrety for the code and to WadDelZ for the improvements!
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_hitmarker.h"
#include "iclientmode.h"
#include "c_baseplayer.h"
#include "fmtstr.h"

// VGUI panel includes
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT(CHudHitmarker);
DECLARE_HUD_MESSAGE(CHudHitmarker, ShowHitmarker);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHitmarker::CHudHitmarker(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudHitmarker")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	// Hitmarker will not show when the player is dead
	SetHiddenBits(HIDEHUD_PLAYERDEAD);

	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHitmarker::Init()
{
	HOOK_HUD_MESSAGE(CHudHitmarker, ShowHitmarker);

	SetAlpha(0);
	m_bHitmarkerShow = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHitmarker::Reset()
{
	SetAlpha(0);
	m_bHitmarkerShow = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHitmarker::ApplySchemeSettings(vgui::IScheme* scheme)
{
// ---------------------------------------------------------------------------------------------
// Creates the font, replace Tahoma with whatever you want. 120 represents the size of the text.
// ---------------------------------------------------------------------------------------------
	m_hFont = vgui::surface()->CreateFont();
	vgui::surface()->SetFontGlyphSet(m_hFont, "Tahoma", 120, 400, 0, 0, vgui::ISurface::FONTFLAG_OUTLINE);

	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudHitmarker::ShouldDraw(void)
{
	return (m_bHitmarkerShow && CHudElement::ShouldDraw());
}

// --------------------------------------------------------
// Convert a const char* to a const wchar_t* appropriately.
// --------------------------------------------------------
const wchar_t* ConvertToWide(const char* input)
{
	static wchar_t wide[32];
	mbstowcs(wide, input, 32);
	return wide;
}

int ScreenTransform(const Vector& point, Vector& screen); // This is defined in view_scene.cpp.

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHitmarker::Paint(void)
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	if (m_bHitmarkerShow)
	{
		Vector screenPos; // Get position on screen.
		int result = ScreenTransform(m_Vec, screenPos);

		if (result == 1) // 1 means behind the camera.
			return;

		// Get pos.
		int x = (int)(0.5f * (1.0f + screenPos.x) * ScreenWidth());
		int y = (int)(0.5f * (1.0f - screenPos.y) * ScreenHeight());
		float scale = 2.0f; // Use this to adjust the size of the hitmarkers.

		vgui::surface()->DrawSetColor(m_HitmarkerColor);
		vgui::surface()->DrawLine(x - 6, y - 5, x - 11*scale, y - 10*scale);
		vgui::surface()->DrawLine(x + 5, y - 5, x + 10*scale, y - 10*scale);
		vgui::surface()->DrawLine(x - 6, y + 5, x - 11*scale, y + 10*scale);
		vgui::surface()->DrawLine(x + 5, y + 5, x + 10*scale, y + 10*scale);

		// Set print stuff.
		vgui::surface()->DrawSetTextFont(m_hFont);
		vgui::surface()->DrawSetTextColor(m_HitmarkerColor);
		vgui::surface()->DrawSetTextPos(x, y);

		// Print text.
		const wchar_t* text = ConvertToWide(CFmtStr("%d", m_iDamage));
		vgui::surface()->DrawPrintText(text, Q_wcslen(text));
	}
}

ConVar cl_show_hitmarker("cl_show_hitmarker", "1", FCVAR_CLIENTDLL, "Draws hitmarkers.");

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHitmarker::MsgFunc_ShowHitmarker(bf_read& msg)
{
	// Get the stuff needed.
	m_bHitmarkerShow = msg.ReadByte();
	bool alive = msg.ReadByte();
	m_iDamage = msg.ReadByte();
	msg.ReadBitVec3Coord(m_Vec);

	// Don't draw hitmarkers if we have disabled them!
	if (!cl_show_hitmarker.GetBool())
		return;

	// Check to see if the enemy is alive or not.
	if (alive)
		m_HitmarkerColor.SetColor(250, 235, 235, 255);
	else
		m_HitmarkerColor.SetColor(250, 10, 10, 255);

	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HitMarkerShow");
}