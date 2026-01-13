//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Adds burst fire functionality.
//
//=============================================================================//

#include "cbase.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
#endif

#include "weapon_hl2mpbase_machinegun.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( HL2MPMachineGun, DT_HL2MPMachineGun )

// ----------------------
// Do we need to do this?
// ----------------------
BEGIN_NETWORK_TABLE( CHL2MPMachineGun, DT_HL2MPMachineGun )
#ifdef CLIENT_DLL
RecvPropVector(RECVINFO(vecSpread)),
RecvPropVector(RECVINFO(spreadToUse)),
#else
SendPropVector(SENDINFO(vecSpread), -1, SPROP_COORD_MP),
SendPropVector(SENDINFO(spreadToUse), -1, SPROP_COORD_MP),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CHL2MPMachineGun )
#ifdef CLIENT_DLL
	DEFINE_PRED_FIELD( m_nShotsFired, FIELD_INTEGER, 0 ),
#endif
END_PREDICTION_DATA()

//=========================================================
//	>> CHLSelectFireMachineGun
//=========================================================
BEGIN_DATADESC( CHL2MPMachineGun )

	DEFINE_FIELD( m_nShotsFired,	FIELD_INTEGER ),
	DEFINE_FIELD( m_flNextSoundTime, FIELD_TIME ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHL2MPMachineGun::CHL2MPMachineGun( void )
{
}

const Vector &CHL2MPMachineGun::GetBulletSpread( void )
{
	static Vector cone = VECTOR_CONE_3DEGREES;
	return cone;
}

//-----------------------------------------------------------------------------
// Purpose: Slightly modified to make burst fire weapons work better...I think. Taken from basecombatweapon_shared.cpp.
//
//
//-----------------------------------------------------------------------------
void CHL2MPMachineGun::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
		return;
	
	// Abort here to handle burst and auto fire modes
	if ( (UsesClipsForAmmo1() && m_iClip1 == 0) || ( !UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType) ) )
		return;

	FireBulletsInfo_t info;
	m_nShotsFired++;

	pPlayer->DoMuzzleFlash();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	info.m_iShots = 0;
	float fireRate = GetFireRate();

	while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		// MUST call sound before removing a round from the clip of a CHLMachineGun
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		info.m_iShots++;
		if (!fireRate)
			break;
	}

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if ( UsesClipsForAmmo1() )
	{
		info.m_iShots = MIN(1, m_iClip1);
		m_iClip1 -= 1;
	}

	CHL2MP_Player *pHL2MPPlayer = ToHL2MPPlayer( pPlayer );
	
// ------------------------------------------------------------------------------------
// Modified the bullet spread to make burst fire weapons have a little more randomness.
// ------------------------------------------------------------------------------------
	vecSpread = pHL2MPPlayer->GetAttackSpread(this);
	float randFloat = vecSpread.GetX(); // x = y = z
	spreadToUse = SharedRandomVector("spreadEagle", 0, randFloat, RandomInt(0, GetMaxClip1()));

	// Fire the bullets
//	info.m_iShots = 1; // This has been moved up a little bit.
	info.m_vecSrc = pHL2MPPlayer->Weapon_ShootPosition( );
	info.m_vecDirShooting = pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	info.m_vecSpread = spreadToUse; // info.m_vecSpread = pHL2MPPlayer->GetAttackSpread(this);
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;
	FireBullets( info );

	//Factor in the view kick
	AddViewKick();
	
	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	SendWeaponAnim( GetPrimaryAttackActivity() );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//-----------------------------------------------------------------------------
void CHL2MPMachineGun::FireBullets( const FireBulletsInfo_t &info )
{
	if(CBasePlayer *pPlayer = ToBasePlayer ( GetOwner() ) )
	{
		pPlayer->FireBullets(info);
	}
}

// ---------
// Addition.
// ---------
bool CHL2MPMachineGun::Reload(void)
{
	bool isReloadAllowed = BaseClass::Reload();
	if (isReloadAllowed)
	{
		WeaponSound(RELOAD);
		return BaseClass::Reload();
	}
	else
		return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MPMachineGun::DoMachineGunKick( CBasePlayer *pPlayer, float dampEasy, float maxVerticleKickAngle, float fireDurationTime, float slideLimitTime )
{
	#define	KICK_MIN_X			0.2f	//Degrees
	#define	KICK_MIN_Y			0.2f	//Degrees
	#define	KICK_MIN_Z			0.1f	//Degrees

	QAngle vecScratch;
	int iSeed = CBaseEntity::GetPredictionRandomSeed() & 255;
	
	//Find how far into our accuracy degradation we are
	float duration	= ( fireDurationTime > slideLimitTime ) ? slideLimitTime : fireDurationTime;
	float kickPerc = duration / slideLimitTime;

	// do this to get a hard discontinuity, clear out anything under 10 degrees punch
	pPlayer->ViewPunchReset( 10 );

	//Apply this to the view angles as well
	vecScratch.x = -( KICK_MIN_X + ( maxVerticleKickAngle * kickPerc ) );
	vecScratch.y = -( KICK_MIN_Y + ( maxVerticleKickAngle * kickPerc ) ) / 3;
	vecScratch.z = KICK_MIN_Z + ( maxVerticleKickAngle * kickPerc ) / 8;

	RandomSeed( iSeed );

	//Wibble left and right
	if ( RandomInt( -1, 1 ) >= 0 )
		vecScratch.y *= -1;

	iSeed++;

	//Wobble up and down
	if ( RandomInt( -1, 1 ) >= 0 )
		vecScratch.z *= -1;

	//Clip this to our desired min/max
	UTIL_ClipPunchAngleOffset( vecScratch, pPlayer->m_Local.m_vecPunchAngle, QAngle( 24.0f, 3.0f, 1.0f ) );

	//Add it to the view punch
	// NOTE: 0.5 is just tuned to match the old effect before the punch became simulated
	pPlayer->ViewPunch( vecScratch * 0.5 );
}

//-----------------------------------------------------------------------------
// Purpose: Reset our shots fired
//-----------------------------------------------------------------------------
bool CHL2MPMachineGun::Deploy( void )
{
	m_nShotsFired = 0;

	return BaseClass::Deploy();
}



//-----------------------------------------------------------------------------
// Purpose: Make enough sound events to fill the estimated think interval
// returns: number of shots needed
//-----------------------------------------------------------------------------
int CHL2MPMachineGun::WeaponSoundRealtime( WeaponSound_t shoot_type )
{
	int numBullets = 0;

	// ran out of time, clamp to current
	if (m_flNextSoundTime < gpGlobals->curtime)
	{
		m_flNextSoundTime = gpGlobals->curtime;
	}

	// make enough sound events to fill up the next estimated think interval
	float dt = clamp( m_flAnimTime - m_flPrevAnimTime, 0, 0.2 );
	if (m_flNextSoundTime < gpGlobals->curtime + dt)
	{
		WeaponSound( SINGLE_NPC, m_flNextSoundTime );
		m_flNextSoundTime += GetFireRate();
		numBullets++;
	}
	if (m_flNextSoundTime < gpGlobals->curtime + dt)
	{
		WeaponSound( SINGLE_NPC, m_flNextSoundTime );
		m_flNextSoundTime += GetFireRate();
		numBullets++;
	}

	return numBullets;
}




//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MPMachineGun::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	// Debounce the recoiling counter
	if ( ( pOwner->m_nButtons & IN_ATTACK ) == false )
	{
		m_nShotsFired = 0;
	}

	BaseClass::ItemPostFrame();
}

// -------------------------------------------------------------------------------
// Implementation of the burst fire weapon class, is some of this stuff necessary?
// -------------------------------------------------------------------------------
IMPLEMENT_NETWORKCLASS_ALIASED(HL2MPSelectFireMachineGun, DT_HL2MPSelectFireMachineGun)

BEGIN_NETWORK_TABLE(CHL2MPSelectFireMachineGun, DT_HL2MPSelectFireMachineGun)
#ifdef CLIENT_DLL
RecvPropInt(RECVINFO(m_iFireMode)),
RecvPropInt(RECVINFO(m_iBurstSize)),
#else
SendPropInt(SENDINFO(m_iFireMode)),
SendPropInt(SENDINFO(m_iBurstSize)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CHL2MPSelectFireMachineGun)
#ifdef CLIENT_DLL
DEFINE_PRED_FIELD(m_iFireMode, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_iBurstSize, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
#endif
END_PREDICTION_DATA()

BEGIN_DATADESC(CHL2MPSelectFireMachineGun)
DEFINE_FIELD(m_iBurstSize, FIELD_INTEGER),
DEFINE_FIELD(m_iFireMode, FIELD_INTEGER),
#ifdef GAME_DLL
DEFINE_THINKFUNC(BurstThink), // Cannot use this on the Client.
#endif
END_DATADESC()


float CHL2MPSelectFireMachineGun::GetFireRate(void)
{
	switch (m_iFireMode)
	{
	case FIREMODE_FULLAUTO:
		// the time between rounds fired on full auto
		return 0.1f;	// 600 rounds per minute = 0.1 seconds per bullet
		break;

	case FIREMODE_3RNDBURST:
		// the time between rounds fired within a single burst
		return 0.1f;	// 600 rounds per minute = 0.1 seconds per bullet
		break;

	default:
		return 0.1f;
		break;
	}
}

bool CHL2MPSelectFireMachineGun::Deploy(void)
{
	// Forget about any bursts this weapon was firing when holstered
	m_iBurstSize = 0;
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHL2MPSelectFireMachineGun::PrimaryAttack(void)
{
	if (m_bFireOnEmpty)
	{
		return;
	}
	switch (m_iFireMode)
	{
	case FIREMODE_FULLAUTO:
		BaseClass::PrimaryAttack();
		// Msg("%.3f\n", m_flNextPrimaryAttack.Get() );
		SetWeaponIdleTime(gpGlobals->curtime + 3.0f);
		break;

	case FIREMODE_3RNDBURST:
		m_iBurstSize = GetBurstSize();

		// Call the think function directly so that the first round gets fired immediately.
		BurstThink();
#ifndef CLIENT_DLL
		SetThink(&CHL2MPSelectFireMachineGun::BurstThink); // This doesn't get called correctly on the Client.
#endif
		m_flNextPrimaryAttack = gpGlobals->curtime + GetBurstCycleRate();
		m_flNextSecondaryAttack = gpGlobals->curtime + GetBurstCycleRate();

		// Pick up the rest of the burst through the think function.
		SetNextThink(gpGlobals->curtime);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHL2MPSelectFireMachineGun::SecondaryAttack(void)
{
	// change fire modes.

	switch (m_iFireMode)
	{
	case FIREMODE_FULLAUTO:
		//Msg( "Burst\n" );
		m_iFireMode = FIREMODE_3RNDBURST;
		WeaponSound(SPECIAL2);
		break;

	case FIREMODE_3RNDBURST:
		//Msg( "Auto\n" );
		m_iFireMode = FIREMODE_FULLAUTO;
		WeaponSound(SPECIAL1);
		break;
	}

	SendWeaponAnim(GetSecondaryAttackActivity());

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHL2MPSelectFireMachineGun::BurstThink(void)
{
	BaseClass::PrimaryAttack();

	m_iBurstSize--;

	if (m_iBurstSize == 0)
	{
		// The burst is over!
		SetThink(NULL);

		// idle immediately to stop the firing animation
		SetWeaponIdleTime(gpGlobals->curtime);
		return;
	}

	SetNextThink(gpGlobals->curtime + GetFireRate());
}

// ---------
// Addition.
// ---------
bool CHL2MPSelectFireMachineGun::Reload(void)
{
	if (m_iBurstSize > 0)
		return false;
	else
		return BaseClass::Reload();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CHL2MPSelectFireMachineGun::WeaponSound(WeaponSound_t shoot_type, float soundtime /*= 0.0f*/)
{
	if (shoot_type == SINGLE)
	{
		switch (m_iFireMode)
		{
		case FIREMODE_FULLAUTO:
			BaseClass::WeaponSound(SINGLE, soundtime);
			break;

		case FIREMODE_3RNDBURST:
			if (m_iBurstSize == GetBurstSize() && m_iClip1 >= m_iBurstSize)
			{
				// First round of a burst, and enough bullets remaining in the clip to fire the whole burst
				BaseClass::WeaponSound(BURST, soundtime);
			}
			else if (m_iClip1 == 1 ) // Only plays one sound! Formerly: ( m_iClip1 < m_iBurstSize )
			{
				// Not enough rounds remaining in the magazine to fire a burst, so play the gunshot
				// sounds individually as each round is fired.
				BaseClass::WeaponSound(SINGLE, soundtime);
			}
			else if (m_iClip1 == 2)
			{
				BaseClass::WeaponSound(SPECIAL3, soundtime);
			}

			break;
		}
		return;
	}

	BaseClass::WeaponSound(shoot_type, soundtime);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHL2MPSelectFireMachineGun::CHL2MPSelectFireMachineGun(void)
{
// ------------------------
// Is this stuff necessary?
// ------------------------
	/*
	m_fMinRange1 = 65;
	m_fMinRange2 = 65;
	m_fMaxRange1 = 1024;
	m_fMaxRange2 = 1024;
	*/
	m_iFireMode = FIREMODE_FULLAUTO;
}
