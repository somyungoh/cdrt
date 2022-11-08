#pragma once

#include "hittable.h"


_CD_NAMESPACE_BEGIN
//----------------------------------------------------

class CAreaLight : public CHittablePlane
{
public:
    CAreaLight(const glm::vec3 &origin, const glm::vec3 &normal, const glm::vec3 &up, float sx, float sy, const std::shared_ptr<CMaterial> &material);

public:
    glm::vec3   m_color;
};

//----------------------------------------------------
_CD_NAMESPACE_END
