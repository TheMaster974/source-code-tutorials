//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Moved CWeaponFrag class definition for flash grenade.
//
// $NoKeywords: $FixedByTheMaster974
//=============================================================================//

#ifndef WEAPON_FRAG_H
#define WEAPON_FRAG_H
#ifdef _WIN32
#pragma once
#endif

#include "basehlcombatweapon.h"

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponFrag : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponFrag, CBaseHLCombatWeapon);
public:
	DECLARE_SERVERCLASS();

public:
	CWeaponFrag();

	virtual void Precache(void);
	void	Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);
	void	PrimaryAttack(void);
	void	SecondaryAttack(void);
	void	DecrementAmmo(CBaseCombatCharacter* pOwner);
	void	ItemPostFrame(void);

	bool	Deploy(void);
	bool	Holster(CBaseCombatWeapon* pSwitchingTo = NULL);

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	bool	Reload(void);

	bool	ShouldDisplayHUDHint() { return true; }

public: // private
	virtual void ThrowGrenade(CBasePlayer* pPlayer); // Made virtual for flash grenade.
	virtual void RollGrenade(CBasePlayer* pPlayer); // Made virtual for flash grenade.
	virtual void LobGrenade(CBasePlayer* pPlayer); // Made virtual for flash grenade.
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void	CheckThrowPosition(CBasePlayer* pPlayer, const Vector& vecEye, Vector& vecSrc);

	bool	m_bRedraw;	//Draw the weapon again after throwing a grenade

	int		m_AttackPaused;
	bool	m_fDrawbackFinished;

	DECLARE_ACTTABLE();

	DECLARE_DATADESC();

// ----------
// Additions.
// ----------
	virtual void Drop(const Vector& vecVelocity);
	virtual void Spawn(void);
	virtual void Event_Killed(const CTakeDamageInfo& info);
};

#endif // WEAPON_FRAG_H