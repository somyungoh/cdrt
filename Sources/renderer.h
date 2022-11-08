#pragma once

#include "common.h"
#include "ray.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

class CHittableList;
class CCamera;

//----------------------------------------------------

struct SRenderSetting
{
    u_int32_t   render_w, render_h;
    u_int32_t   nSamples;
    u_int32_t   nMaxDepth;

    // AA
    u_int32_t   nSamplesW, nSamplesH;
    float       nSamplesOffset;
};

//----------------------------------------------------

class CRenderer
{
public:
    CRenderer();

    void    Render();
    void    GetLastRender(float* &outMap);

    void    SetRenderSetting(const SRenderSetting &renderSetting);
    void    InitScene();

    bool    IsFinished() { return m_isFinished; };

private:
    // Different Raycast methods.
    glm::vec3   _Raycast(const CRay &ray);
    glm::vec3   _ConvolutionPrimaryRaycast(const CRay &ray);
    glm::vec3   _ConvolutionSecondaryRaycast(const CRay &ray);
    glm::vec3   _RecursivePathTrace(const CRay &ray, int depth);

private:
    void        _ClearOldRender();

private:
    std::shared_ptr<CHittableList>  m_scene;
    // TODO: Light
    std::shared_ptr<CCamera>        m_camera;

    SRenderSetting                  m_renderSetting;
    bool                            m_isFinished;
    u_int32_t                       m_currentSample;    // for progressive rendering

    float*                          m_pixmap;
};

//----------------------------------------------------
_CD_NAMESPACE_END
