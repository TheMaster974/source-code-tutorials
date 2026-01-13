//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Restores beam-drawing flamethrower-like weapon. Make sure to add
//			"reload" "Weapon_Immolator.Reload" to weapon_immolator.txt, and
//			add the weapon sounds to game_sounds_weapons.txt.
//
// $NoKeywords: $FixedByTheMaster974
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "soundent.h"
#include "te_particlesystem.h"
#include "ndebugoverlay.h"
#include "in_buttons.h"
#include "ai_basenpc.h"
#include "ai_memory.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MAX_BURN_RADIUS		256
#define RADIUS_GROW_RATE	50.0	// units/sec 

#define IMMOLATOR_TARGET_INVALID Vector( FLT_MAX, FLT_MAX, FLT_MAX )

class CWeaponImmolator : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponImmolator, CBaseHLCombatWeapon );

	DECLARE_SERVERCLASS();

	CWeaponImmolator( void );
	
	void Precache( void );
//	void PrimaryAttack( void ); // Commented out.
	void ItemPostFrame( void );

	int CapabilitiesGet( void ) {	return bits_CAP_WEAPON_RANGE_ATTACK1;	}

	void ImmolationDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore );
	virtual bool WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );	
	virtual int	WeaponRangeAttack1Condition( float flDot, float flDist );

	void Update();
	void UpdateThink();

	void StartImmolating();
	void StopImmolating();
//	bool IsImmolating() { return m_flBurnRadius != 0.0; }
	bool IsImmolating() { return isActive; }

// ----------------------------------------------------------------------
// Additions, these are to fix issues when switching/dropping the weapon.
// ----------------------------------------------------------------------
	bool	Holster(CBaseCombatWeapon* pSwitchingTo);
	void	Drop(const Vector& vecVelocity);
	void	Equip(CBaseCombatCharacter* pOwner);

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();

	int	m_beamIndex;

	float m_flBurnRadius;
	float m_flTimeLastUpdatedRadius;

	Vector  m_vecImmolatorTarget;
	bool isActive; // Addition.
};

IMPLEMENT_SERVERCLASS_ST(CWeaponImmolator, DT_WeaponImmolator)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( info_target_immolator, CPointEntity );
LINK_ENTITY_TO_CLASS( weapon_immolator, CWeaponImmolator );
PRECACHE_WEAPON_REGISTER( weapon_immolator );

BEGIN_DATADESC( CWeaponImmolator )

	DEFINE_FIELD( m_beamIndex, FIELD_INTEGER ),
	DEFINE_FIELD( m_flBurnRadius, FIELD_FLOAT ),
	DEFINE_FIELD( m_flTimeLastUpdatedRadius, FIELD_TIME ),
	DEFINE_FIELD( m_vecImmolatorTarget, FIELD_VECTOR ),
	DEFINE_FIELD( isActive, FIELD_BOOLEAN ), // Addition.

	DEFINE_ENTITYFUNC( UpdateThink ),
END_DATADESC()


//-----------------------------------------------------------------------------
// Maps base activities to weapons-specific ones so our characters do the right things.
//-----------------------------------------------------------------------------
acttable_t CWeaponImmolator::m_acttable[] = 
{
	{	ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SNIPER_RIFLE, true }
};

IMPLEMENT_ACTTABLE( CWeaponImmolator );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponImmolator::CWeaponImmolator( void )
{
	m_fMaxRange1 = 4096;
//	StopImmolating();
	isActive = false;
}

void CWeaponImmolator::StartImmolating()
{
	isActive = true; // Immolator is active!
	SendWeaponAnim(ACT_VM_PRIMARYATTACK); // Play the primary attack animation.
	// Start the radius really tiny because we use radius == 0.0 to 
	// determine whether the immolator is operating or not.
	m_flBurnRadius = 0.1;
	m_flTimeLastUpdatedRadius = gpGlobals->curtime;
	SetThink( &CWeaponImmolator::UpdateThink ); // Proper pointer.
	SetNextThink( gpGlobals->curtime );

	CSoundEnt::InsertSound( SOUND_DANGER, m_vecImmolatorTarget, 256, 5.0, GetOwner() );
}

void CWeaponImmolator::StopImmolating()
{
	isActive = false; // Immolator is no longer active.
	StopWeaponSound(SINGLE); // Stop weapon sound.
	SendWeaponAnim(ACT_VM_IDLE); // Play idle animation.

	m_flBurnRadius = 0.0;
	SetThink( NULL );
	m_vecImmolatorTarget= IMMOLATOR_TARGET_INVALID;
	m_flNextPrimaryAttack = gpGlobals->curtime + 5.0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponImmolator::Precache( void )
{
	m_beamIndex = PrecacheModel( "sprites/bluelaser1.vmt" );

	BaseClass::Precache();
}
/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponImmolator::PrimaryAttack( void )
{
	WeaponSound( SINGLE );

	if( !IsImmolating() )
	{
		StartImmolating();
	} 
}
*/
//-----------------------------------------------------------------------------
// This weapon is said to have Line of Sight when it CAN'T see the target, but
// can see a place near the target than can.
//-----------------------------------------------------------------------------
bool CWeaponImmolator::WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
	CAI_BaseNPC* npcOwner = GetOwner()->MyNPCPointer();

	if( !npcOwner )
	{
		return false;
	}

	if( IsImmolating() )
	{
		// Don't update while Immolating. This is a committed attack.
		return false;
	}

	// Assume we won't find a target.
	m_vecImmolatorTarget = targetPos;
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Weapon firing conditions
//-----------------------------------------------------------------------------
int CWeaponImmolator::WeaponRangeAttack1Condition( float flDot, float flDist )
{
	if( m_flNextPrimaryAttack > gpGlobals->curtime )
	{
		// Too soon to attack!
		return COND_NONE;
	}

	if( IsImmolating() )
	{
		// Once is enough!
		return COND_NONE;
	}

	if(	m_vecImmolatorTarget == IMMOLATOR_TARGET_INVALID )
	{
		// No target!
		return COND_NONE;
	}

	if ( flDist > m_fMaxRange1 )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	else if ( flDot < 0.5f )	// UNDONE: Why check this here? Isn't the AI checking this already?
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_RANGE_ATTACK1;
}

void CWeaponImmolator::UpdateThink( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if( pOwner && !pOwner->IsAlive() )
	{
		StopImmolating();
		return;
	}

	Update();
	SetNextThink( gpGlobals->curtime + 0.05 );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponImmolator::Update()
{
	float flDuration = gpGlobals->curtime - m_flTimeLastUpdatedRadius;
	if( flDuration != 0.0 )
	{
		m_flBurnRadius += RADIUS_GROW_RATE * flDuration;
	}

	// Clamp
	m_flBurnRadius = MIN( m_flBurnRadius, MAX_BURN_RADIUS );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	Vector vecSrc, vecAiming, forward, right, up; // Added forward, right and up.
//	QAngle angle; // Unused!

// ---------------------
// Switched this around.
// ---------------------
	if( !pOwner )
	{
		CBaseCombatCharacter* pOwner = GetOwner();
		vecSrc = pOwner->Weapon_ShootPosition();
		vecAiming = m_vecImmolatorTarget - vecSrc;
		VectorNormalize(vecAiming);
	}
	else
	{
		vecSrc = pOwner->Weapon_ShootPosition();
		pOwner->GetVectors(&forward, &right, &up);
		vecSrc += (forward * 8.0f) + (right * 3.0f) + (up * -2.0f); // Adjust the start position slightly.
		vecAiming = pOwner->GetAutoaimVector(AUTOAIM_2DEGREES); // 2 degrees is arbitrary.
	}

	trace_t	tr;
	UTIL_TraceLine( vecSrc, vecSrc + vecAiming * MAX_TRACE_LENGTH, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr );

	int brightness;
	brightness = 255 * (m_flBurnRadius/MAX_BURN_RADIUS);
	UTIL_Beam(  vecSrc,
				tr.endpos,
				m_beamIndex,
				0,		//halo index
				0,		//frame start
				2.0f,	//framerate
				0.1f,	//life
				20,		// width
				1,		// endwidth
				0,		// fadelength,
				1,		// noise

				0,		// red
				255,	// green
				0,		// blue,

				brightness, // bright
				100  // speed
				);


	if( tr.DidHitWorld() )
	{
		int beams;

		for( beams = 0 ; beams < 5 ; beams++ )
		{
			Vector vecDest;

			// Random unit vector
			vecDest.x = random->RandomFloat( -1, 1 );
			vecDest.y = random->RandomFloat( -1, 1 );
			vecDest.z = random->RandomFloat( 0, 1 );

			// Push out to radius dist.
			vecDest = tr.endpos + vecDest * m_flBurnRadius;

			UTIL_Beam(  tr.endpos,
						vecDest,
						m_beamIndex,
						0,		//halo index
						0,		//frame start
						2.0f,	//framerate
						0.15f,	//life
						20,		// width
						1.75,	// endwidth
						0.75,	// fadelength,
						15,		// noise

						0,		// red
						255,	// green
						0,		// blue,

						128, // bright
						100  // speed
						);
		}

		// Immolator starts to hurt a few seconds after the effect is seen
		if( m_flBurnRadius > 64.0 )
		{
			ImmolationDamage( CTakeDamageInfo( this, this, 1, DMG_BURN ), tr.endpos, m_flBurnRadius, CLASS_NONE );
		}
	}
	else
	{
// ---------------------------------
// Do damage to NPCs and stuff here!
// ---------------------------------
		// The attack beam struck some kind of entity directly. Limit the attacking range with m_fMaxRange1.
		if (m_flBurnRadius > 16.0 && (tr.endpos - tr.startpos).Length() <= m_fMaxRange1)
		{
			ClearMultiDamage();
			CTakeDamageInfo dmgInfo(this, this, 2, DMG_BURN); // 2 damage is arbitrary.
			tr.m_pEnt->DispatchTraceAttack(dmgInfo, forward, &tr);
			ApplyMultiDamage();
		}
	}

	m_flTimeLastUpdatedRadius = gpGlobals->curtime;

	if( m_flBurnRadius >= MAX_BURN_RADIUS )
	{
//		StopImmolating();
		m_flBurnRadius = MAX_BURN_RADIUS; // Don't stop automatically!
	}
}

// -----------------------------------------------------------------
// Set these as static variables, so they only get initialized once.
// Moved out of ItemPostFrame so Equip/Drop/Holster can access them.
// -----------------------------------------------------------------
static bool isCurrentlyFiring = false; // Are we currently firing?
static float m_flNextAttackTime = 0.0f; // What is the next attack time?

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponImmolator::ItemPostFrame( void )
{
// ------------------
// Obtain the player!
// ------------------
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

// -----------------------------------------------------------
// If we have no ammo, don't allow the Immolator to be usable.
// -----------------------------------------------------------
	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		StopImmolating();
		if (isCurrentlyFiring)
			WeaponSound(RELOAD);
		isCurrentlyFiring = false;
		return BaseClass::ItemPostFrame();
	}

// --------------------------------------------------------------------------------------------------------------------
// If the +attack button is being pressed. Don't allow the player to spam the attack button and use up all of the ammo!
// --------------------------------------------------------------------------------------------------------------------
	if ((pPlayer->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack < gpGlobals->curtime)
	{
		if (m_flNextAttackTime < gpGlobals->curtime)
		{
			pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType); // Subtract ammo.
			m_flNextAttackTime = gpGlobals->curtime + 0.25f; // Delay next attack.
		}

// -----------------------------------------------
// If we aren't immolating, start if we have ammo.
// -----------------------------------------------
		if (!IsImmolating() && !isCurrentlyFiring)
		{
			if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				StopImmolating();
				WeaponSound(RELOAD);
				isCurrentlyFiring = false;
				return BaseClass::ItemPostFrame();
			}

			WeaponSound(SINGLE);
			StartImmolating();
			isCurrentlyFiring = true;
		}
	}

// ---------------------------------------------------------
// If the +attack button has been released, stop immolating.
// ---------------------------------------------------------
	if (pPlayer->m_afButtonReleased & IN_ATTACK)
	{
		if (isCurrentlyFiring)
			WeaponSound(RELOAD);
		m_flNextAttackTime = 0.0f;
		StopImmolating();
		isCurrentlyFiring = false;
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
	}

//	BaseClass::ItemPostFrame(); // This causes the Immolator to shoot bullets!
}

void CWeaponImmolator::ImmolationDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore )
{
	CBaseEntity *pEntity = NULL;
	trace_t		tr;
	Vector		vecSpot;

	Vector vecSrc = vecSrcIn;
	CEntitySphereQuery sphere(vecSrc, flRadius); // Moved out of for loop.

	// iterate on all entities in the vicinity.
//	for ( CEntitySphereQuery sphere( vecSrc, flRadius ); pEntity = sphere.GetCurrentEntity(); sphere.NextEntity() )
	for ( sphere; sphere.GetCurrentEntity(); sphere.NextEntity() )
	{
		CBaseCombatCharacter *pBCC;
		pEntity = sphere.GetCurrentEntity(); // Moved out of for loop.

		pBCC = pEntity->MyCombatCharacterPointer();

		if ( pBCC && !pBCC->IsOnFire() )
		{
			// UNDONE: this should check a damage mask, not an ignore
			if ( iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore )
			{
				continue;
			}

			if( pEntity == GetOwner() )
			{
				continue;
			}

			pBCC->Ignite( random->RandomFloat( 15, 20 ) );
		}
	}
}

// ----------
// Additions.
// ----------
bool CWeaponImmolator::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	StopImmolating();
	isCurrentlyFiring = false;
	m_flNextAttackTime = 0.0f;
	return BaseClass::Holster(pSwitchingTo);
}

void CWeaponImmolator::Drop(const Vector& vecVelocity)
{
	StopImmolating();
	isCurrentlyFiring = false;
	m_flNextAttackTime = 0.0f;
	BaseClass::Drop(vecVelocity);
}

void CWeaponImmolator::Equip(CBaseCombatCharacter* pOwner)
{
	isCurrentlyFiring = false;
	m_flNextAttackTime = 0.0f;
	BaseClass::Equip(pOwner);
}