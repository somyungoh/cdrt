#pragma once

#include "common.h"
#include "aabb.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

class CHittableList;
class IMaterial;
class CRay;

//----------------------------------------------------

struct SHitRec
{
    glm::vec3                   p;
    glm::vec3                   n;
    float                       t;
    std::shared_ptr<IMaterial>  p_material;
    bool                        frontFace;

    inline void setFaceNormal(const CRay &ray)
    {
        frontFace = glm::dot(ray.m_dir, n) < 0;
        n = frontFace ? n : -n;
    }
};

// vector type
typedef std::vector<SHitRec> VHits;

// comparator
static bool     cmpHitRec(const SHitRec &hr1, const SHitRec &hr2) { return hr1.t < hr2.t; };

//----------------------------------------------------

class IHittable
{
public:
    virtual bool    Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec) const = 0;
    virtual bool    HitAll(const CRay &ray, float t_min, float t_max, VHits &hits) const = 0;

public:
    std::shared_ptr<IMaterial>  m_material;
    CAABB                       m_aabb;
};

//----------------------------------------------------

class CHittableSphere : public IHittable
{
public:
    CHittableSphere(const glm::vec3 &origin, float radius, const std::shared_ptr<IMaterial> &material);

    virtual bool    Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec) const override;
    virtual bool    HitAll(const CRay &ray, float t_min, float t_max, VHits &hits) const override;

public:
    glm::vec3   m_origin;
    float       m_radius;
};

//----------------------------------------------------

class CHittableTriangle : public IHittable
{
public:
    CHittableTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const std::shared_ptr<IMaterial> &material);

    virtual bool    Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec) const override;
    virtual bool    HitAll(const CRay &ray, float t_min, float t_max, VHits &hits) const override;

public:
    glm::vec3   m_v0, m_v1, m_v2;
    glm::vec3   m_n;
};

//----------------------------------------------------

class CHittablePlane : public IHittable
{
public:
    CHittablePlane(const glm::vec3 &origin, const glm::vec3 &normal, const glm::vec3 &up, float sx, float sy, const std::shared_ptr<IMaterial> &material);

    virtual bool    Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec) const override;
    virtual bool    HitAll(const CRay &ray, float t_min, float t_max, VHits &hits) const override;

public:
    glm::vec3   m_origin;
    glm::vec3   m_vx, m_vy, m_vz;
    float       m_sx, m_sy;
};

//----------------------------------------------------

class CHittableMesh : public IHittable
{
public:
    CHittableMesh(const glm::vec3 &origin, const std::shared_ptr<IMaterial> &material);

    virtual bool    Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec) const override;
    virtual bool    HitAll(const CRay &ray, float t_min, float t_max, VHits &hits) const override;
    bool            Load(const char* file);

public:
    glm::vec3                       m_origin;

private:
    // mesh data
    std::vector<glm::vec3>          m_vertices;
    std::vector<uint32_t>           m_indices;
    std::shared_ptr<CHittableList>  m_triangles;

    bool                            m_isMeshLoaded;
};

//----------------------------------------------------
_CD_NAMESPACE_END
