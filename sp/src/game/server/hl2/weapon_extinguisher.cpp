//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Restores a fire extinguisher weapon. Uses a particle system instead
//			of drawing particles on the Client, this needs to be added to your
//			particles folder and added to particles_manifest.txt. If the extinguish
//			decal is too large, modify the extinguish.vmt file to change the
//			$decalscale variable to be something like 0.125.
//			Also, thanks to Mazdo for pointing out an issue with particles/sounds
//			continuing to play when dropping or holstering this weapon!
//
// $NoKeywords: $FixedByTheMaster974
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "gamerules.h"
#include "game.h"
#include "in_buttons.h"
//#include "extinguisherjet.h" // Commented out.
#include "entitylist.h"
#include "fire.h"
#include "ar2_explosion.h"
#include "ndebugoverlay.h"
#include "engine/IEngineSound.h"

// ----------
// Additions.
// ----------
#include "particle_parse.h" // Needed to be able to access particle system code.
#include "decals.h" // Needed to be able to draw decals on the Server.

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	fire_extinguisher_debug( "fire_extinguisher_debug", "0" );
ConVar	fire_extinguisher_radius( "fire_extinguisher_radius", "45" );
ConVar	fire_extinguisher_distance( "fire_extinguisher_distance", "200" );
ConVar	fire_extinguisher_strength( "fire_extinguisher_strength", "0.97" );	//TODO: Stub for real numbers
ConVar	fire_extinguisher_explode_radius( "fire_extinguisher_explode_radius", "256" );
ConVar	fire_extinguisher_explode_strength( "fire_extinguisher_explode_strength", "0.3" );	//TODO: Stub for real numbers

#define	EXTINGUISHER_AMMO_RATE	0.2

extern short	g_sModelIndexFireball;	// (in combatweapon.cpp) holds the index for the smoke cloud

class CWeaponExtinguisher: public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponExtinguisher, CHLSelectFireMachineGun );

	DECLARE_SERVERCLASS();

	CWeaponExtinguisher();

	void	Spawn( void );
	void	Precache( void );

	void	ItemPostFrame( void );
	void	Event_Killed( const CTakeDamageInfo &info );
	void	Equip( CBaseCombatCharacter *pOwner );

// -------------------------------------------------------
// Additions, thanks to Mazdo for pointing this out to me!
// -------------------------------------------------------
	bool	Holster(CBaseCombatWeapon* pSwitchingTo);
	void	Drop(const Vector& vecVelocity);
	void	DecrementAmmo(CBaseCombatCharacter* pOwner);
	void	OnPickedUp(CBaseCombatCharacter* pOwner);
	int		extinguisher_ammo_to_subtract; // This is to ensure the correct amount of ammo is subtracted when the weapon is dropped.

protected:
	
	void	StartJet( void );
	void	StopJet( void );

//	CExtinguisherJet	*m_pJet; // Commented out.
	float m_flNextDecalDrawTime; // Addition.
	bool isActive; // Addition.
};

IMPLEMENT_SERVERCLASS_ST(CWeaponExtinguisher, DT_WeaponExtinguisher)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_extinguisher, CWeaponExtinguisher );
PRECACHE_WEAPON_REGISTER( weapon_extinguisher );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CWeaponExtinguisher )

//	DEFINE_FIELD( m_pJet,	FIELD_CLASSPTR ),
	DEFINE_FIELD( m_flNextDecalDrawTime, FIELD_TIME ),
	DEFINE_FIELD( isActive, FIELD_BOOLEAN ),

END_DATADESC()


CWeaponExtinguisher::CWeaponExtinguisher( void )
{
//	m_pJet		= NULL;
	isActive = false;
	extinguisher_ammo_to_subtract = 0; // Addition.
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::Precache( void )
{
	BaseClass::Precache();
	PrecacheScriptSound( "ExtinguisherCharger.Use" );
//	UTIL_PrecacheOther( "env_extinguisherjet" );

// ---------------------------------------------------------------------
// Additions. The particle system here is NOT the name of the .pcf file!
// ---------------------------------------------------------------------
	m_flNextDecalDrawTime = 0;
	PrecacheParticleSystem("Fire_EX1");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::Spawn( void )
{
	BaseClass::Spawn();

	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 25;//FIXME: Define
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );

	m_takedamage	= DAMAGE_NO;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pInflictor - 
//			*pAttacker - 
//			flDamage - 
//			bitsDamageType - 
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::Event_Killed( const CTakeDamageInfo &info )
{
	//TODO: Use a real effect
	if ( AR2Explosion *pExplosion = AR2Explosion::CreateAR2Explosion( GetAbsOrigin() ) )
	{
		pExplosion->SetLifetime( 10 );
	}		

	//TODO: Use a real effect
	CPASFilter filter( GetAbsOrigin() );

	te->Explosion( filter, 0.0,
		&GetAbsOrigin(), 
		g_sModelIndexFireball,
		2.0, 
		15,
		TE_EXPLFLAG_NONE,
		250,
		100 );

	//Put out fire in a radius
	FireSystem_ExtinguishInRadius( GetAbsOrigin(), fire_extinguisher_explode_radius.GetInt(), fire_extinguisher_explode_strength.GetFloat() );

// -------------------------------
// Modified this section slightly.
// -------------------------------
//	SetThink( &CBaseEntity::SUB_Remove ); // Proper pointer.
//	SetNextThink( gpGlobals->curtime + 0.1f );
	UTIL_Remove(this);
}

//-----------------------------------------------------------------------------
// Purpose: Turn the jet effect and noise on
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::StartJet( void )
{
// ------------------
// Modified function.
// ------------------
	isActive = true; // The fire extinguisher is active!
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner()); // Find the player.
	if (!pPlayer)
		return;
	DispatchParticleEffect("Fire_EX1", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "muzzle", false); // Create the particle system at the weapon's muzzle attachment.

	/*
	//See if the jet needs to be created
	if ( m_pJet == NULL )
	{
		m_pJet = (CExtinguisherJet *) CreateEntityByName( "env_extinguisherjet" );

		if ( m_pJet == NULL )
		{
			Msg( "Unable to create jet for weapon_extinguisher!\n" );
			return;
		}

		//Setup the jet
		m_pJet->m_bEmit	= false;
		UTIL_SetOrigin( m_pJet, GetAbsOrigin() );
		m_pJet->SetParent( this );

		m_pJet->m_bUseMuzzlePoint = true;
		m_pJet->m_bAutoExtinguish = false;
		m_pJet->m_nLength = fire_extinguisher_distance.GetInt();
	}

	//Turn the jet on
	if ( m_pJet != NULL )
	{
		m_pJet->TurnOn();
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose: Turn the jet effect and noise off
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::StopJet( void )
{
// ------------------
// Modified function.
// ------------------
	isActive = false; // The fire extinguisher is no longer active.
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner()); // Find the player.
	if (!pPlayer)
		return;
	StopParticleEffects(pPlayer->GetViewModel()); // Stop all particle effects on the viewmodel.

	/*
	//Turn the jet off
	if ( m_pJet != NULL )
	{
		m_pJet->TurnOff();
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponExtinguisher::ItemPostFrame( void )
{	
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	static float m_flNextAttackTime = 0.0f; // What is the next attack time?

	//Only shoot if we have ammo
	if ( pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0 )
	{
		if (isActive)
		{
			StopWeaponSound(SINGLE); // Stop active weapon sound.
			WeaponSound(SPECIAL1); // Play a new weapon sound.
			SendWeaponAnim(ACT_VM_IDLE); // Play an idle animation.
		}
		StopJet();
		StopParticleEffects(pOwner->GetViewModel()); // Addition.
		return;
	}
	
	//See if we should try and extinguish fires
	if (pOwner->m_nButtons & IN_ATTACK && m_flNextPrimaryAttack < gpGlobals->curtime) // Added next primary attack check.
	{
		WeaponSound(SINGLE);
		if (!isActive)
		{
			SendWeaponAnim(ACT_VM_PRIMARYATTACK); // Play primary attack animation.
			//			pOwner->SetAnimation(PLAYER_ATTACK1); // Is this necessary?
		}

		//Drain ammo
//		if ( m_flNextPrimaryAttack < gpGlobals->curtime  )
// -----------------------------------------------------------------------------
// Don't allow the player to spam the attack button and consume all of the ammo!
// -----------------------------------------------------------------------------
		if (m_flNextAttackTime < gpGlobals->curtime)
		{
			pOwner->RemoveAmmo(1, m_iSecondaryAmmoType);
			extinguisher_ammo_to_subtract += 1; // Addition.
			//			m_flNextPrimaryAttack = gpGlobals->curtime + EXTINGUISHER_AMMO_RATE;
			m_flNextAttackTime = gpGlobals->curtime + EXTINGUISHER_AMMO_RATE;
		}

		//If we're just run out...
		if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
		{
			if (isActive)
			{
				StopWeaponSound(SINGLE); // Stop active weapon sound.
				WeaponSound(SPECIAL1); // Play a new weapon sound.
				SendWeaponAnim(ACT_VM_IDLE); // Play an idle animation.
			}
			StopJet();
			return;
		}

		//Turn the jet on
		StartJet();

		Vector	vTestPos, vMuzzlePos;
		Vector	vForward, vRight, vUp;

		pOwner->EyeVectors(&vForward, &vRight, &vUp);

		vMuzzlePos = pOwner->Weapon_ShootPosition();

		//FIXME: Need to get the exact same muzzle point!

		//FIXME: This needs to be adjusted so the server collision matches the visuals on the client
		vMuzzlePos += vForward * 15.0f;
		vMuzzlePos += vRight * 6.0f;
		vMuzzlePos += vUp * -4.0f;

		QAngle aTmp;
		VectorAngles(vForward, aTmp);
		aTmp[PITCH] += 10;
		AngleVectors(aTmp, &vForward);

		//		vTestPos	= vMuzzlePos + ( vForward * fire_extinguisher_distance.GetInt() );
		vTestPos = vMuzzlePos + (vForward * 8192);

		trace_t	tr;
		UTIL_TraceLine(vMuzzlePos, vTestPos, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		Vector end = tr.endpos; // End point of the trace.
		Vector difference = end - vMuzzlePos; // End - start.
		float distance = difference.Length(); // Distance from start to end.

// -----------------------------------------------------------------------------------------
// Contain this code in an if statement to determine if fires should be extinguished or not.
// -----------------------------------------------------------------------------------------
		if (distance <= fire_extinguisher_distance.GetFloat())
		{
			// Decal drawing.
			if (m_flNextDecalDrawTime < gpGlobals->curtime)
			{
				int index = decalsystem->GetDecalIndexForName("Extinguish");
				CBroadcastRecipientFilter filter;

// ---------------------------------------------------------------------------------
// This section is entirely dependent on your particle system. If the particles have
// constant speed, you can use time = distance / speed to get the delay time.
// Otherwise, as is the case for Fire_EX1 from Missing Information, you need to
// use one of the SUVAT equations, so delay time = 2*distance/speed. However, I
// end up using some stock values, change these as you see fit!
// ---------------------------------------------------------------------------------
				float speed = RandomFloat(210, 275); // Particle system starting speed.
				float delay; // How long should it take before the decal is drawn?
				if (distance <= 95)
					delay = 0.0f; // If the end position is close to us, don't delay.
				else
					delay = distance / speed; // Otherwise, use time = distance / speed.

				te->Decal(filter, delay, &tr.endpos, &tr.startpos, tr.GetEntityIndex(), tr.hitbox, index); // Create the decal.
				m_flNextDecalDrawTime = gpGlobals->curtime + 0.05f; // Delay next decal draw time.
			}

			//Extinguish the fire where we hit
			FireSystem_ExtinguishInRadius(tr.endpos, fire_extinguisher_radius.GetInt(), fire_extinguisher_strength.GetFloat());

			//Debug visualization
			if (fire_extinguisher_debug.GetInt())
			{
				int	radius = fire_extinguisher_radius.GetInt();

				NDebugOverlay::Line(vMuzzlePos, tr.endpos, 0, 0, 128, false, 0.0f);

				NDebugOverlay::Box(vMuzzlePos, Vector(-1, -1, -1), Vector(1, 1, 1), 0, 0, 128, false, 0.0f);
				NDebugOverlay::Box(tr.endpos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 0, 128, false, 0.0f);
				NDebugOverlay::Box(tr.endpos, Vector(-radius, -radius, -radius), Vector(radius, radius, radius), 0, 0, 255, false, 0.0f);
			}
		}
	}
//	else
// --------------------------------------------------------
// If the +attack button has been released, stop operation.
// --------------------------------------------------------
	if(pOwner->m_afButtonReleased & IN_ATTACK)
	{
		if (isActive)
		{
			StopWeaponSound(SINGLE); // Stop active weapon sound.
			WeaponSound(SPECIAL1); // Play a new weapon sound.
			SendWeaponAnim(ACT_VM_IDLE); // Play an idle animation.
		}
// ---------------------------------------------------
// Reset variables, stop the particle system and idle.
// ---------------------------------------------------
		m_flNextDecalDrawTime = 0;
		m_flNextAttackTime = 0.0f;
		m_flNextPrimaryAttack = gpGlobals->curtime + EXTINGUISHER_AMMO_RATE;
		StopJet();
		WeaponIdle(); // Addition.
	}
}

// ---------------------------------------------------------------------------------------------
// Additions, this is to stop the firing sound and particles when switching/dropping the weapon.
// ---------------------------------------------------------------------------------------------
bool CWeaponExtinguisher::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	StopJet();
	StopParticleEffects(ToBasePlayer(GetOwner())->GetViewModel());
	StopWeaponSound(SINGLE);
	return BaseClass::Holster(pSwitchingTo);
}

void CWeaponExtinguisher::Drop(const Vector& vecVelocity)
{
	StopJet();
	StopParticleEffects(ToBasePlayer(GetOwner())->GetViewModel());
	StopWeaponSound(SINGLE);
	DecrementAmmo(GetOwner());
	m_takedamage = DAMAGE_YES;
	BaseClass::Drop(vecVelocity);
}

// This subtracts ammo when the weapon is dropped.
void CWeaponExtinguisher::DecrementAmmo(CBaseCombatCharacter* pOwner)
{
	pOwner->RemoveAmmo(extinguisher_ammo_to_subtract, m_iSecondaryAmmoType);
}

// Subtract the right amount of ammo from the player when this weapon is picked up again.
void CWeaponExtinguisher::OnPickedUp(CBaseCombatCharacter* pOwner)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	DecrementAmmo(pPlayer);
	BaseClass::OnPickedUp(pOwner);
}

// --------------------------------------------------
// This is an optional entity to include in your mod!
// --------------------------------------------------
class CExtinguisherCharger : public CBaseToggle
{
public:
	DECLARE_CLASS( CExtinguisherCharger, CBaseToggle );

	void Spawn( void );
	bool CreateVPhysics();
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int	ObjectCaps( void ) { return (BaseClass::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }

protected:
	float	m_flNextCharge; 
	bool	m_bSoundOn;

	void	TurnOff( void );

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( func_extinguishercharger, CExtinguisherCharger );

BEGIN_DATADESC( CExtinguisherCharger )

	DEFINE_FIELD( m_flNextCharge, FIELD_TIME),
	DEFINE_FIELD( m_bSoundOn, FIELD_BOOLEAN ),

	DEFINE_FUNCTION( TurnOff ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Spawn the object
//-----------------------------------------------------------------------------
void CExtinguisherCharger::Spawn( void )
{
	Precache();

	SetSolid( SOLID_BSP );
	SetMoveType( MOVETYPE_PUSH );

	SetModel( STRING( GetModelName() ) );

	m_bSoundOn = false;

	CreateVPhysics();
}

//-----------------------------------------------------------------------------

bool CExtinguisherCharger::CreateVPhysics()
{
	VPhysicsInitStatic();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CExtinguisherCharger::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// Make sure that we have a caller
	if ( pActivator == NULL )
		return;
	
	// If it's not a player, ignore
	if ( pActivator->IsPlayer() == false )
		return;

	// Turn our sound on, if it's not already
	if ( m_bSoundOn == false )
	{
		EmitSound( "ExtinguisherCharger.Use" );
		m_bSoundOn = true;
	}

	SetNextThink( gpGlobals->curtime + 0.25 );
	
	SetThink( &CExtinguisherCharger::TurnOff ); // Proper pointer.

	CBasePlayer	*pPlayer = ToBasePlayer( pActivator );

// ----------------------------------------------------------------------------------------------------------
// This section has been modified slightly, only regain ammo when the Fire Extinguisher is the active weapon.
// TODO: Investigate this further to ensure appropriate functionality!
// ----------------------------------------------------------------------------------------------------------
	if (pPlayer)
	{
		CBaseCombatWeapon* pTestWeapon = pPlayer->Weapon_OwnsThisType("weapon_extinguisher");
		CWeaponExtinguisher* pExtinguisher = dynamic_cast<CWeaponExtinguisher*>(pTestWeapon);

		if (pExtinguisher)
		{
			//FIXME: Need a way to do this silently
			pPlayer->GiveAmmo(1, "extinguisher");
			pExtinguisher->extinguisher_ammo_to_subtract -= 1;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExtinguisherCharger::TurnOff( void )
{
	//Turn the sound off
	if ( m_bSoundOn )
	{
		StopSound( "ExtinguisherCharger.Use" );
		m_bSoundOn = false;
	}
}
