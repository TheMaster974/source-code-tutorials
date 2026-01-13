// --------------------------------------------------------------------------------------------------------------------
// Code written/fixed by Gareth Pugh, sourced from https://developer.valvesoftware.com/wiki/Making_a_concussion_grenade
// --------------------------------------------------------------------------------------------------------------------
// This is a flashbang/concussion grenade that blinds players that can see it and are within its range.
// --------------------------------------------------------------------------------------------------------------------

#include "cbase.h"					// Mandatory include.
#include "grenade_frag.h"			// Needed so the flashbang can derive from CGrenadeFrag.
#include "engine/IEngineSound.h"	// Needed to modify the player's DSP (explosion ringing sound).
#include "soundent.h"				// Needed for the grenade explosion.
#include "tier0/memdbgon.h"			// Last file to be included.

// ---------------------------------------
// Creates a flashbang/concussion grenade.
// ---------------------------------------
class CGrenadeFlash : public CGrenadeFrag
{
public:
	DECLARE_CLASS(CGrenadeFlash, CGrenadeFrag);

	void Detonate(void);	// Override the Detonate function.
	void Explode(trace_t* pTrace, int bitsDamageType); // Override the Explode function.
};

LINK_ENTITY_TO_CLASS(npc_grenade_flash, CGrenadeFlash);

#define CONC_FALLOFF_TIME 5.0f // 20.0f

// -----------------------------------------
// What happens when the flashbang explodes?
// -----------------------------------------
void CGrenadeFlash::Detonate(void)
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
			
			color32 white = { 255,255,255,255*ratio }; // Colour of the screen flash, incorporate the ratio into the alpha.

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

	BaseClass::Detonate(); // Handle the other grenade detonation code.
}

extern short g_sModelIndexFireball; // (in combatweapon.cpp) holds the index for the fireball
extern short	g_sModelIndexWExplosion; // (in combatweapon.cpp) holds the index for the underwater explosion

// ---------------------------------------------------------------------------------------------
// Override the Explode function from basegrenade_shared.cpp so the player does not take damage.
// ---------------------------------------------------------------------------------------------
void CGrenadeFlash::Explode(trace_t *pTrace, int bitsDamageType)
{
	SetModelName(NULL_STRING);//invisible
	AddSolidFlags(FSOLID_NOT_SOLID);

	m_takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if (pTrace->fraction != 1.0)
	{
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 0.6));
	}

	Vector vecAbsOrigin = GetAbsOrigin();
	int contents = UTIL_PointContents(vecAbsOrigin);

	if (pTrace->fraction != 1.0)
	{
		Vector vecNormal = pTrace->plane.normal;
		surfacedata_t* pdata = physprops->GetSurfaceData(pTrace->surface.surfaceProps);
		CPASFilter filter(vecAbsOrigin);

		te->Explosion(filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin,
			!(contents & MASK_WATER) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			m_DmgRadius * .03,
			25,
			TE_EXPLFLAG_NONE,
			0,
			0,
			&vecNormal,
			(char)pdata->game.material);
	}
	else
	{
		CPASFilter filter(vecAbsOrigin);
		te->Explosion(filter, -1.0, // don't apply cl_interp delay
			&vecAbsOrigin,
			!(contents & MASK_WATER) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			m_DmgRadius * .03,
			25,
			TE_EXPLFLAG_NONE,
			0,
			0);
	}

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0);

	// Use the thrower's position as the reported position
	Vector vecReported = GetThrower() ? GetThrower()->GetAbsOrigin() : vec3_origin;

	UTIL_DecalTrace(pTrace, "Scorch");

	EmitSound("BaseGrenade.Explode");

	SetThink(&CBaseGrenade::SUB_Remove);
	SetTouch(NULL);
	SetSolid(SOLID_NONE);

	AddEffects(EF_NODRAW);
	SetAbsVelocity(vec3_origin);

#if HL2_EPISODIC
	// Because the grenade is zipped out of the world instantly, the EXPLOSION sound that it makes for
	// the AI is also immediately destroyed. For this reason, we now make the grenade entity inert and
	// throw it away in 1/10th of a second instead of right away. Removing the grenade instantly causes
	// intermittent bugs with env_microphones who are listening for explosions. They will 'randomly' not
	// hear explosion sounds when the grenade is removed and the SoundEnt thinks (and removes the sound)
	// before the env_microphone thinks and hears the sound.
	SetNextThink(gpGlobals->curtime + 0.1);
#else
	SetNextThink(gpGlobals->curtime);
#endif//HL2_EPISODIC
}

// ----------------------------------------------
// This spawns the flashbang/concussion grenades.
// ----------------------------------------------
CBaseGrenade* Flashgrenade_Create(const Vector& position, const QAngle& angles, const Vector& velocity, const AngularImpulse& angVelocity, CBaseEntity* pOwner, float timer, bool combineSpawned)
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeFlash* pGrenade = (CGrenadeFlash*)CBaseEntity::Create("npc_grenade_flash", position, angles, pOwner);

	pGrenade->SetTimer(timer, timer - 1.5f); // FRAG_GRENADE_WARN_TIME = 1.5f
	pGrenade->SetVelocity(velocity, angVelocity);
	pGrenade->SetThrower(ToBaseCombatCharacter(pOwner));
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->SetCombineSpawned(combineSpawned);

	return pGrenade;
}