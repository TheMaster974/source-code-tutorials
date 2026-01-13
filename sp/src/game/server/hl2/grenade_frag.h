//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Moved CGrenadeFrag class definition for flash grenade.
//
// $NoKeywords: $FixedByTheMaster974
//=============================================================================//

#ifndef GRENADE_FRAG_H
#define GRENADE_FRAG_H
#pragma once

// ----------
// Additions.
// ----------
#include "basegrenade_shared.h"
#include "SpriteTrail.h"

class CBaseGrenade;
struct edict_t;

CBaseGrenade *Fraggrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned );
bool	Fraggrenade_WasPunted( const CBaseEntity *pEntity );
bool	Fraggrenade_WasCreatedByCombine( const CBaseEntity *pEntity );

// ---------------------------
// Moved from grenade_frag.cpp
// ---------------------------
class CGrenadeFrag : public CBaseGrenade
{
public: // Moved slightly.
	DECLARE_CLASS(CGrenadeFrag, CBaseGrenade);

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif

	~CGrenadeFrag(void);

	void	Spawn(void);
	void	OnRestore(void);
	void	Precache(void);
	bool	CreateVPhysics(void);
	void	CreateEffects(void);
	void	SetTimer(float detonateDelay, float warnDelay);
	void	SetVelocity(const Vector& velocity, const AngularImpulse& angVelocity);
	int		OnTakeDamage(const CTakeDamageInfo& inputInfo);
	void	BlipSound() { EmitSound("Grenade.Blip"); }
	void	DelayThink();
	void	VPhysicsUpdate(IPhysicsObject* pPhysics);
	void	OnPhysGunPickup(CBasePlayer* pPhysGunUser, PhysGunPickup_t reason);
	void	SetCombineSpawned(bool combineSpawned) { m_combineSpawned = combineSpawned; }
	bool	IsCombineSpawned(void) const { return m_combineSpawned; }
	void	SetPunted(bool punt) { m_punted = punt; }
	bool	WasPunted(void) const { return m_punted; }

	// this function only used in episodic.
#if defined(HL2_EPISODIC) && 0 // FIXME: HandleInteraction() is no longer called now that base grenade derives from CBaseAnimating
	bool	HandleInteraction(int interactionType, void* data, CBaseCombatCharacter* sourceEnt);
#endif 

	void	InputSetTimer(inputdata_t& inputdata);

protected:
	CHandle<CSprite>		m_pMainGlow;
	CHandle<CSpriteTrail>	m_pGlowTrail;

	float	m_flNextBlipTime;
	bool	m_inSolid;
	bool	m_combineSpawned;
	bool	m_punted;
};

// ----------------------------------------------------------------
// Needed so the flashbang/concussion grenades can spawn correctly.
// ----------------------------------------------------------------
CBaseGrenade* Flashgrenade_Create(const Vector& position, const QAngle& angles, const Vector& velocity, const AngularImpulse& angVelocity, CBaseEntity* pOwner, float timer, bool combineSpawned);

#endif // GRENADE_FRAG_H
