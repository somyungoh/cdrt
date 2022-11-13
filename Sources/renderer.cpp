#include "renderer.h"
#include "hittable_list.h"
#include "camera.h"
#include "light.h"
#include "material.h"
#include "ray.h"

#include <chrono>   // steady_clock


_CD_NAMESPACE_BEGIN
//----------------------------------------------------

CRenderer::CRenderer()
: m_isFinished(false)
, m_currentSample(0)
{
}

//----------------------------------------------------

void    CRenderer::InitScene()
{
    // Scene
    m_scene = std::make_shared<CHittableList>();

    // Camera
    float   aspectRatio = (float)m_renderSetting.render_w / m_renderSetting.render_h;
    m_camera = std::make_shared<CCamera>(CCamera(45.f, aspectRatio));
    m_camera->SetPos(glm::vec3(0, 0.65, -1));
    m_camera->LookAt(glm::vec3(0, 0, 0));

    // Light
    m_light = std::make_shared<CPointLight>(glm::vec3(0, 1.f, 0), glm::vec3(1));

    // Material
    std::shared_ptr<cd::IMaterial>  mat_labmbertGreen = std::make_shared<cd::CMaterialLambertian>(glm::vec3(0.15, 0.6, 0.09));
    std::shared_ptr<cd::IMaterial>  mat_lambertWhite = std::make_shared<cd::CMaterialLambertian>(glm::vec3(1.0f));
    std::shared_ptr<cd::IMaterial>  mat_lambertBrown = std::make_shared<cd::CMaterialLambertian>(glm::vec3(0.92f, 0.59f, 0.17f));
    std::shared_ptr<cd::IMaterial>  mat_lambertWhiteGray = std::make_shared<cd::CMaterialLambertian>(glm::vec3(0.8f));
    std::shared_ptr<cd::IMaterial>  mat_lambertBlue = std::make_shared<cd::CMaterialLambertian>(glm::vec3(0.2, 0.18, 0.87));
    std::shared_ptr<cd::IMaterial>  mat_metalWhite = std::make_shared<cd::CMaterialMetal>(glm::vec3(1.0, 1.0, 1.0), 0);
    std::shared_ptr<cd::IMaterial>  mat_metalGold = std::make_shared<cd::CMaterialMetal>(glm::vec3(0.8, 0.6, 0.2), 0);
    std::shared_ptr<cd::IMaterial>  mat_metalBlue = std::make_shared<cd::CMaterialMetal>(glm::vec3(0.2, 0.3, 0.8), 0);
    std::shared_ptr<cd::IMaterial>  mat_metalRose = std::make_shared<cd::CMaterialMetal>(glm::vec3(0.8, 0.3, 0.2), 0.2);
    std::shared_ptr<cd::IMaterial>  mat_glass = std::make_shared<cd::CMaterialGlass>(1.9, 0);

    // Hittables
    // OBJ Mesh
    auto    croissant = std::make_shared<cd::CHittableMesh>(glm::vec3(0, 0, 0), mat_lambertBrown);
    croissant->Load("Model/Croissants_obj/Croissant.obj");
    m_scene->Add(croissant);

    // some spheres
    m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(0.1, 0.097, 0.3), 0.15, mat_lambertWhite)));
    m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(0.35, 0.07, 0.18), 0.12, mat_metalRose)));
    m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(-0.2, 0.3, 0.02), 0.15, mat_metalWhite)));
    m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(0.18, 0.025, -0.15), 0.1, mat_metalRose)));
    m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(-0.155, 0.06, 0.23), 0.11, mat_metalBlue)));
    m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(0.07, 0.3, -0.05), 0.1, mat_metalBlue)));

    // floor
    m_scene->Add(std::make_shared<cd::CHittablePlane>(cd::CHittablePlane(glm::vec3(0, -0.1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), 1, 1, mat_lambertWhite)));

    // BVH-Tree Construction
    m_scene->BuildBVHTree();
}

//----------------------------------------------------

glm::vec3   CRenderer::_Raycast(const CRay &ray)
{
#if 1
    return _ConvolutionPrimaryRaycast(ray);
#else
    return _RecursivePathTrace(ray, m_renderSetting.nMaxDepth);
#endif
}

//----------------------------------------------------

// Primary raycast for convoluation domain raytracing.
// This is a camera -> object raycast.
glm::vec3   CRenderer::_ConvolutionPrimaryRaycast(const CRay &ray)
//color shader::_ConvolutionCast(const light *light, const hitrec &hit) const
{
    // Set of data to collect
    float R0 = 0;
    float RN = 0;

    // Primary Hit!
    SHitRec     hitRec;
    if (!m_scene->Hit(ray, _EPSILON, _INFINITY, hitRec))
        return glm::vec3(0);    // empty hit`

    // ------------------------------------------------
    // 1. Compute R0 (Direct Illumination) Term
    // ------------------------------------------------
#if 1
    // compute general D/R towards the center of the light
    R0 = _ConvolutionSecondRaycast(ray, m_light->Origin(), hitRec.p_hittable, hitRec);
#else
    // N dot L, looks the same but way cheaper
    R0 = glm::clamp(glm::dot(glm::normalize(pointLight - hitRec.p), hitRec.n));
#endif

    // ------------------------------------------------
    // 2. Compute RN (Energy Portion) Term
    // ------------------------------------------------

    // Collect RN (Energy portion) for all objects
    for (auto &p_hittable : m_scene->m_hittables)
    {
        // We already performed self-intersection from R0
        if (p_hittable == hitRec.p_hittable)
            continue;

        // shoot RN ray to the target point
        glm::vec3 targetPoint;
        targetPoint = m_light->Origin();

        // TODO: Area Light
        // ---------------------< Original Code >----------------------
//		if (light->get_type() == light::POINT) {
//			targetPoint = light->origin();
//		}
//		else if (light->get_type() == light::DRAREA) {
//
//			//	0. compute ratio point	//
//			//	the ratio point is due to size of the light - object.
//
//			vec3  light_center	= light->origin();
//			vec3  object_center	= raytracer->scene_->shapes[i]->get_center();
//			float sLight		= ((drAreaLight*)light)->get_size();
//			float sObject		= raytracer->scene_->shapes[i]->get_size();
//
//			// compute ratio point
//			float ratio = sObject / (sLight + sObject);
//			targetPoint = object_center + normalize(light_center - object_center) * distance(light_center, object_center) * ratio;	// FINAL POINT
//		}
        // ----------------------------------------------------------

        // raycast RN
        RN += _ConvolutionSecondRaycast(ray, targetPoint, p_hittable, hitRec);
        RN *= _ConvolutionThirdRaycast(ray, targetPoint, p_hittable, hitRec);
    }

    // ------------------------------------------------
    // 3. Combine all together
    // ------------------------------------------------
    // tweak R0, RN
    R0 *= m_renderSetting.K_R0;
    RN *= m_renderSetting.K_RN;

    float cosDR = m_renderSetting.K_DIG / (R0 + RN + _EPSILON);
    // teak cosDR
    cosDR = glm::pow(cosDR * m_renderSetting.K_TOTAL_DR_S, m_renderSetting.EXP_TOTAL_DR_S);

    // final color
    const glm::vec3 out_color = hitRec.p_material->Albedo() * cosDR;

    return out_color;
}

//----------------------------------------------------
// This secondary raycast collects all the hits from the new ray,
// which corresponds to amount of occlusion (R0, RN)
float   CRenderer::_ConvolutionSecondRaycast(const CRay &primaryRay, const glm::vec3 &targetP, const std::shared_ptr<IHittable> &targetObj, const SHitRec &primaryHitRec)
{
    // secondary ray
    const glm::vec3 new_origin = primaryHitRec.p - primaryHitRec.n * m_renderSetting.K_DIG;;
    const glm::vec3 new_direction = glm::normalize(targetP - new_origin);    // towards center of the light
    const CRay      secondRay = {new_origin, new_direction};

    // hitqueue hits;
    VHits hits;

    targetObj->HitAll(secondRay, _EPSILON, _INFINITY, hits);
    std::sort(hits.begin(), hits.end(), cmpHitRec);

    // Data to collect
    float R = 0;
    float t_start = 0;
    float t_end = 0;

    // TODO: Make sure it's always inside for R0.
    // If it's not the case, we did something wrong, or the D value is not small enough.

    // HACK: This will ensure that 't_start' will initially set differently:
    //  - 0: If it's computing R0
    //  - t: If it's computing RN
    bool  isInside = (targetObj == primaryHitRec.p_hittable);
    for (auto &hit : hits)
    {
        if (isInside)
        {
            t_end = hit.t;
            const float  new_r = t_end - t_start;
            R += new_r;
        }
        else
        {
            t_start = hit.t;
        }
        isInside = !isInside;
    }

    return R;
}

//----------------------------------------------------
// This third raycast computes the light energy from the given ray.
float   CRenderer::_ConvolutionThirdRaycast(const CRay &primaryRay, const glm::vec3 &targetP, const std::shared_ptr<IHittable> &targetObj, const SHitRec &primaryHitRec)
{
    const glm::vec3 new_origin = primaryHitRec.p - primaryHitRec.n * m_renderSetting.K_DIG;
    const glm::vec3 new_direction = glm::normalize(targetP - new_origin);    // towards center of the light
    const CRay      secondRay = {new_origin, new_direction};

    const float     LIGHT_ENERGY = m_light->GetIntensityFromRay(secondRay);

    // RN Term
    return LIGHT_ENERGY;
}

//----------------------------------------------------

glm::vec3   CRenderer::_RecursivePathTrace(const CRay &ray, int depth)
{
    // max-depth reached
    if (depth <= 0) {
        return glm::vec3(0);
    }

    SHitRec     hitRec;
    if (m_scene->Hit(ray, 0.00001f, _INFINITY, hitRec))
    {
        // bounced rays
        CRay        scatteredRay;
        glm::vec3   attenuation;
        if (hitRec.p_material->Scatter(ray, hitRec, attenuation, scatteredRay))
            return attenuation * _RecursivePathTrace(scatteredRay, depth - 1);
        return glm::vec3(0);
    }

    // coloring
    float   t = 0.5f * (ray.m_dir.y + 1.0f);
    return glm::vec3(1.0) * (1.0f - t) + glm::vec3(0.5, 0.7, 1.0) * t;
}

//----------------------------------------------------

void    CRenderer::Render()
{
    if (m_pixmap == nullptr)    // initial render
        m_pixmap = new float[m_renderSetting.render_w * m_renderSetting.render_h * 3];
    else if (m_isFinished)      // previous render exists
        _ClearOldRender();

    // Render loop
    printf("[Render] Start rendering...\n");
    printf("[0].....................|...................[100]\n");
    printf("   ");
    fflush(stdout);

    const int logEvery = m_renderSetting.render_w / 41;

    // Timer
    auto    begin = std::chrono::steady_clock::now();

    for (size_t w = 0; w < m_renderSetting.render_w; w++) {
        for (size_t h = 0; h < m_renderSetting.render_h; h++) {
#pragma omp parallel for
            for (size_t s = 0; s < m_renderSetting.nSamples; s++)
            {
                const int   si = s % m_renderSetting.nSamplesW;
                const int   sj = s / m_renderSetting.nSamplesH;
                const float u = (w + (float)si / m_renderSetting.nSamplesW + m_renderSetting.nSamplesOffset) / m_renderSetting.render_w;
                const float v = (h + (float)sj / m_renderSetting.nSamplesH + m_renderSetting.nSamplesOffset) / m_renderSetting.render_h;
                const CRay  ray = m_camera->GetRay(u, v);

                // Raycast!
                glm::vec3   color = _Raycast(ray);

                m_pixmap[(h * m_renderSetting.render_w + w) * 3 + 0] += color.r;
                m_pixmap[(h * m_renderSetting.render_w + w) * 3 + 1] += color.g;
                m_pixmap[(h * m_renderSetting.render_w + w) * 3 + 2] += color.b;
            }
        }
        if ((int)(w % logEvery) == 0)
            printf("|");
    }
    printf("\n");

    m_isFinished = true;
    m_currentSample = m_renderSetting.nSamples;     // because it will be used for AA correction later

    // elapsed time
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    printf("[Render] Elpased: %.3fs.\n", elapsed / 1000.f);
}

//----------------------------------------------------

void    CRenderer::GetLastRender(float* &outMap)
{
    if (outMap == nullptr)
        outMap = new float[m_renderSetting.render_w * m_renderSetting.render_h * 3];

    for (size_t w = 0; w < m_renderSetting.render_w; w++) {
#pragma omp parallel for
        for (size_t h = 0; h < m_renderSetting.render_h; h++)
        {
            glm::vec3 rawColor( m_pixmap[(h * m_renderSetting.render_w + w) * 3 + 0],
                                m_pixmap[(h * m_renderSetting.render_w + w) * 3 + 1],
                                m_pixmap[(h * m_renderSetting.render_w + w) * 3 + 2]);

            // gamma correction
            float   scale = 1.0f / m_currentSample;
            rawColor = glm::sqrt(rawColor * scale);

            // copy
            outMap[(h * m_renderSetting.render_w + w) * 3 + 0] = rawColor.r;
            outMap[(h * m_renderSetting.render_w + w) * 3 + 1] = rawColor.g;
            outMap[(h * m_renderSetting.render_w + w) * 3 + 2] = rawColor.b;
        }
    }
}

//----------------------------------------------------

void    CRenderer::SetRenderSetting(const SRenderSetting &renderSetting)
{
    m_renderSetting                 = renderSetting;
    m_renderSetting.nSamplesW       = glm::sqrt(m_renderSetting.nSamples);
    m_renderSetting.nSamplesH       = glm::sqrt(m_renderSetting.nSamples);
    m_renderSetting.nSamplesOffset  = 0.5f / m_renderSetting.nSamplesW;
}

//----------------------------------------------------

void    CRenderer::_ClearOldRender()
{
    m_isFinished = false;
    m_currentSample = 0;

#pragma omp parallel for
    for (int i = 0; i < m_renderSetting.render_w * m_renderSetting.render_h * 3; ++i)
        m_pixmap[i] = 0.0f;
}

//----------------------------------------------------
_CD_NAMESPACE_END
