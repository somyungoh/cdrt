#pragma once

#include "common.h"
#include "hittable.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

class CRay;

//----------------------------------------------------

class ILight
{
public:
    inline virtual float        GetIntensityFromRay(const CRay &ray) const = 0;
    inline virtual glm::vec3    Origin() const = 0;

public:
    glm::vec3   m_color;
};

//----------------------------------------------------

class CPointLight : public ILight
{
public:
    CPointLight(const glm::vec3 &origin, const glm::vec3 &color);

    inline virtual float        GetIntensityFromRay(const CRay &ray) const override;
    inline virtual glm::vec3    Origin() const override { return m_origin; };

public:
    glm::vec3   m_origin;
};

//----------------------------------------------------

class CAreaLight : public ILight, public CHittablePlane
{
public:
    CAreaLight();
    CAreaLight(const glm::vec3 &origin, const glm::vec3 &normal, const glm::vec3 &up, float sx, float sy, const glm::vec3 &color);

    inline virtual float        GetIntensityFromRay(const CRay &ray) const override;
    inline virtual glm::vec3    Origin() const override { return m_origin; };

public:
    glm::vec3   m_vx, m_vy, m_vz;
    float       m_sx, m_sy;
};

//----------------------------------------------------
_CD_NAMESPACE_END
