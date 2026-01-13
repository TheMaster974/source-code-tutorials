//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "BaseVSShader.h"
#include "skydome_atmosphere_helper.h"
#include "convar.h"
#include "cpp_shader_constant_register_map.h"
#include "skydome_vs30.inc"
#include "skydome_ps30.inc"
#include "commandbuilder.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar mat_fullbright("mat_fullbright", "0", FCVAR_CHEAT);

ConVar cl_sky_thickness("cl_sky_thickness", "3");
ConVar cl_sky_coverage("cl_sky_coverage", "0.50");
ConVar cl_sky_SunPos("cl_sky_sunpos", "1 1 1 5");
ConVar cl_sky_windspeed("cl_sky_windspeed", "0 0 0 5");
ConVar cl_sky_render("cl_sky_render", "1");

static void UTIL_StringToFloatArray(float *pVector, int count, const char *pString)
{
	char *pstr, *pfront, tempString[128];
	int	j;

	Q_strncpy(tempString, pString, sizeof(tempString));
	pstr = pfront = tempString;

	for (j = 0; j < count; j++)			// lifted from pr_edict.c
	{
		pVector[j] = atof(pfront);

		// skip any leading whitespace
		while (*pstr && *pstr <= ' ')
			pstr++;

		// skip to next whitespace
		while (*pstr && *pstr > ' ')
			pstr++;

		if (!*pstr)
			break;

		pstr++;
		pfront = pstr;
	}
	for (j++; j < count; j++)
	{
		pVector[j] = 0;
	}
}

// Textures may be bound to the following samplers:
//	SHADER_SAMPLER0	 Base (Albedo) / Gloss in alpha
//	SHADER_SAMPLER4	 Flashlight Shadow Depth Map
//	SHADER_SAMPLER5	 Normalization cube map
//	SHADER_SAMPLER6	 Flashlight Cookie


//-----------------------------------------------------------------------------
// Initialize shader parameters
//-----------------------------------------------------------------------------
void InitParamsSkydome(CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, Skydome_Vars_t &info)
{
	// FLASHLIGHTFIXME: Do ShaderAPI::BindFlashlightTexture
	Assert(info.m_nFlashlightTexture >= 0);

	if (g_pHardwareConfig->SupportsBorderColor())
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
	}

	// This shader can be used with hw skinning
	SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
	SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);
}

//-----------------------------------------------------------------------------
// Initialize shader
//-----------------------------------------------------------------------------
void InitSkydome(CBaseVSShader *pShader, IMaterialVar** params, Skydome_Vars_t &info)
{
	Assert(info.m_nFlashlightTexture >= 0);
	pShader->LoadTexture(info.m_nFlashlightTexture, TEXTUREFLAGS_SRGB);

	if (params[info.m_nLUTTexture]->IsDefined())
	{
		pShader->LoadTexture(info.m_nLUTTexture);
	}
}

class CSkydome_Context : public CBasePerMaterialContextData
{
public:
	CCommandBufferBuilder< CFixedCommandStorageBuffer< 800 > > m_SemiStaticCmdsOut;
	bool m_bFastPath;

};

//-----------------------------------------------------------------------------
// Draws the shader
//-----------------------------------------------------------------------------
void DrawSkydome_Internal(CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow,
	bool bHasFlashlight, Skydome_Vars_t &info, VertexCompressionType_t vertexCompression,
	CBasePerMaterialContextData **pContextDataPtr)
{
	CSkydome_Context *pContextData = reinterpret_cast<CSkydome_Context *> (*pContextDataPtr);
	if (!pContextData)
	{
		pContextData = new CSkydome_Context;
		*pContextDataPtr = pContextData;
	}

	if (pShader->IsSnapshotting())
	{

		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
		int userDataSize = 0;

		// Always enable...will bind white if nothing specified...
		pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);		// Base (albedo) map

		// Always enable, since flat normal will be bound
		pShaderShadow->EnableTexture(SHADER_SAMPLER3, true);		// Normal map
		userDataSize = 4; // tangent S
		pShaderShadow->EnableTexture(SHADER_SAMPLER5, true);		// Normalizing cube map
		pShaderShadow->EnableSRGBWrite(true);

		// texcoord0 : base texcoord, texcoord2 : decal hw morph delta
		int pTexCoordDim[3] = { 2, 0, 3 };
		int nTexCoordCount = 1;

		// This shader supports compressed vertices, so OR in that flag:
		flags |= VERTEX_FORMAT_COMPRESSED;

		pShaderShadow->VertexShaderVertexFormat(flags, nTexCoordCount, pTexCoordDim, userDataSize);

		DECLARE_STATIC_VERTEX_SHADER(skydome_vs30);
		SET_STATIC_VERTEX_SHADER(skydome_vs30);

		// Assume we're only going to get in here if we support 2b
		DECLARE_STATIC_PIXEL_SHADER(skydome_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(CONVERT_TO_SRGB, 0);
		SET_STATIC_PIXEL_SHADER(skydome_ps30);

		if (bHasFlashlight)
		{
			pShader->FogToBlack();
		}
		else
		{
			pShader->DefaultFog();
		}

		// HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
		pShaderShadow->EnableAlphaWrites(true);
	}
	else // not snapshotting -- begin dynamic state
	{

		pShader->BindTexture(SHADER_SAMPLER0, info.m_nLUTTexture);

		LightState_t lightState = { 0, false, false };
		bool bFlashlightShadows = false;
		if (bHasFlashlight)
		{
			Assert(info.m_nFlashlightTexture >= 0 && info.m_nFlashlightTextureFrame >= 0);
			pShader->BindTexture(SHADER_SAMPLER6, info.m_nFlashlightTexture, info.m_nFlashlightTextureFrame);
			VMatrix worldToTexture;
			ITexture *pFlashlightDepthTexture;
			FlashlightState_t state = pShaderAPI->GetFlashlightStateEx(worldToTexture, &pFlashlightDepthTexture);
			bFlashlightShadows = state.m_bEnableShadows && (pFlashlightDepthTexture != NULL);

			SetFlashLightColorFromState(state, pShaderAPI, PSREG_FLASHLIGHT_COLOR);

			if (pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && state.m_bEnableShadows)
			{
				pShader->BindTexture(SHADER_SAMPLER4, pFlashlightDepthTexture, 0);
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER5, TEXTURE_SHADOW_NOISE_2D);
			}
		}
		else // no flashlight
		{
			pShaderAPI->GetDX9LightState(&lightState);
		}

		MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
		int fogIndex = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z) ? 1 : 0;
		int numBones = pShaderAPI->GetCurrentNumBones();

		DECLARE_DYNAMIC_VERTEX_SHADER(skydome_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, numBones > 0);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(LIGHTING_PREVIEW, pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING) != 0);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
		SET_DYNAMIC_VERTEX_SHADER(skydome_vs30);


		bool bDrawSky = cl_sky_render.GetBool();

		DECLARE_DYNAMIC_PIXEL_SHADER(skydome_ps30);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(RENDER_SKY, bDrawSky);
		SET_DYNAMIC_PIXEL_SHADER(skydome_ps30);

		pShader->SetModulationPixelShaderDynamicState_LinearColorSpace(1);

		if (!bHasFlashlight)
		{
			pShaderAPI->BindStandardTexture(SHADER_SAMPLER5, TEXTURE_NORMALIZATION_CUBEMAP_SIGNED);
		}

		float flthickness[4];
		flthickness[0] = cl_sky_thickness.GetFloat();
		flthickness[1] = flthickness[2] = flthickness[3] = flthickness[0];
		pShaderAPI->SetPixelShaderConstant(PSREG_CONSTANT_04, flthickness);

		float flcoverage[4];
		flcoverage[0] = cl_sky_coverage.GetFloat();
		flcoverage[1] = flcoverage[2] = flcoverage[3] = flcoverage[0];
		pShaderAPI->SetPixelShaderConstant(PSREG_CONSTANT_05, flcoverage);

		float flSunPos[4];
		UTIL_StringToFloatArray(flSunPos, 4, cl_sky_SunPos.GetString());
		pShaderAPI->SetPixelShaderConstant(PSREG_CONSTANT_06, flSunPos);

		float flwindspeed[4];
		UTIL_StringToFloatArray(flwindspeed, 4, cl_sky_windspeed.GetString());
		pShaderAPI->SetPixelShaderConstant(PSREG_CONSTANT_07, flwindspeed);

		float flTime[4];
		flTime[0] = pShaderAPI->CurrentTime();
		flTime[1] = flTime[2] = flTime[3] = flTime[0];
		pShaderAPI->SetPixelShaderConstant(PSREG_CONSTANT_08, flTime);
	}
	pShader->Draw();
}


//-----------------------------------------------------------------------------
// Draws the shader
//-----------------------------------------------------------------------------
void DrawSkydome(CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow,
	Skydome_Vars_t &info, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr)

{
	bool bHasFlashlight = pShader->UsingFlashlight(params);
	if (bHasFlashlight)
	{
		DrawSkydome_Internal(pShader, params, pShaderAPI, pShaderShadow, false, info, vertexCompression, pContextDataPtr++);
		if (pShaderShadow)
		{
			pShader->SetInitialShadowState();
		}
	}
	DrawSkydome_Internal(pShader, params, pShaderAPI, pShaderShadow, bHasFlashlight, info, vertexCompression, pContextDataPtr);
}
