#include "light.h"
#include "material.h"
#include "ray.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

CPointLight::CPointLight(const glm::vec3 &origin, const glm::vec3 &color)
{
    m_origin = origin;
    m_color = color;
}

//----------------------------------------------------

float   CPointLight::GetIntensityFromRay(const CRay &ray) const
{
    (void)ray;
    return m_color.x;
}

//----------------------------------------------------

CAreaLight::CAreaLight(const glm::vec3 &origin, const glm::vec3 &normal, const glm::vec3 &up, float sx, float sy, const glm::vec3 &color)
: CHittablePlane(origin, normal, up, sx, sy, std::make_shared<CMaterialLambertian>(color))
{
    m_color = color;
}

//----------------------------------------------------

float   CAreaLight::GetIntensityFromRay(const CRay &ray) const
{
    (void)ray;
    return m_color.x;
}

//----------------------------------------------------
_CD_NAMESPACE_END
