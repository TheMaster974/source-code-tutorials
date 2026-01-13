//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Adds burst fire functionality and fixes viewmodel bobbing.
//
//=============================================================================//

#include "weapon_hl2mpbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifndef BASEHLCOMBATWEAPON_H
#define BASEHLCOMBATWEAPON_H
#ifdef _WIN32
#pragma once
#endif

#if defined( CLIENT_DLL )
	#define CHL2MPMachineGun C_HL2MPMachineGun
#endif

//=========================================================
// Machine gun base class, fixes viewmodel bobbing.
//=========================================================
class CHL2MPMachineGun : public CBaseHL2MPCombatWeapon // CWeaponHL2MPBase
{
public:
	DECLARE_CLASS( CHL2MPMachineGun, CBaseHL2MPCombatWeapon );
	DECLARE_DATADESC();

	CHL2MPMachineGun();
	
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	void	PrimaryAttack( void );

	// Default calls through to m_hOwner, but plasma weapons can override and shoot projectiles here.
	virtual void	ItemPostFrame( void );
	virtual void	FireBullets( const FireBulletsInfo_t &info );
	virtual bool	Deploy( void );
	
	// Addition.
	virtual bool	Reload( void );

	virtual const Vector &GetBulletSpread( void );

	int				WeaponSoundRealtime( WeaponSound_t shoot_type );

	// utility function
	static void DoMachineGunKick( CBasePlayer *pPlayer, float dampEasy, float maxVerticleKickAngle, float fireDurationTime, float slideLimitTime );

private:
	
	CHL2MPMachineGun( const CHL2MPMachineGun & );

protected:

	int	m_nShotsFired;	// Number of consecutive shots fired

	float	m_flNextSoundTime;	// real-time clock of when to make next sound

// -------------------------------------------
// Additions. Should these be CNetworkVectors?
// -------------------------------------------
	CNetworkVector(vecSpread);
	CNetworkVector(spreadToUse);
};

//=========================================================
// Machine guns capable of switching between full auto and 
// burst fire modes.
//=========================================================
// Mode settings for select fire weapons
enum
{
	FIREMODE_FULLAUTO = 1,
	FIREMODE_SEMI,
	FIREMODE_3RNDBURST,
};

#ifdef CLIENT_DLL
#define CHL2MPSelectFireMachineGun C_HL2MPSelectFireMachineGun
#endif

// ---------------------------------------------------------------------------------------------------------------
// Burst fire weapon class. TODO: Investigate Client/Server networking? The burst fire doesn't look right to me...
// ---------------------------------------------------------------------------------------------------------------
class CHL2MPSelectFireMachineGun : public CHL2MPMachineGun
{
public:
	DECLARE_CLASS(CHL2MPSelectFireMachineGun, CHL2MPMachineGun);
	DECLARE_DATADESC();

	CHL2MPSelectFireMachineGun();

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual float	GetBurstCycleRate(void) { return 0.5f; }
	virtual float	GetFireRate(void);

	virtual bool	Deploy(void);
	virtual void	WeaponSound(WeaponSound_t shoot_type, float soundtime = 0.0f);

	virtual int		GetBurstSize(void) { return 3; };

	virtual void	PrimaryAttack(void);
	virtual void	SecondaryAttack(void);
	
	// Addition.
	virtual bool	Reload(void);

	void			BurstThink(void);

protected:
// -----------------------------
// Should these be CNetworkVars?
// -----------------------------
	CNetworkVar(int, m_iBurstSize);
	CNetworkVar(int, m_iFireMode);
};

#endif // BASEHLCOMBATWEAPON_H
