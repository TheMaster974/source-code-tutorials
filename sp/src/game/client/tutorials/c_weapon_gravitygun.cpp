//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Client-side version of the Physgun, restores beam drawing.
// 
// Note: Make sure to have models/weapons/glueblob.mdl in your mod,
//       plus weapon_physgun.txt in the scripts folder!
//
// $NoKeywords: $FixedByTheMaster974
//===========================================================================//

#include "cbase.h"
#include "hud.h"
#include "in_buttons.h"
#include "beamdraw.h"
#include "c_weapon__stubs.h"
#include "clienteffectprecachesystem.h"

// ----------
// Additions.
// ----------
#include "c_basehlcombatweapon.h"
#include "dlight.h"
#include "r_efx.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectGravityGun )
CLIENTEFFECT_MATERIAL( "sprites/physbeam" )
CLIENTEFFECT_REGISTER_END()

class C_BeamQuadratic : public CDefaultClientRenderable
{
public:
	C_BeamQuadratic();
	void			Update( C_BaseEntity *pOwner );

	// IClientRenderable
	virtual const Vector&			GetRenderOrigin( void ) { return m_targetPosition; } // return m_worldPosition;
	virtual const QAngle&			GetRenderAngles( void ) { return vec3_angle; }
	virtual bool					ShouldDraw( void ) { return true; }
	virtual bool					IsTransparent( void ) { return true; }
	virtual bool					ShouldReceiveProjectedTextures( int flags ) { return false; }
	virtual int						DrawModel( int flags );

	// Addition, this is needed for the beam drawing to work properly.
	// Improved to match how this function works in other files.
	const matrix3x4_t& RenderableToWorldTransform()
	{
		static matrix3x4_t mat;
		SetIdentityMatrix(mat);
		PositionMatrix(GetRenderOrigin(), mat);
		return mat;
	}

	// Returns the bounds relative to the origin (render bounds)
	virtual void	GetRenderBounds( Vector& mins, Vector& maxs )
	{
		// bogus.  But it should draw if you can see the end point
		mins.Init(-32,-32,-32);
		maxs.Init(32,32,32);
	}

	C_BaseEntity			*m_pOwner;
	Vector					m_targetPosition;
	Vector					m_worldPosition;
	int						m_active;
	int						m_glueTouching;
	int						m_viewModelIndex;
};


class C_WeaponGravityGun : public C_BaseHLCombatWeapon // Formerly C_BaseCombatWeapon.
{
	DECLARE_CLASS( C_WeaponGravityGun, C_BaseHLCombatWeapon ); // Formerly C_BaseCombatWeapon.
public:
	C_WeaponGravityGun() {}

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	int KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
	{
		if ( gHUD.m_iKeyBits & IN_ATTACK )
		{
			switch ( keynum )
			{
			case MOUSE_WHEEL_UP:
				gHUD.m_iKeyBits |= IN_WEAPON1;
				return 0;

			case MOUSE_WHEEL_DOWN:
				gHUD.m_iKeyBits |= IN_WEAPON2;
				return 0;
			}
		}

		// Allow engine to process
		return BaseClass::KeyInput( down, keynum, pszCurrentBinding );
	}

	void OnDataChanged( DataUpdateType_t updateType )
	{
		BaseClass::OnDataChanged( updateType );
		m_beam.Update( this );
	}

private:
	C_WeaponGravityGun( const C_WeaponGravityGun & );

	C_BeamQuadratic	m_beam;
};

STUB_WEAPON_CLASS_IMPLEMENT( weapon_physgun, C_WeaponGravityGun );

IMPLEMENT_CLIENTCLASS_DT( C_WeaponGravityGun, DT_WeaponGravityGun, CWeaponGravityGun )
	RecvPropVector( RECVINFO_NAME(m_beam.m_targetPosition,m_targetPosition) ),
	RecvPropVector( RECVINFO_NAME(m_beam.m_worldPosition, m_worldPosition) ),
	RecvPropInt( RECVINFO_NAME(m_beam.m_active, m_active) ),
	RecvPropInt( RECVINFO_NAME(m_beam.m_glueTouching, m_glueTouching) ),
	RecvPropInt( RECVINFO_NAME(m_beam.m_viewModelIndex, m_viewModelIndex) ),
END_RECV_TABLE()


C_BeamQuadratic::C_BeamQuadratic()
{
	m_pOwner = NULL;
	m_hRenderHandle = INVALID_CLIENT_RENDER_HANDLE; // Addition.
}

void C_BeamQuadratic::Update( C_BaseEntity *pOwner )
{
	m_pOwner = pOwner;
	if ( m_active )
	{
		if ( m_hRenderHandle == INVALID_CLIENT_RENDER_HANDLE )
		{
			ClientLeafSystem()->AddRenderable( this, RENDER_GROUP_TRANSLUCENT_ENTITY );
		}
		else
		{
			ClientLeafSystem()->RenderableChanged( m_hRenderHandle );
		}
	}
	else if ( !m_active && m_hRenderHandle != INVALID_CLIENT_RENDER_HANDLE )
	{
		ClientLeafSystem()->RemoveRenderable( m_hRenderHandle );
		m_hRenderHandle = INVALID_CLIENT_RENDER_HANDLE; // Addition.
	}
}


int	C_BeamQuadratic::DrawModel( int )
{
	Vector points[3];
	QAngle tmpAngle;

	if ( !m_active )
		return 0;

	C_BaseEntity *pEnt = cl_entitylist->GetEnt( m_viewModelIndex );
	if ( !pEnt )
		return 0;
	pEnt->GetAttachment( 1, points[0], tmpAngle );

	points[1] = 0.5 * (m_targetPosition + points[0]);
	
	// a little noise 11t & 13t should be somewhat non-periodic looking
	//points[1].z += 4*sin( gpGlobals->curtime*11 ) + 5*cos( gpGlobals->curtime*13 );
	points[2] = m_worldPosition;

	IMaterial *pMat = materials->FindMaterial( "sprites/physbeam", TEXTURE_GROUP_CLIENT_EFFECTS );
	Vector color;
	if ( m_glueTouching )
	{
		color.Init(1,0,0);
	}
	else
	{
		color.Init(1,1,1);
	}

	float scrollOffset = gpGlobals->curtime - (int)gpGlobals->curtime;

	// This section has been changed so beam drawing can work properly.
	// materials->Bind( pMat );
	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind( pMat );
	DrawBeamQuadratic( points[0], points[1], points[2], 13, color, scrollOffset );
	
// ------------------------------------------------------------------------------
// Create dynamic lights at the start, middle and end points of the Physgun beam.
// ------------------------------------------------------------------------------
	dlight_t* dl[3]; // Define an array of dynamic lights.
	for (int i = 0; i < 3; i++)
	{
		dl[i] = effects->CL_AllocDlight(m_viewModelIndex + i); // Create 3 different dynamic lights.
		dl[i]->origin = points[i]; // Positions.
		dl[i]->color.r = 159; // Red.
		dl[i]->color.g = 255; // Green.
		dl[i]->color.b = 255; // Blue.
		dl[i]->color.exponent = -2; // Brightness.
		dl[i]->die = gpGlobals->curtime + 0.05f; // Stop after this time.
		dl[i]->radius = random->RandomFloat(245.0f / (i + 1), 256.0f / (i + 1)); // How large is the dynamic light? Decrease in size over the beam's length.
		dl[i]->decay = 512.0f; // Drop this much each second.
		dl[i]->style = 1; // Light style.
	}
	return 1;
}

