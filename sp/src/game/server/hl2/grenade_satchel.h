//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Satchel Charge
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $FixedByTheMaster974
//=============================================================================//

#ifndef	SATCHEL_H
#define	SATCHEL_H

#ifdef _WIN32
#pragma once
#endif

#include "basegrenade_shared.h"
#include "weapon_slam.h"

class CSoundPatch;
class CSprite;

class CSatchelCharge : public CBaseGrenade
{
public:
	DECLARE_CLASS( CSatchelCharge, CBaseGrenade );

	void			Spawn( void );
	void			Precache( void );
	void			BounceSound( void );
	void			SatchelTouch( CBaseEntity *pOther );
	void			SatchelThink( void );
	
	// Input handlers
	void			InputExplode( inputdata_t &inputdata );

// ----------
// Additions.
// ----------
	int OnTakeDamage(const CTakeDamageInfo& inputInfo);
	void Event_Killed(const CTakeDamageInfo& info);
	void DeathThink(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	float			m_flNextBounceSoundTime;
	bool			m_bInAir;
	Vector			m_vLastPosition;

public:
	CWeapon_SLAM*	m_pMyWeaponSLAM;	// Who shot me..
	bool			m_bIsAttached;
	void			Deactivate( void );

	CSatchelCharge();
	~CSatchelCharge();

	DECLARE_DATADESC();

private:
	void				CreateEffects( void );
	CHandle<CSprite>	m_hGlowSprite;
};

#endif	//SATCHEL_H
