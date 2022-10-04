#pragma once

#include "common.h"
#include "ray.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

class CCamera
{
public:
    CCamera() {};
    CCamera(float vFov, float aspectRatio)
    : m_vFov(vFov)
    , m_aspectRatio(aspectRatio)
    {
        // default coordinates
        m_origin = glm::vec3(0, 0, 0);
        m_up = glm::vec3(0, 1, 0);
        m_forward = glm::vec3(0, 0, -1);
        m_right = glm::vec3(glm::normalize(glm::cross(m_forward, m_up)));

        // viewport
        float   h = glm::tan(glm::radians(vFov) / 2.f);
        m_Sy = 2.0f * h;
        m_Sx = aspectRatio * m_Sy;
        m_lowerLeftCorner = m_origin - m_right * m_Sx * 0.5f - m_up * m_Sy * 0.5f - m_forward * 1.0f;   // focal dist 1.0
    }

    inline void     SetPos(const glm::vec3 &pos)
    {
        m_origin = pos;
        m_lowerLeftCorner = m_origin - m_right * m_Sx * 0.5f - m_up * m_Sy * 0.5f - m_forward * 1.0f;
    }

    inline void     LookAt(const glm::vec3 &lookAt)
    {
        m_forward = glm::normalize(m_origin - lookAt);
        m_up = glm::normalize(glm::cross(m_right, m_forward));
        m_right = glm::normalize(glm::cross(m_forward, m_up));
        m_lowerLeftCorner = m_origin - m_right * m_Sx * 0.5f - m_up * m_Sy * 0.5f - m_forward * 1.0f;
    }

    inline const CRay   GetRay(float u, float v) const
    {
        // TODO: check u,v out of range?
        return CRay(m_origin, m_lowerLeftCorner + u * m_right * m_Sx + v * m_up * m_Sy - m_origin);
    }

public:
    float       m_vFov;
    float       m_aspectRatio;
    glm::vec3   m_lowerLeftCorner;

    float       m_Sx;       // size of the pane
    float       m_Sy;

    glm::vec3   m_origin;
    glm::vec3   m_up;
    glm::vec3   m_forward;  // opposite direction from lookAt
    glm::vec3   m_right;
};

//----------------------------------------------------
_CD_NAMESPACE_END
