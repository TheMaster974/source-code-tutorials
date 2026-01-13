// ------------------------------------------------------------
// Code written by Gareth Pugh, using code from weapon_frag.cpp
// ------------------------------------------------------------

#include "cbase.h"					// Mandatory include.
#include "gamestats.h"				// Needed for gamestats stuff.
#include "grenade_frag.h"			// Needed for Flashgrenade_Create function.
#include "weapon_frag.h"			// Needed so the flashbang weapon can derive from CWeaponFrag.
#include "engine/IEngineSound.h"	// Needed to modify the player's DSP (explosion ringing sound).
#include "explode.h"				// Needed for when the weapon explodes!
#include "tier0/memdbgon.h"			// Last file to be included.

// --------------------------
// Taken from weapon_frag.cpp
// --------------------------
#define GRENADE_TIMER	3.0f //Seconds
#define GRENADE_RADIUS	4.0f // inches

// ---------------------------------------------
// Adds the flashbang/concussion grenade weapon.
// ---------------------------------------------
class CWeaponFlash : public CWeaponFrag
{
public:
	DECLARE_CLASS(CWeaponFlash, CWeaponFrag);
	DECLARE_SERVERCLASS();

	void Precache(void); // Initialize any assets that will be used. 

// --------------------------------------------------------------------------
// Override the grenade throwing functions so a flash grenade will be thrown.
// --------------------------------------------------------------------------
	void ThrowGrenade(CBasePlayer* pPlayer);
	void LobGrenade(CBasePlayer* pPlayer);
	void RollGrenade(CBasePlayer* pPlayer);

// ---------------------------------------------------
// Also override the explosion when this is destroyed!
// ---------------------------------------------------
	void Event_Killed(const CTakeDamageInfo& info);
};

// --------------------------------
// Declare a Serverclass like this.
// --------------------------------
IMPLEMENT_SERVERCLASS_ST(CWeaponFlash, DT_WeaponFlash)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_flash, CWeaponFlash);
PRECACHE_WEAPON_REGISTER(weapon_flash);

// -----------------------
// Initialize assets here.
// -----------------------
void CWeaponFlash::Precache(void)
{
	BaseClass::Precache();

	UTIL_PrecacheOther("npc_grenade_flash"); // Precache the flash grenade.
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponFlash::ThrowGrenade(CBasePlayer* pPlayer)
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors(&vForward, &vRight, NULL);
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
	BaseClass::CheckThrowPosition(pPlayer, vecEye, vecSrc);
	//	vForward[0] += 0.1f;
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vForward * 1200;
	Flashgrenade_Create(vecSrc, vec3_angle, vecThrow, AngularImpulse(600, random->RandomInt(-1200, 1200), 0), pPlayer, GRENADE_TIMER, false);

	m_bRedraw = true;

	WeaponSound(SINGLE);

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponFlash::LobGrenade(CBasePlayer* pPlayer)
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors(&vForward, &vRight, NULL);
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f + Vector(0, 0, -8);
	CheckThrowPosition(pPlayer, vecEye, vecSrc);

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vForward * 350 + Vector(0, 0, 50);
	Flashgrenade_Create(vecSrc, vec3_angle, vecThrow, AngularImpulse(200, random->RandomInt(-600, 600), 0), pPlayer, GRENADE_TIMER, false);

	WeaponSound(WPN_DOUBLE);

	m_bRedraw = true;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponFlash::RollGrenade(CBasePlayer* pPlayer)
{
	// BUGBUG: Hardcoded grenade width of 4 - better not change the model :)
	Vector vecSrc;
	pPlayer->CollisionProp()->NormalizedToWorldSpace(Vector(0.5f, 0.5f, 0.0f), &vecSrc);
	vecSrc.z += GRENADE_RADIUS;

	Vector vecFacing = pPlayer->BodyDirection2D();
	// no up/down direction
	vecFacing.z = 0;
	VectorNormalize(vecFacing);
	trace_t tr;
	UTIL_TraceLine(vecSrc, vecSrc - Vector(0, 0, 16), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction != 1.0)
	{
		// compute forward vec parallel to floor plane and roll grenade along that
		Vector tangent;
		CrossProduct(vecFacing, tr.plane.normal, tangent);
		CrossProduct(tr.plane.normal, tangent, vecFacing);
	}
	vecSrc += (vecFacing * 18.0);
	CheckThrowPosition(pPlayer, pPlayer->WorldSpaceCenter(), vecSrc);

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vecFacing * 700;
	// put it on its side
	QAngle orientation(0, pPlayer->GetLocalAngles().y, -90);
	// roll it
	AngularImpulse rotSpeed(0, 0, 720);
	Flashgrenade_Create(vecSrc, orientation, vecThrow, rotSpeed, pPlayer, GRENADE_TIMER, false);

	WeaponSound(SPECIAL1);

	m_bRedraw = true;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
}

#define CONC_FALLOFF_TIME 5.0f // Taken from grenade_flash.cpp

// -----------------------------------------------------------
// Cause an explosion that displays a flash to nearby players.
// -----------------------------------------------------------
void CWeaponFlash::Event_Killed(const CTakeDamageInfo& info)
{
	CBaseEntity* list[1024]; // Hold all entities that are within the range of the flashbang.

	Vector start = GetAbsOrigin() + Vector(0, 0, 10); // Displace the flashbang's position slightly, changed Vector(0, 10, 0) to Vector(0, 0, 10).

	int count = UTIL_EntitiesInSphere(list, 1024, start, 400, MASK_PLAYERSOLID); // Find all entities that are within 400 units of the flashbang.

	for (int i = 0; i < count; i++) // For every entity that is in range...
	{
		if (list[i]->IsPlayer()) // ...check to see if they are a player.
		{
			trace_t tr; // Trace line to make.
			UTIL_TraceLine(list[i]->GetAbsOrigin(), start, MASK_PLAYERSOLID, NULL, COLLISION_GROUP_NONE, &tr);

			CBasePlayer* pPlayer = ToBasePlayer(list[i]); // Find the player.

			float distancesq = (pPlayer->GetAbsOrigin() - start).LengthSqr(); // distance^2 between the player and the flashbang.

			if (distancesq > 160000) // If the entity is greater than 400 units away, skip it. 400^2 = 160,000.
				break;

			float ratio = 1.0; // This is a ratio of the player's distance from the flashbang versus the range of the flashbang.

			// If we are greater than 200 units away from the flashbang, allow the ratio to falloff. 200^2 = 40,000.
			if (distancesq >= 40000)
				ratio = 1.0f - pow((FastSqrt(distancesq) / 200.0) - 1, 2); // 1 - ((distance/200) - 1)^2

			// -----------------------------------------------------------------
			// Uncomment for information about the distance and ratio variables.
			// -----------------------------------------------------------------
			//			Msg("distancesq = %.3f\n distance = %.3f\n ratio = %.3f\n", distancesq, FastSqrt(distancesq), ratio);

			color32 white = { 255,255,255,255 * ratio }; // Colour of the screen flash, incorporate the ratio into the alpha.

			int fadehold = random->RandomInt(0, 4); // Create a random time to add to the screen flash duration.

			if (tr.fraction != 1.0f) // If we hit something...
			{
				if (pPlayer->FInViewCone(this)) // ...and if the player can see the flashbang, create a screen flash.
					UTIL_ScreenFade(pPlayer, white, CONC_FALLOFF_TIME + (.5f * fadehold * ratio), fadehold * ratio, FFADE_IN);

				// ---------------------------------------------------------------
				// This is the explosion sound effect that the player experiences.
				// ---------------------------------------------------------------
				int effect = random->RandomInt(0, 1) ? random->RandomInt(35, 37) : random->RandomInt(32, 34);

				CSingleUserRecipientFilter user(pPlayer);
				enginesound->SetPlayerDSP(user, effect, false); // Apply the explosion sound effect.
			}
		}
	}

	m_takedamage = DAMAGE_NO; // No longer take damage.

	UTIL_ScreenShake(GetAbsOrigin(), 25.0, 150.0, 1.0, 750, SHAKE_START); // Screen shake.

// ----------------------------
// Explode, but with no damage!
// ----------------------------
	ExplosionCreate(GetAbsOrigin() + Vector(0, 0, 16), GetAbsAngles(), NULL, 0, 0,
		SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this);

	UTIL_Remove(this);
}