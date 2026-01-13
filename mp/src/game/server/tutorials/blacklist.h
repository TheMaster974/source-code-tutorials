// ---------------------------------------------------------------------------------------
// Code written by Gareth Pugh, intended for usage with SLAM Tripmines.
// ---------------------------------------------------------------------------------------
// The following code creates an array of strings and checks them against an input entity.
// ---------------------------------------------------------------------------------------

// ---------------------------------
// Mandatory includes for this file.
// ---------------------------------
#include "cbase.h"
#include "tier0/memdbgon.h"

// --------------------------------------------------------------------------
// Blacklist of entities the Tripmine should NOT be parented to!
// Basically, these are entities that can 'despawn' or cause Tripmine errors.
// Expand/contract as you see fit!
// --------------------------------------------------------------------------
const char* blacklist[] = {
	"worldspawn",
	"func_breakable",
	"func_breakable_surf",
	"prop_physics",
	"prop_physics_respawnable",
	"prop_ragdoll",
	"func_physbox",
	"grenade_hopwire",
	"grenade_satchel",
	"grenade_tripmine",
	"item_ammo_357",
	"item_ammo_357_large",
	"item_ammo_ar2",
	"item_ammo_ar2_altfire",
	"item_ammo_ar2_large",
	"item_ammo_crossbow",
	"item_ammo_pistol",
	"item_ammo_pistol_large",
	"item_ammo_physgun_pellet",
	"item_ammo_smg1",
	"item_ammo_smg1_grenade",
	"item_ammo_smg1_large",
	"item_battery",
	"item_box_buckshot",
	"item_box_flare_rounds",
	"item_box_sniper_rounds",
	"item_dynamic_resupply",
	"item_healthkit",
	"item_healthvial",
	"item_item_crate",
	"item_rpg_round",
	"physics_cannister",
	"weapon_357",
	"weapon_alyxgun",
	"weapon_annabelle",
	"weapon_ar2",
	"weapon_bugbait",
	"weapon_cguard",
	"weapon_crossbow",
	"weapon_crowbar",
	"weapon_extinguisher",
	"weapon_flaregun",
	"weapon_frag",
	"weapon_hopwire",
	"weapon_immolator",
	"weapon_iontau",
	"weapon_molotov",
	"weapon_physcannon",
	"weapon_physgun",
	"weapon_pistol",
	"weapon_rpg",
	"weapon_shotgun",
	"weapon_slam",
	"weapon_smg1",
	"weapon_smg2",
	"weapon_sniperrifle",
	"weapon_stunstick"
};

// ------------------------------------------------
// Obtain the size of the array quickly and easily!
// ------------------------------------------------
//const int blacklistSize = sizeof(blacklist) / sizeof(blacklist[0]);
const int blacklistSize = ARRAYSIZE(blacklist);

// --------------------------------------------------------------------------------------
// Check to see if the object the Tripmine is trying to attach to matches the entities in
// the blacklist. Returns true if the entity classname matches an entry in the blacklist.
// --------------------------------------------------------------------------------------
bool TripmineBlacklistCheck(CBaseEntity* pInputEnt)
{
	const char* inputString = pInputEnt->GetClassname();
	for (int i = 0; i < blacklistSize; i++)
	{
//		Msg("Input: %s vs. Compare: %s\n", inputString, blacklist[i]);
		
		if (Q_strcmp(inputString, blacklist[i]) == 0)
		{
			// If we have a match, then don't parent the Tripmine!
			// Msg("We have a match!\n");
			return true;
		}
	}
	// Otherwise, we can parent the Tripmine!
	return false;
}