#include "renderer.h"
#include "hittable_list.h"
#include "camera.h"
#include "material.h"

#include <chrono>   // steady_clock

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

CRenderer::CRenderer()
: m_scene(std::make_shared<CHittableList>(CHittableList()))
, m_camera(std::make_shared<CCamera>(CCamera()))
, m_isFinished(false)
, m_currentSample(0)
, m_pixmap(nullptr)
{
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
    fflush(stdout);

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
    }

    m_isFinished = true;
    m_currentSample = m_renderSetting.nSamples;     // because it will be used for AA correction later

    // elapsed time
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    printf("[Render] Done.\n");
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

void    CRenderer::InitScene()
{
    // Camera
    float   aspectRatio = (float)m_renderSetting.render_w / m_renderSetting.render_h;
    m_camera = std::make_shared<CCamera>(CCamera(45.f, aspectRatio));
    m_camera->SetPos(glm::vec3(0, 0.65, -1));
    m_camera->LookAt(glm::vec3(0, 0, 0));

    // Material
    std::shared_ptr<cd::CMaterial>  mat_labmbertGreen = std::make_shared<cd::CMaterialLambertian>(glm::vec3(0.15, 0.6, 0.09));
    std::shared_ptr<cd::CMaterial>  mat_lambertWhite = std::make_shared<cd::CMaterialLambertian>(glm::vec3(1.0f));
    std::shared_ptr<cd::CMaterial>  mat_lambertBrown = std::make_shared<cd::CMaterialLambertian>(glm::vec3(0.92f, 0.59f, 0.17f));
    std::shared_ptr<cd::CMaterial>  mat_lambertWhiteGray = std::make_shared<cd::CMaterialLambertian>(glm::vec3(0.8f));
    std::shared_ptr<cd::CMaterial>  mat_lambertBlue = std::make_shared<cd::CMaterialLambertian>(glm::vec3(0.2, 0.18, 0.87));
    std::shared_ptr<cd::CMaterial>  mat_metalWhite = std::make_shared<cd::CMaterialMetal>(glm::vec3(1.0, 1.0, 1.0), 0);
    std::shared_ptr<cd::CMaterial>  mat_metalGold = std::make_shared<cd::CMaterialMetal>(glm::vec3(0.8, 0.6, 0.2), 0);
    std::shared_ptr<cd::CMaterial>  mat_metalBlue = std::make_shared<cd::CMaterialMetal>(glm::vec3(0.2, 0.3, 0.8), 0);
    std::shared_ptr<cd::CMaterial>  mat_metalRose = std::make_shared<cd::CMaterialMetal>(glm::vec3(0.8, 0.3, 0.2), 0.2);
    std::shared_ptr<cd::CMaterial>  mat_glass = std::make_shared<cd::CMaterialGlass>(1.9, 0);

    // Object
#if 0   // Use Obj
    auto    croissant = std::make_shared<cd::CHittableMesh>(glm::vec3(0, 0, 0), mat_lambertBrown);
    croissant->Load("Model/Croissants_obj/Croissant.obj");
    m_scene->Add(croissant);
#else
    //m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(0, 0, 0), 0.1, mat_lambertWhite)));
#endif
    m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(0.1, 0.097, 0.3), 0.15, mat_lambertWhite)));
    m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(0.35, 0.07, 0.18), 0.12, mat_metalRose)));
    m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(-0.3, 0.05, 0), 0.1, mat_metalWhite)));
    m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(0.18, 0.025, -0.15), 0.05, mat_glass)));
    m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(-0.155, 0.06, 0.23), 0.11, mat_metalBlue)));

    // floor
    m_scene->Add(std::make_shared<cd::CHittablePlane>(cd::CHittablePlane(glm::vec3(0, -0.1, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), 1, 1, mat_lambertWhite)));
    // m_scene->Add(std::make_shared<cd::CHittableSphere>(cd::CHittableSphere(glm::vec3(0, -10.05, 0), 10, mat_lambertWhite)));

    // BVH-Tree Construction
    m_scene->BuildBVHTree();
}

//----------------------------------------------------

glm::vec3   CRenderer::_Raycast(const CRay &ray)
{
    return _ConvolutionPrimaryRaycast(ray);
    // return _RecursivePathTrace(ray, m_renderSetting.nMaxDepth);
}

//----------------------------------------------------

// Primary raycast for convoluation domain raytracing.
// This is a camera -> object raycast.
glm::vec3   CRenderer::_ConvolutionPrimaryRaycast(const CRay &ray)
//color shader::_ConvolutionCast(const light *light, const hitrec &hit) const
{
    // if there is no hit, return background color
    // if (!&hit) return raytracer->setting.bgColor;

    // initialize
    float R0 = 0;
    float RN = 0;
    float cosDR;

    // 1. Compute R0 (Direct Illumination) Term

    SHitRec     hitRec;
    if (!m_scene->Hit(ray, 0.00001f, _INFINITY, hitRec))
        return glm::vec3(0);    // empty hit

    // TEMP Light
    constexpr glm::vec3 pointLight = {0, 0.5f, 0};
    float ndotl = glm::dot(glm::normalize(pointLight - hitRec.p), hitRec.n);
    ndotl = glm::clamp(ndotl, 0.f, 1.f);

    return hitRec.p_material->Albedo() * ndotl;

    // TODO: Create a Plane 'hittable'
    // TODO: Make that as a light

/*

    // *****	R0 TERM		***** //

	// compute general D/R towards the center of the light
	vec3 light_center = light->origin();
	R0 = raytracer->raycast_DR(hit, light_center, hit.object, 0);	// it will only return the r0 term.
	R0 = R0 > raytracer->setting.DIG ? R0 : raytracer->setting.DIG;

	// *** Archive this way of R0 shading
	//float cosT	= dot(normalize(light_center - hit.hitP), hit.normal);
	//float t		= (1 + cosT) / 2.f;
	//float delta = 0;
	//float overcos = (float)((2 + delta) / (1 + cosT + delta)) * DIG;
	//R0 = t * overcos + (1 - t) * (R0 + DIG);

	// ***** end of R0 (Direct Illumination) term

	// *****	RN TERM		***** //

	// Collect RN (Energy portion) for all objects
	// - compute ratio point
	// - compute ray
	// - compute d/r and light energy
	
	for (int i = 0; i < raytracer->scene_->shapes.size(); i++) {

		// ***** TEMP: Skip planes for now. It is either light or floor
		if (dynamic_cast<plane*>(raytracer->scene_->shapes[i])) continue;
		if (raytracer->scene_->shapes[i]->getID() == hit.object->getID()) continue;	// we already did this for R0

		// shoot RN ray to the target point
		vec3 targetPoint;

		//	different ray direction by light type
		if (light->get_type() == light::POINT) {
			
			targetPoint = light->origin();
		}
		else if (light->get_type() == light::DRAREA) {
		
			//	0. compute ratio point	//
			//	the ratio point is due to size of the light - object.

			vec3  light_center	= light->origin();
			vec3  object_center	= raytracer->scene_->shapes[i]->get_center();
			float sLight		= ((drAreaLight*)light)->get_size();
			float sObject		= raytracer->scene_->shapes[i]->get_size();

			// compute ratio point
			float ratio = sObject / (sLight + sObject);
			targetPoint = object_center + normalize(light_center - object_center) * distance(light_center, object_center) * ratio;	// FINAL POINT
		}

		// raycast RN
		RN += raytracer->raycast_DR(hit, targetPoint, raytracer->scene_->shapes[i], light);
	}

	// end of RN term (Energy Portion)

	// 3. Finally, combine with the light energy.
	
	// tweak values
	R0 *= raytracer->setting.K_R0;
	RN *= raytracer->setting.K_RN;

	cosDR = (raytracer->setting.DIG) / (R0 + RN);
	
	// computing surface color
	color surface_color = hit.object->get_mat().Diffuse() * (float)pow(cosDR * raytracer->setting.K_TOTAL_DR_S, raytracer->setting.EXP_TOTAL_DR_S);

	return surface_color;
*/

}

/*
// raycast in D/R ray
// D/R ray is a secondary ray that shoots towards the light from the primary hit.
float renderer::raycast_DR(const hitrec &hit, const vec3 &targetP, const shape* targetObj, const light* light) const {

	// ready to perform intersection test
	float R = 0;
	float t_start = 0;	// t of origin point(start) = 0
	float t_end = 0;
	float LIGHT_ENERGY = 0;

	bool  isInside;
	bool  selfIntersect;
	isInside = selfIntersect = hit.object->getID() == targetObj->getID();

	vec3	 new_origin = hit.hitP - hit.normal * setting.DIG;	// dig inside
	vec3	 new_direction = normalize(targetP - new_origin);	// towards center of the light
	ray		 new_ray(new_origin, new_direction);
	hitqueue hits;


	// ---	Step1. Perform intersection test with current target object		--- //

	targetObj->intersection(new_ray, hits);	// Intersection Test!


	while (!hits.empty()) {
		if (isInside) {	// going in -> out
//		if (dot(new_direction, hits.top().normal) >= 0) {	// going in -> out

			t_end = hits.top().t;
			float new_r = t_end - t_start;

			if (selfIntersect)	// R0 term happens here
				R += new_r;
			else
				R += (new_r / (t_start + setting.A_ZI));

			isInside = false;
		}
		else {				// going out -> in
			t_start = hits.top().t;
			isInside = true;
		}

		if (!hits.empty()) hits.pop();	// done with current hit object
	}

	// return R0 Term
	if (selfIntersect) return R;


	// ---	Step2. Perform intersection test with light	--- //

	// clear previous queue
	hitqueue emptyQueue;
	std::swap(hits, emptyQueue);

	if (light->get_type() == light::DRAREA) {

		LIGHT_ENERGY = ((drAreaLight*)(light))->get_intensity(new_ray);

	}
	else if (light->get_type() == light::POINT) {

		LIGHT_ENERGY = light->get_intensity();
	}

	// RN Term
	return R * LIGHT_ENERGY;
}
*/


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
