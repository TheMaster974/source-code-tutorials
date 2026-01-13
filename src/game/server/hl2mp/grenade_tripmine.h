//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Fixed floating Tripmine bug.
//
// $NoKeywords: $FixedByTheMaster974
//=============================================================================//

#ifndef GRENADE_TRIPMINE_H
#define GRENADE_TRIPMINE_H
#ifdef _WIN32
#pragma once
#endif

#include "basegrenade_shared.h"

class CBeam;


class CTripmineGrenade : public CBaseGrenade
{
public:
	DECLARE_CLASS( CTripmineGrenade, CBaseGrenade );

	CTripmineGrenade();
	void Spawn( void );
	void Precache( void );

#if 0 // FIXME: OnTakeDamage_Alive() is no longer called now that base grenade derives from CBaseAnimating
	int OnTakeDamage_Alive( const CTakeDamageInfo &info );
#endif	
	void WarningThink( void );
	void PowerupThink( void );
	void BeamBreakThink( void );
	void DelayDeathThink( void );
	void Event_Killed( const CTakeDamageInfo &info );

	void MakeBeam( void );
	void KillBeam( void );

// ----------
// Additions.
// ----------
	void AttachToEntity(CBaseEntity* ent);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void OnPhysGunPickup(CBasePlayer* pPhysGunUser, PhysGunPickup_t reason);

public:
	EHANDLE		m_hOwner;

private:
	float		m_flPowerUp;
	Vector		m_vecDir;
	Vector		m_vecEnd;
	float		m_flBeamLength;

	CBeam		*m_pBeam;
	Vector		m_posOwner;
	Vector		m_angleOwner;

// ------------------------------------
// Additions to fix floating Tripmines.
// ------------------------------------
	CBaseEntity* m_pAttachedObject;
	Vector m_vecOldPosAttachedObject;
	QAngle m_vecOldAngAttachedObject;
	int m_pAttachedSequence;

	DECLARE_DATADESC();
};

#endif // GRENADE_TRIPMINE_H
