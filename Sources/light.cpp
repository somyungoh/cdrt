#include "light.h"


_CD_NAMESPACE_BEGIN
//----------------------------------------------------

CAreaLight::CAreaLight(const glm::vec3 &origin, const glm::vec3 &normal, const glm::vec3 &up, float sx, float sy, const std::shared_ptr<CMaterial> &material)
: CHittablePlane(origin, normal, up, sx, sy, material)
, m_color(glm::vec3(1.f))
{
}

//----------------------------------------------------
_CD_NAMESPACE_END
