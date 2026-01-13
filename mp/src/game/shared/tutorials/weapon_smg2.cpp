//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Adds a burst fire weapon.
//
//=============================================================================//

#include "cbase.h"

#ifdef CLIENT_DLL
#include "c_hl2mp_player.h"
#else
#include "hl2mp_player.h"
#endif

#include "weapon_hl2mpbase.h"
#include "weapon_hl2mpbase_machinegun.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponSMG2 C_WeaponSMG2
#endif

class CWeaponSMG2 : public CHL2MPSelectFireMachineGun
{
public:
	DECLARE_CLASS(CWeaponSMG2, CHL2MPSelectFireMachineGun);

	CWeaponSMG2();

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	const Vector& GetBulletSpread(void);

	void			Precache(void);
	void			AddViewKick(void);

	int				GetMinBurst() { return 2; }
	int				GetMaxBurst() { return 5; }

	float			GetFireRate(void) { return 0.1f; }

	const WeaponProficiencyInfo_t* GetProficiencyValues();

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponSMG2, DT_WeaponSMG2)

BEGIN_NETWORK_TABLE(CWeaponSMG2, DT_WeaponSMG2)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponSMG2)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_smg2, CWeaponSMG2);
PRECACHE_WEAPON_REGISTER(weapon_smg2);

#ifndef CLIENT_DLL
acttable_t	CWeaponSMG2::m_acttable[] =
{
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_SMG1,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_SMG1,						false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_SMG1,				false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_SMG1,				false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_SMG1,			false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_SMG1,					false },
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_SMG1,					false },
};

IMPLEMENT_ACTTABLE(CWeaponSMG2);
#endif

//=========================================================
CWeaponSMG2::CWeaponSMG2()
{
	m_fMaxRange1 = 2000;
	m_fMinRange1 = 32;

	m_iFireMode = FIREMODE_3RNDBURST;
}

void CWeaponSMG2::Precache(void)
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector& CWeaponSMG2::GetBulletSpread(void)
{
	static const Vector cone = VECTOR_CONE_10DEGREES;
	return cone;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG2::AddViewKick(void)
{
#define	EASY_DAMPEN			0.5f
#define	MAX_VERTICAL_KICK	2.0f	//Degrees
#define	SLIDE_LIMIT			1.0f	//Seconds

	//Get the view kick
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t* CWeaponSMG2::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0 / 3.0, 0.75	},
		{ 5.0 / 3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
