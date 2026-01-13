#ifndef C_SKYDOME_H
#define C_SKYDOME_H

#include "c_baseentity.h"


class C_SkyDome : public C_BaseEntity
{
	DECLARE_CLASS(C_SkyDome, C_BaseEntity);
public:
	DECLARE_CLIENTCLASS();

	C_SkyDome();
	virtual ~C_SkyDome();

	void Spawn();

	void OnDataChanged(DataUpdateType_t updateType);


	bool IsDynamicSkyEnabled() const;

	virtual void ClientThink();


	Vector m_vCurrentSunPos;
	Vector m_vCurrentWindSpeed;
	float m_flCurrentThickness;
	float  m_flCurrentCoverage;

	CNetworkVector(m_vDesiredSunPos);
	CNetworkVector(m_vDesiredWindSpeed);
	CNetworkVar(float, m_flDesiredThickness);
	CNetworkVar(float, m_flDesiredCoverage);


	virtual void FireEvent(const Vector& origin, const QAngle& angles, int event, const char *options) {}
private:
	bool m_bEnableDynamicSky;
};

extern C_SkyDome* g_pSkyDome;

#endif