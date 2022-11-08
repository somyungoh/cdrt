#pragma once

#include "common.h"
#include "ray.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

struct SHitRec;

class IMaterial
{
public:
    virtual bool        Scatter(const CRay &ray, const SHitRec &hitRec, glm::vec3 &attenuation, CRay &scattered) const = 0;
    virtual glm::vec3   Albedo() const = 0;
};

//----------------------------------------------------

class CMaterialLambertian : public IMaterial
{
public:
    CMaterialLambertian(const glm::vec3 &color);

    virtual bool        Scatter(const CRay &ray, const SHitRec &hitRec, glm::vec3 &attenuation, CRay &scattered) const override;
    virtual glm::vec3   Albedo() const override { return m_albedo; } ;

public:
    glm::vec3   m_albedo;
};

//----------------------------------------------------

class CMaterialMetal : public IMaterial
{
public:
    CMaterialMetal(const glm::vec3 &color, float glossiness);

    virtual bool        Scatter(const CRay &ray, const SHitRec &hitRec, glm::vec3 &attenuation, CRay &scattered) const override;
    virtual glm::vec3   Albedo() const override { return m_albedo; };
public:
    glm::vec3   m_albedo;
    float       m_glossiness;
};

//----------------------------------------------------

class CMaterialGlass : public IMaterial
{
public:
    CMaterialGlass(float refrativeIndex, float glossiness);

    virtual bool        Scatter(const CRay &ray, const SHitRec &hitRec, glm::vec3 &attenuation, CRay &scattered) const override;
    virtual glm::vec3   Albedo() const override { return glm::vec3(0); };

public:
    float   m_refractiveIndex;
    float   m_glossiness;

private:
    float   _Reflectance(float cos, float refIdx) const;
};

//----------------------------------------------------
_CD_NAMESPACE_END
