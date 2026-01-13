#include "cbase.h"

#include "tier0/memdbgon.h"


//------------------------------------------------------------------------------
// Purpose : Skydome control entity
//------------------------------------------------------------------------------
class CSkyDome : public CBaseEntity
{
public:
	DECLARE_CLASS(CSkyDome, CBaseEntity);

	CSkyDome() {}

	
	void Spawn(void);


	virtual bool KeyValue(const char *szKeyName, const char *szValue);

	virtual int UpdateTransmitState()
	{
		return SetTransmitState(FL_EDICT_ALWAYS);
	}

	virtual int ObjectCaps()
	{
		return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;
	}


	// Inputs
	void	InputSetEnableDynamicSky(inputdata_t& inputdata);
	void	InputSetSunPos(inputdata_t& inputdata);
	void	InputSetWindSpeed(inputdata_t& inputdata);
	void	InputSetThickness(inputdata_t& inputdata);
	void	InputSetCoverage(inputdata_t& inputdata);

	DECLARE_SERVERCLASS()
	DECLARE_DATADESC();
private:

	CNetworkVar(bool, m_bEnableDynamicSky);
	CNetworkVector(m_vDesiredSunPos);
	CNetworkVector(m_vDesiredWindSpeed);
	CNetworkVar(float, m_flDesiredThickness);
	CNetworkVar(float, m_flDesiredCoverage);
};

LINK_ENTITY_TO_CLASS(env_skydome, CSkyDome);

//-----------------------------------------------------------------------------
// Save/load 
//-----------------------------------------------------------------------------
BEGIN_DATADESC(CSkyDome)

DEFINE_FIELD(m_bEnableDynamicSky, FIELD_BOOLEAN),
DEFINE_FIELD(m_vDesiredSunPos, FIELD_VECTOR),
DEFINE_FIELD(m_vDesiredWindSpeed, FIELD_VECTOR),
DEFINE_FIELD(m_flDesiredThickness, FIELD_FLOAT),
DEFINE_FIELD(m_flDesiredCoverage, FIELD_FLOAT),

DEFINE_INPUTFUNC(FIELD_BOOLEAN, "EnableDynamicSky", InputSetEnableDynamicSky),
DEFINE_INPUTFUNC(FIELD_VECTOR, "SetSunPos", InputSetSunPos),
DEFINE_INPUTFUNC(FIELD_VECTOR, "SetWindSpeed", InputSetWindSpeed),
DEFINE_INPUTFUNC(FIELD_FLOAT, "InputSetThickness", InputSetThickness),
DEFINE_INPUTFUNC(FIELD_FLOAT, "InputSetCoverage", InputSetCoverage),
END_DATADESC();

IMPLEMENT_SERVERCLASS_ST(CSkyDome, DT_SkyDome)
SendPropBool(SENDINFO(m_bEnableDynamicSky)),
SendPropVector(SENDINFO(m_vDesiredSunPos)),
SendPropVector(SENDINFO(m_vDesiredWindSpeed)),
SendPropFloat(SENDINFO(m_flDesiredThickness)),
SendPropFloat(SENDINFO(m_flDesiredCoverage)),
END_SEND_TABLE()

void CSkyDome::Spawn(void)
{
	
	SetSolid(SOLID_NONE);

	BaseClass::Spawn();

	m_bEnableDynamicSky = true;
}

bool CSkyDome::KeyValue(const char *szKeyName, const char *szValue)
{

	if (FStrEq(szKeyName, "thickness"))
	{
		m_flDesiredThickness = atof(szValue);
		if(cvar->FindVar("cl_sky_thickness"))
			cvar->FindVar("cl_sky_thickness")->SetValue(szValue);
		return true;
	}
	
	if (FStrEq(szKeyName, "coverage"))
	{
		m_flDesiredCoverage = atof(szValue);
		if (cvar->FindVar("cl_sky_coverage"))
			cvar->FindVar("cl_sky_coverage")->SetValue(szValue);
		return true;
	}

	if (FStrEq(szKeyName, "sunpos"))
	{
		Vector Pos;
		UTIL_StringToVector(Pos.Base(), szValue);
		m_vDesiredSunPos = Pos;

		if (cvar->FindVar("cl_sky_SunPos"))
			cvar->FindVar("cl_sky_SunPos")->SetValue(szValue);
		return true;
	}
	if (FStrEq(szKeyName, "windspeed"))
	{
		Vector Pos;
		UTIL_StringToVector(Pos.Base(), szValue);
		m_vDesiredWindSpeed = Pos;

		if (cvar->FindVar("cl_sky_windspeed"))
			cvar->FindVar("cl_sky_windspeed")->SetValue(szValue);
		return true;
	}

	return true;
}

void CSkyDome::InputSetEnableDynamicSky(inputdata_t& inputdata)
{
	m_bEnableDynamicSky = inputdata.value.Bool();
}


//------------------------------------------------------------------------------
// Input values
//------------------------------------------------------------------------------
void CSkyDome::InputSetSunPos(inputdata_t& inputdata)
{
	const char* pPos = inputdata.value.String();

	Vector Pos;
	UTIL_StringToVector(Pos.Base(), pPos);
	m_vDesiredSunPos = Pos;
}

//------------------------------------------------------------------------------
// Input values
//------------------------------------------------------------------------------
void CSkyDome::InputSetWindSpeed(inputdata_t& inputdata)
{
	const char* pPos = inputdata.value.String();

	Vector Pos;
	UTIL_StringToVector(Pos.Base(), pPos);
	m_vDesiredWindSpeed = Pos;
}

//------------------------------------------------------------------------------
// Input values
//------------------------------------------------------------------------------
void CSkyDome::InputSetThickness(inputdata_t& inputdata)
{
	m_flDesiredThickness = inputdata.value.Float();
}

//------------------------------------------------------------------------------
// Input values
//------------------------------------------------------------------------------
void CSkyDome::InputSetCoverage(inputdata_t& inputdata)
{
	m_flDesiredCoverage = inputdata.value.Float();
}
