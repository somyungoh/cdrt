#pragma once

#include "common.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

class IHittable;
class CHittableList;
class ILight;
class CCamera;
class CRay;
struct SHitRec;

//----------------------------------------------------

struct SRenderSetting
{
    u_int32_t   render_w, render_h;
    u_int32_t   nSamples;
    u_int32_t   nMaxDepth;

    // D/R
    float K_DIG;
    float A_ZI;
    float K_R0;
    float K_RN;
    float K_TOTAL_DR_S;
    float EXP_TOTAL_DR_S;

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
    float       _ConvolutionSecondRaycast(const CRay &primaryRay, const glm::vec3 &targetP,  const std::shared_ptr<IHittable> &targetObj, const SHitRec &primaryHitRec);
    float       _ConvolutionThirdRaycast(const CRay &primaryRay, const glm::vec3 &targetP, const std::shared_ptr<IHittable> &targetObj, const SHitRec &primaryHitRec);
    glm::vec3   _RecursivePathTrace(const CRay &ray, int depth);

private:
    void        _ClearOldRender();

private:
    std::shared_ptr<CHittableList>  m_scene;
    std::shared_ptr<ILight>         m_light;    // TODO: Generic light type
    std::shared_ptr<CCamera>        m_camera;

    SRenderSetting                  m_renderSetting;
    bool                            m_isFinished;
    u_int32_t                       m_currentSample;

    float*                          m_pixmap;
};

//----------------------------------------------------
_CD_NAMESPACE_END
