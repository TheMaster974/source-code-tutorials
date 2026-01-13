//===================== Copyright (c) Valve Corporation. All Rights Reserved. ======================
//
// Example shader that can be applied to models
//
//==================================================================================================

#include "BaseVSShader.h"
#include "convar.h"
#include "skydome_atmosphere_helper.h"

BEGIN_VS_SHADER(SKYDOME_ATMOSPHERE, "Help for Example Model Shader")

BEGIN_SHADER_PARAMS
SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "")
SHADER_PARAM(LUT, SHADER_PARAM_TYPE_TEXTURE, "0.0", "")
END_SHADER_PARAMS

void SetupVars(Skydome_Vars_t& info)
{
	info.m_nLUTTexture = LUT;
}

SHADER_INIT_PARAMS()
{
	Skydome_Vars_t info;
	SetupVars(info);
	InitParamsSkydome(this, params, pMaterialName, info);
}

SHADER_FALLBACK
{
	return 0;
}

SHADER_INIT
{
	Skydome_Vars_t info;
	SetupVars(info);
	InitSkydome(this, params, info);
}

SHADER_DRAW
{
	Skydome_Vars_t info;
	SetupVars(info);
	DrawSkydome(this, params, pShaderAPI, pShaderShadow, info, vertexCompression, pContextDataPtr);
}

END_SHADER

