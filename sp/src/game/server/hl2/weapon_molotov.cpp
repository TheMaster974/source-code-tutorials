//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: This is the molotov weapon
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $FixedByTheMaster974
//=============================================================================//

#include "cbase.h"
#include	"npcevent.h"
#include	"basehlcombatweapon.h"
#include	"basecombatcharacter.h"
#include	"ai_basenpc.h"
#include	"AI_Memory.h"
#include	"player.h"
#include	"gamerules.h"		// For g_pGameRules
#include	"weapon_molotov.h"
#include	"grenade_molotov.h"
#include	"in_buttons.h"
#include	"game.h"			
#include "vstdlib/random.h"
#include "movevars_shared.h"

// ----------
// Additions.
// ----------
#include "gamestats.h"
#include "fire.h"
#include "explode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CWeaponMolotov )

//	DEFINE_FIELD( m_nNumAmmoTypes, FIELD_INTEGER ),
//	DEFINE_FIELD( m_bNeedDraw, FIELD_BOOLEAN ),
//	DEFINE_FIELD( m_iThrowBits, FIELD_INTEGER ),
//	DEFINE_FIELD( m_fNextThrowCheck, FIELD_TIME ),
	DEFINE_FIELD( m_bRedraw, FIELD_BOOLEAN ), // Addition.
	DEFINE_FIELD( m_vecTossVelocity, FIELD_VECTOR ),

	// Function Pointers
//	DEFINE_FUNCTION( MolotovTouch ),

END_DATADESC()


IMPLEMENT_SERVERCLASS_ST(CWeaponMolotov, DT_WeaponMolotov)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_molotov, CWeaponMolotov );
PRECACHE_WEAPON_REGISTER(weapon_molotov);

acttable_t	CWeaponMolotov::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_THROW, true },
};
IMPLEMENT_ACTTABLE(CWeaponMolotov);


void CWeaponMolotov::Precache( void )
{
//	PrecacheModel("models/props_junk/w_garb_beerbottle.mdl");	//<<TEMP>> need real model
	PrecacheModel("models/weapons/w_molotov.mdl");
	UTIL_PrecacheOther("grenade_molotov"); // Addition.
	BaseClass::Precache();
}

void CWeaponMolotov::Spawn( void )
{
	// Call base class first
	BaseClass::Spawn();

// -----------------------
// Additions/Subtractions.
// -----------------------
	m_takedamage = DAMAGE_YES;
	m_iHealth = 15;

//	m_bNeedDraw		= true;

//	SetModel( GetWorldModel() );
//	UTIL_SetSize(this, Vector( -6, -6, -2), Vector(6, 6, 2));
}
/*
//------------------------------------------------------------------------------
// Purpose : Override to use molotovs pickup touch function
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CWeaponMolotov::SetPickupTouch( void )
{
	SetTouch(MolotovTouch);
}
*/
/*
//-----------------------------------------------------------------------------
// Purpose: Override so give correct ammo
// Input  : pOther - the entity that touched me
// Output :
//-----------------------------------------------------------------------------
void CWeaponMolotov::MolotovTouch( CBaseEntity *pOther )
{
	// ---------------------------------------------------
	//  First give weapon to touching entity if allowed
	// ---------------------------------------------------
	BaseClass::DefaultTouch(pOther);

	// ----------------------------------------------------
	//  Give molotov ammo if touching client
	// ----------------------------------------------------
	if (pOther->GetFlags() & FL_CLIENT)
	{
		// ------------------------------------------------
		//  If already owned weapon of this type remove me
		// ------------------------------------------------
		CBaseCombatCharacter* pBCC = ToBaseCombatCharacter( pOther );
		CWeaponMolotov* oldWeapon = (CWeaponMolotov*)pBCC->Weapon_OwnsThisType( GetClassname() );
		if (oldWeapon != this)
		{
			UTIL_Remove( this );
		}
		else
		{
			pBCC->GiveAmmo( 1, m_iSecondaryAmmoType );
			SetThink (NULL);
		}
	}
}
*/
//-----------------------------------------------------------------------------
// Purpose: Gets event from anim stream and throws the object
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponMolotov::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
// ------------------
// Modified function.
// ------------------
	bool fThrewMolotov = false; // Has the molotov been thrown?
	switch( pEvent->event )
	{
		case EVENT_WEAPON_THROW:
			ThrowMolotov(pOperator); // Throw the molotov!
			pOperator->RemoveAmmo(1, m_iSecondaryAmmoType); // Subtract 1 from the ammo count.
			fThrewMolotov = true; // The molotov has been thrown!
			break;
		/*
			CAI_BaseNPC *pNPC	= GetOwner()->MyNPCPointer();
			if (!pNPC)
			{
				return;
			}

			CBaseEntity *pEnemy = pNPC->GetEnemy();
			if (!pEnemy)
			{
				return;
			}

			Vector vec_target = pNPC->GetEnemyLKP();

			// -----------------------------------------------------
			//  Get position of throw
			// -----------------------------------------------------
			// If owner has a hand, set position to the hand bone position
			Vector launchPos;
			int iBIndex = pNPC->LookupBone("Bip01 R Hand");
			if (iBIndex != -1) 
			{
				Vector origin;
				QAngle angles;
				pNPC->GetBonePosition( iBIndex, launchPos, angles);
			}
			// Otherwise just set to in front of the owner
			else 
			{
				Vector vFacingDir = pNPC->BodyDirection2D( );
				vFacingDir = vFacingDir * 60.0; 
				launchPos = pNPC->GetAbsOrigin()+vFacingDir;
			}


			//Vector vecVelocity = VecCheckToss( pNPC, launchPos, vec_target, 1.0 );

			ThrowMolotov( launchPos, m_vecTossVelocity);

			// Drop the weapon and remove as no more ammo
			pNPC->Weapon_Drop( this );
			UTIL_Remove( this );
		*/
		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}

#define RETHROW_DELAY 1.0f; // Arbitrary rethrow delay.
	if (fThrewMolotov)
		m_flNextPrimaryAttack = gpGlobals->curtime + RETHROW_DELAY; // Delay next throw.
}

/*
//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponMolotov::ObjectInWay( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	if (!pOwner)
	{
		return false;
	}

	Vector vecSrc	 = pOwner->Weapon_ShootPosition( );
	Vector vecAiming = pOwner->BodyDirection2D( );

	trace_t tr;

	Vector	vecEnd = vecSrc + (vecAiming * 32);
	UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );
	
	if (tr.fraction < 1.0)
	{
		// Don't block on a living creature
		if (tr.m_pEnt)
		{
			CBaseEntity *pEntity = tr.m_pEnt;
			CBaseCombatCharacter *pBCC		= ToBaseCombatCharacter( pEntity );
			if (pBCC)
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}
*/
/*
//-----------------------------------------------------------------------------
// Purpose: Override to allow throw w/o LOS
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponMolotov::WeaponLOSCondition(const Vector &ownerPos, const Vector &targetPos,bool bSetConditions)
{
	// <<TODO>> should test if can throw from present location here...
	return true;
}
*/
/*
//-----------------------------------------------------------------------------
// Purpose: Override to check throw
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CWeaponMolotov::WeaponRangeAttack1Condition( float flDot, float flDist )
{
	// If things haven't changed too much since last time
	// just return that previously calculated value
	if (gpGlobals->curtime < m_fNextThrowCheck )
	{
		return m_iThrowBits;
	}
	
	if ( flDist < m_fMinRange1) {
		m_iThrowBits = COND_TOO_CLOSE_TO_ATTACK;
	}
	else if (flDist > m_fMaxRange1) {
		m_iThrowBits = COND_TOO_FAR_TO_ATTACK;
	}
	else if (flDot < 0.5) {
		m_iThrowBits = COND_NOT_FACING_ATTACK;
	}

	// If moving, can't throw.
	else if ( m_flGroundSpeed != 0 )
	{
		m_iThrowBits = COND_NONE;
	}
	else {
		// Ok we should check again as some time has passed
		// This function is only used by NPC's so we can cast to a Base Monster
		CAI_BaseNPC *pNPC	= GetOwner()->MyNPCPointer();
		CBaseEntity *pEnemy = pNPC->GetEnemy();

		if (!pEnemy)
		{
			m_iThrowBits = COND_NONE;
		}
		// Get Enemy Position 
		Vector vecTarget;
		pEnemy->CollisionProp()->NormalizedToWorldSpace( Vector( 0.5f, 0.5f, 0.0f ), &vecTarget );

		// Get Toss Vector
		Vector			throwStart  = pNPC->Weapon_ShootPosition();
		Vector			vecToss;
		CBaseEntity*	pBlocker	= NULL;
		float			throwDist	= (throwStart - vecTarget).Length();
		float			fGravity	= GetCurrentGravity();
		float			throwLimit	= pNPC->ThrowLimit(throwStart, vecTarget, fGravity, 35, WorldAlignMins(), WorldAlignMaxs(), pEnemy, &vecToss, &pBlocker);

		// If I can make the throw (or most of the throw)
		if (!throwLimit || (throwLimit != throwDist && throwLimit > 0.8*throwDist))
		{
			m_vecTossVelocity = vecToss;
			m_iThrowBits = COND_CAN_RANGE_ATTACK1;

		}
		else
		{
			m_iThrowBits = COND_NONE;
		}

	}
	// don't check again for a while.
	m_fNextThrowCheck = gpGlobals->curtime + 0.33; // 1/3 second.

	return m_iThrowBits;
}
*/
/*
//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponMolotov::ThrowMolotov( const Vector &vecSrc, const Vector &vecVelocity)
{
	CGrenade_Molotov *pMolotov = (CGrenade_Molotov*)Create( "grenade_molotov", vecSrc, vec3_angle, GetOwner() );

	if (!pMolotov)
	{
		Msg("Couldn't make molotov!\n");
		return;
	}
	pMolotov->SetAbsVelocity( vecVelocity );

	// Tumble through the air
	QAngle angVel( random->RandomFloat ( -100, -500 ), random->RandomFloat ( -100, -500 ), random->RandomFloat ( -100, -500 ) ); 
	pMolotov->SetLocalAngularVelocity( angVel );

	pMolotov->SetThrower( GetOwner() );
	pMolotov->SetOwnerEntity( ((CBaseEntity*)GetOwner()) );
}
*/
//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponMolotov::PrimaryAttack( void )
{
// ------------------
// Modified function.
// ------------------
	if (m_bRedraw) // If we need to redraw, don't attack.
		return;

	CBaseCombatCharacter* pOwner = GetOwner(); // Find the molotov's owner.
	if (!pOwner)
		return;

	CBasePlayer* pPlayer = ToBasePlayer(GetOwner()); // Find the player.
	if (!pPlayer)
		return;

	ThrowMolotov(pPlayer); // Throw molotov!
	pOwner->RemoveAmmo(1, m_iSecondaryAmmoType); // Subtract 1 from the ammo count.
	SendWeaponAnim(ACT_VM_THROW); // Play the throwing animation.
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.75; // Delay next throw, 0.75 seconds is arbitrary.

	if (!HasSecondaryAmmo())
		pPlayer->SwitchToNextBestWeapon(this); // If we run out of ammo, switch weapons.

	/*
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

	Vector vecSrc		= pPlayer->WorldSpaceCenter();
	Vector vecFacing	= pPlayer->BodyDirection3D( );
	vecSrc				= vecSrc + vecFacing * 18.0;
	// BUGBUG: is this some hack because it's not at the eye position????
	vecSrc.z		   += 24.0f;

	// Player may have turned to face a wall during the throw anim in which case
	// we don't want to throw the SLAM into the wall
	if (ObjectInWay())
	{
		vecSrc   = pPlayer->WorldSpaceCenter() + vecFacing * 5.0;
	}

	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	vecAiming.z += 0.20; // Raise up so passes through reticle

	ThrowMolotov(vecSrc, vecAiming*800);
	pPlayer->RemoveAmmo( 1, m_iSecondaryAmmoType );

	
	// Don't fire again until fire animation has completed
	//m_flNextPrimaryAttack = gpGlobals->curtime + CurSequenceDuration();
	//<<TEMP>> - till real animation is avaible
	m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;

	m_bNeedDraw = true;
	*/
}
/*/
//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponMolotov::SecondaryAttack( void )
{
	//<<TEMP>>
	// Hmmm... what should I do here?
}
*/
/*
//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponMolotov::DrawAmmo( void )
{
	// -------------------------------------------
	// Make sure I have ammo of the current type
	// -------------------------------------------
	CBaseCombatCharacter *pOwner = GetOwner();
	if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) <=0)
	{
		pOwner->Weapon_Drop( this );
		UTIL_Remove(this);
		return;
	}
	Msg("Drawing Molotov...\n");
	m_bNeedDraw = false;

	//<<TEMP>> - till real animation is avaible
	m_flNextPrimaryAttack	= gpGlobals->curtime + 2.0;
	m_flNextSecondaryAttack = gpGlobals->curtime + 2.0;

}
*/
//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do mulitple reloads in a row
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponMolotov::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
	{
		return;
	}

// ------------------
// Modified function.
// ------------------
	if ((pOwner->m_afButtonPressed & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->curtime)
		PrimaryAttack();

	if (m_bRedraw)
	{
		Reload();
		return;
	}

	BaseClass::ItemPostFrame();

	/*
	if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		SecondaryAttack();
	}
	else if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		// Uses secondary ammo only
		if (pOwner->GetAmmoCount(m_iSecondaryAmmoType))
		{
			PrimaryAttack();
		}
	}
	else if (m_bNeedDraw)
	{
		DrawAmmo();
	}
	else
	{
		WeaponIdle( );
	}
	*/
}

/*
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponMolotov::CWeaponMolotov( void )
{
#ifdef _DEBUG
	m_vecTossVelocity.Init();
#endif

	m_fMinRange1	= 200;
	m_fMaxRange1	= 1000;
}
*/

// ---------------------
// Modified constructor.
// ---------------------
CWeaponMolotov::CWeaponMolotov() :
	CBaseHLCombatWeapon(),
	m_bRedraw(false)
{
	NULL;
}

// ----------
// Additions.
// ----------
bool CWeaponMolotov::Deploy()
{
	m_bRedraw = false;
	return BaseClass::Deploy();
}

bool CWeaponMolotov::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	m_bRedraw = false;
	return BaseClass::Holster(pSwitchingTo);
}

bool CWeaponMolotov::Reload()
{
	if (!HasSecondaryAmmo()) // If we don't have ammo, don't reload!
		return false;

	if (m_bRedraw && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		SendWeaponAnim(ACT_VM_DRAW); // Play drawing animation.

		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

		m_bRedraw = false;
	}
	return true;
}

void CWeaponMolotov::CheckThrowPosition(CBasePlayer* pPlayer, const Vector& vecEye, Vector& vecSrc)
{
// ---------------------------------------
// Trace to see what the molotov will hit.
// ---------------------------------------
	trace_t tr;
	UTIL_TraceHull(vecEye, vecSrc, -Vector(6, 6, 6), Vector(6, 6, 6), pPlayer->PhysicsSolidMaskForEntity(),
		pPlayer, pPlayer->GetCollisionGroup(), &tr);

// --------------------------------------------------
// If the trace hits something, change the end point.
// --------------------------------------------------
	if (tr.DidHit())
		vecSrc = tr.endpos;
}

void CWeaponMolotov::ThrowMolotov(CBaseCombatCharacter *pThrower)
{
// ----------------------------------------
// Get throw position, angles and velocity.
// ----------------------------------------
	Vector vecEye = pThrower->EyePosition();
	Vector vForward, vRight, vUp, vecSrc, vecThrow;

	if (pThrower->IsPlayer())
	{
		CBasePlayer* pPlayer = ToBasePlayer(pThrower);
		if (!pPlayer)
			return;

		pPlayer->EyeVectors(&vForward, &vRight, NULL);
		vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
		CheckThrowPosition(pPlayer, vecEye, vecSrc);
		vForward[2] += 0.1f;

		pPlayer->GetVelocity(&vecThrow, NULL);
		vecThrow += vForward * 1200;

		gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
	}
	else
	{
		pThrower->GetAttachment("anim_attachment_RH", vecSrc);
		Vector vecTarget = pThrower->GetEnemy()->GetAbsOrigin();
		vecThrow = VecCheckThrow(pThrower, vecSrc, vecTarget, 650, 1.0);
		if (vecThrow == vec3_origin)
		{
			vecThrow = VecCheckToss(pThrower, vecSrc, vecTarget, -1, 1.0, true);
			if (vecThrow == vec3_origin)
			{
				GetVectors(&vForward, NULL, &vUp);
				vecThrow = vForward * 750 + vUp * 175;
			}
		}
	}

// -----------------------------------------------------------
// Create the molotov grenade and apply some properties to it.
// -----------------------------------------------------------
	CGrenade_Molotov* pMolotov = (CGrenade_Molotov*)Create("grenade_molotov", vecSrc, vec3_angle, GetOwner());

	m_bRedraw = true; // Need to redraw the viewmodel.

	pMolotov->SetAbsVelocity(vecThrow);

	QAngle angVel(random->RandomFloat(-100, -500), random->RandomFloat(-100, -500), random->RandomFloat(-100, -500));
	pMolotov->SetLocalAngularVelocity(angVel); // Tumble in the air.
	pMolotov->SetThrower(GetOwner());
	pMolotov->SetOwnerEntity((CBaseEntity*)GetOwner());
	m_iPrimaryAttacks++;
}

// If the player drops this weapon, the right amount of ammo is subtracted.
void CWeaponMolotov::DecrementAmmo(CBaseCombatCharacter* pOwner)
{
	pOwner->RemoveAmmo(1, m_iSecondaryAmmoType);
}

// Make the weapon receive damage when dropped.
void CWeaponMolotov::Drop(const Vector& velocity)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	DecrementAmmo(pPlayer);
	m_takedamage = DAMAGE_YES;
	BaseClass::Drop(velocity);
}

// Explode! Most of this was taken from grenade_molotov.cpp.
void CWeaponMolotov::Event_Killed(const CTakeDamageInfo& info)
{
	m_takedamage = DAMAGE_NO;

	trace_t trace;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -128), MASK_SOLID_BRUSHONLY,
		this, COLLISION_GROUP_NONE, &trace);

	int contents = UTIL_PointContents(GetAbsOrigin());

	if ((contents & MASK_WATER))
	{
		UTIL_Remove(this);
		return;
	}

	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "Grenade_Molotov.Detonate");

	// Start some fires
	int i;
	QAngle vecTraceAngles;
	Vector vecTraceDir;
	trace_t firetrace;

	for (i = 0; i < 16; i++)
	{
		// build a little ray
		vecTraceAngles[PITCH] = random->RandomFloat(45, 135);
		vecTraceAngles[YAW] = random->RandomFloat(0, 360);
		vecTraceAngles[ROLL] = 0.0f;

		AngleVectors(vecTraceAngles, &vecTraceDir);

		Vector vecStart, vecEnd;

		vecStart = GetAbsOrigin() + (trace.plane.normal * 128);
		vecEnd = vecStart + vecTraceDir * 512;

		UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &firetrace);

		Vector	ofsDir = (firetrace.endpos - GetAbsOrigin());
		float	offset = VectorNormalize(ofsDir);

		if (offset > 128)
			offset = 128;

		//Get our scale based on distance
		float scale = 0.1f + (0.75f * (1.0f - (offset / 128.0f)));
		float growth = 0.1f + (0.75f * (offset / 128.0f));

		if (firetrace.fraction != 1.0)
		{
			FireSystem_StartFire(firetrace.endpos, scale, growth, 30.0f, (SF_FIRE_START_ON | SF_FIRE_SMOKELESS | SF_FIRE_NO_GLOW), (CBaseEntity*)this, FIRE_NATURAL);
		}
	}
	// End Start some fires

	ExplosionCreate(GetAbsOrigin() + Vector(0, 0, 16), GetAbsAngles(), NULL, 50, 100,
		SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this, DMG_BURN);

	CBaseEntity* pOwner;
	pOwner = GetOwnerEntity();
	SetOwnerEntity(NULL); // can't traceline attack owner if this is set

	UTIL_DecalTrace(&trace, "Scorch");

	UTIL_ScreenShake(GetAbsOrigin(), 25.0, 150.0, 1.0, 750, SHAKE_START);
	CSoundEnt::InsertSound(SOUND_DANGER, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0);

	AddEffects(EF_NODRAW);
	SetAbsVelocity(vec3_origin);
	SetNextThink(gpGlobals->curtime + 0.2);

	UTIL_Remove(this);
}