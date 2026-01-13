// ----------------------------------------------------------------------------
// Code written by Gareth Pugh, adds a Client-side version of the battery item.
// ----------------------------------------------------------------------------
// This creates a dynamic light at the battery's position.
// ----------------------------------------------------------------------------

#include "cbase.h"			// Mandatory include.
#include "dlight.h"			// These two includes are needed for dynamic lights.
#include "r_efx.h"
#include "tier0/memdbgon.h" // Last file to be included.

// --------------------------------------------------------------------------------------
// Creates a Client-side version of the battery item. The BaseClass is C_BaseAnimating,
// as C_Item doesn't exist on the Client. If you look into this, the BaseClass for CItem
// on the Server is CBaseAnimating, so we can use C_BaseAnimating without causing issues.
// --------------------------------------------------------------------------------------
class C_ItemBattery : public C_BaseAnimating
{
public:
	DECLARE_CLASS(C_ItemBattery, C_BaseAnimating);
	DECLARE_CLIENTCLASS();

	void OnDataChanged(DataUpdateType_t updateType); // Called whenever data is changed.
	void ClientThink(void); // Think function for the Client.
};

// ----------------------------
// Add a Clientclass like this.
// ----------------------------
IMPLEMENT_CLIENTCLASS_DT(C_ItemBattery, DT_ItemBattery, CItemBattery)
END_RECV_TABLE()

void C_ItemBattery::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	SetNextClientThink(CLIENT_THINK_ALWAYS);
}

void C_ItemBattery::ClientThink(void)
{
	dlight_t* dl = effects->CL_AllocDlight(index); // Create a dynamic light.
	dl->origin = GetAbsOrigin(); // Position, replaces batteryPos.
	dl->color.r = 16; // Red.
	dl->color.g = 252; // Green.
	dl->color.b = 248; // Blue.
	dl->color.exponent = -1; // Brightness.
	dl->die = gpGlobals->curtime + 0.05f; // Stop after this time.
	dl->radius = random->RandomFloat(145.0f, 156.0f); // How large is the dynamic light?
	dl->decay = 512.0f; // Drop this much each second.

//	return BaseClass::ClientThink(); // Does nothing.
}