#pragma once

#include "common.h"
#include "ray.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

struct SHitRec;

class CMaterial
{
public:
    virtual bool    Scatter(const CRay &ray, const SHitRec &hitRec, glm::vec3 &attenuation, CRay &scattered) const = 0;
};

//----------------------------------------------------

class CMaterialLambertian : public CMaterial
{
public:
    CMaterialLambertian(const glm::vec3 &color);

    virtual bool    Scatter(const CRay &ray, const SHitRec &hitRec, glm::vec3 &attenuation, CRay &scattered) const override;

public:
    glm::vec3   m_albedo;
};

//----------------------------------------------------

class CMaterialMetal : public CMaterial
{
public:
    CMaterialMetal(const glm::vec3 &color, float glossiness);

    virtual bool    Scatter(const CRay &ray, const SHitRec &hitRec, glm::vec3 &attenuation, CRay &scattered) const override;

public:
    glm::vec3   m_albedo;
    float       m_glossiness;
};

//----------------------------------------------------

class CMaterialGlass : public CMaterial
{
public:
    CMaterialGlass(float refrativeIndex, float glossiness);

    virtual bool    Scatter(const CRay &ray, const SHitRec &hitRec, glm::vec3 &attenuation, CRay &scattered) const override;

public:
    float   m_refractiveIndex;
    float   m_glossiness;

private:
    float   _Reflectance(float cos, float refIdx) const;
};

//----------------------------------------------------
_CD_NAMESPACE_END
