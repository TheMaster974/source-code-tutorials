//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Adds Hit Markers. Thanks to Ferrety for the code and to WadDelZ for the improvements!
//
//=============================================================================//

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Panel.h>

class CHudHitmarker : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudHitmarker, vgui::Panel);

public:
	CHudHitmarker(const char* pElementName);

	void Init();
	void Reset();
	bool ShouldDraw();

	void MsgFunc_ShowHitmarker(bf_read& msg);

protected:
	virtual void ApplySchemeSettings(vgui::IScheme* scheme);
	virtual void Paint(void);

private:
	bool m_bHitmarkerShow;
	int m_iDamage;
	Vector m_Vec;

	vgui::HFont m_hFont;

	CPanelAnimationVar(Color, m_HitmarkerColor, "HitMarkerColor", "250 235 235 255");
};
