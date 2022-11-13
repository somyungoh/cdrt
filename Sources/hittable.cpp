#include "hittable_list.h"
#include "ray.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

CHittableSphere::CHittableSphere(const glm::vec3 &origin, float radius, const std::shared_ptr<IMaterial> &material)
: m_origin(origin)
, m_radius(radius)
{
    m_material = material;
    m_aabb = CAABB(m_origin - m_radius, m_origin + m_radius);
}

//----------------------------------------------------

bool    CHittableSphere::Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec)
{
    glm::vec3   oc = ray.m_origin - m_origin;
    float       b = glm::dot(oc, ray.m_dir);
    float       c = glm::dot(oc, oc) - m_radius * m_radius;
    float       h = b * b - c;

    if (h < 0.0)
        return false;

    hitRec.t = -b - glm::sqrt(h);
    if (hitRec.t < t_min)
        hitRec.t = -b + glm::sqrt(h);
    if (hitRec.t < t_min || hitRec.t > t_max)
        return false;

    hitRec.p = ray.At(hitRec.t);
    hitRec.n = (hitRec.p - m_origin) / m_radius;
    hitRec.setFaceNormal(ray);
    hitRec.p_hittable = shared_from_this();
    hitRec.p_material = m_material;

    return true;
}

//----------------------------------------------------

bool    CHittableSphere::HitAll(const CRay &ray, float t_min, float t_max, VHits &hits)
{
    glm::vec3   oc = ray.m_origin - m_origin;
    float       b = glm::dot(oc, ray.m_dir);
    float       c = glm::dot(oc, oc) - m_radius * m_radius;
    float       h = b * b - c;

    if (h < 0.0)
        return false;

    float t[2];
    SHitRec hitRec;

    t[0] = -b - glm::sqrt(h);
    t[1] = -b + glm::sqrt(h);

    hitRec.t = t[0];
    if (hitRec.t < t_min)
        hitRec.t = t[1];
    if (hitRec.t < t_min || hitRec.t > t_max)
        return false;

    hitRec.p = ray.At(hitRec.t);
    hitRec.n = (hitRec.p - m_origin) / m_radius;
    hitRec.setFaceNormal(ray);
    hitRec.p_hittable = shared_from_this();
    hitRec.p_material = m_material;
    hits.push_back(hitRec);

    if (hitRec.t == t[1])
        return true;

    hitRec.t = t[1];
    hitRec.p = ray.At(hitRec.t);
    hitRec.n = (hitRec.p - m_origin) / m_radius;
    hitRec.setFaceNormal(ray);
    hitRec.p_hittable = shared_from_this();
    hitRec.p_material = m_material;
    hits.push_back(hitRec);
    return true;
}

//----------------------------------------------------

CHittableTriangle::CHittableTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const std::shared_ptr<IMaterial> &material)
: m_v0(v0)
, m_v1(v1)
, m_v2(v2)
, m_n(glm::normalize(glm::cross(v1 - v0, v0 - v2)))
{
    m_material = material;
    m_aabb = CAABB(
        glm::vec3(glm::min(glm::min(v0.x, v1.x), v2.x),
                  glm::min(glm::min(v0.y, v1.y), v2.y),
                  glm::min(glm::min(v0.z, v1.z), v2.z)),
        glm::vec3(glm::max(glm::max(v0.x, v1.x), v2.x),
                  glm::max(glm::max(v0.y, v1.y), v2.y),
                  glm::max(glm::max(v0.z, v1.z), v2.z))
    );
}

//----------------------------------------------------

bool    CHittableTriangle::Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec)
{
    glm::vec3   v1v0 = m_v1 - m_v0;
    glm::vec3   v2v0 = m_v2 - m_v0;
    glm::vec3   rov0 = ray.m_origin - m_v0;
    glm::vec3   n = glm::cross( v1v0, v2v0 );
    glm::vec3   q = glm::cross( rov0, ray.m_dir );
    float       d = 1.0f / dot( ray.m_dir, n );
    float       u = d * glm::dot( -q, v2v0 );
    float       v = d * glm::dot(  q, v1v0 );
    float       t = d * glm::dot( -n, rov0 );

    if (u < 0.0f || v < 0.0f || (u + v) > 1.0f)
        return false;
    else if (t < t_min || t > t_max)
        return false;

    // there is a hit
    hitRec.t = t;
    hitRec.p = ray.At(t);
    hitRec.n = m_n;
    hitRec.setFaceNormal(ray);
    hitRec.p_hittable = shared_from_this();
    hitRec.p_material = m_material;

    return true;
}

//----------------------------------------------------

bool    CHittableTriangle::HitAll(const CRay &ray, float t_min, float t_max, VHits &hits)
{
    SHitRec   hitRec;
    if(Hit(ray, t_min, t_max, hitRec))
    {
        hits.push_back(hitRec);
        return true;
    }
    return false;
}

//----------------------------------------------------

CHittablePlane::CHittablePlane(const glm::vec3 &origin, const glm::vec3 &normal, const glm::vec3 &up, float sx, float sy, const std::shared_ptr<IMaterial> &material)
: m_origin(origin)
, m_vz(glm::normalize(normal))
, m_vy(glm::normalize(up))
, m_vx(glm::normalize(glm::cross(normal, up)))
, m_sx(sx)
, m_sy(sy)
{
    m_material = material;

    const glm::vec3 p1 = m_origin - m_vx * m_sx * 0.5f - m_vy * sy * 0.5f - m_vz * _EPSILON;
    const glm::vec3 p2 = m_origin + m_vx * m_sx * 0.5f + m_vy * sy * 0.5f + m_vz * _EPSILON;
    m_aabb = CAABB( glm::vec3(glm::min(p1.x, p2.x), glm::min(p1.y, p2.y), glm::min(p1.z, p2.z)),
                    glm::vec3(glm::max(p1.x, p2.x), glm::max(p1.y, p2.y), glm::max(p1.z, p2.z))
    );
}

//----------------------------------------------------

bool    CHittablePlane::Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec)
{
    const glm::vec3     oc = m_origin - ray.m_origin;
    const float         dotNL = glm::dot(ray.m_dir, m_vz);

    // compute t and intersection point
    const float t = dot(oc, m_vz) / dotNL;

    if (t < t_min || t > t_max)
        return false;

    const glm::vec3     intersection_point = ray.m_origin + ray.m_dir * t;
    const glm::vec3     projected_vector = intersection_point - m_origin;
    const float         dotPNX = dot(projected_vector, m_vx);
    const float         dotPNY = dot(projected_vector, m_vy);

    // there is a hit
    if (t > _EPSILON
        && dotPNX >= -0.5 * m_sx && dotPNX < m_sx * 0.5
        && dotPNY >= -0.5 * m_sy && dotPNY < m_sy * 0.5)
    {
        hitRec.t = t;
        hitRec.p = ray.At(hitRec.t);
        hitRec.n = m_vz;
        hitRec.setFaceNormal(ray);
        hitRec.p_hittable = shared_from_this();
        hitRec.p_material = m_material;

        return true;
    }
    return false;
}

//----------------------------------------------------

bool    CHittablePlane::HitAll(const CRay &ray, float t_min, float t_max, VHits &hits)
{
    SHitRec   hitRec;
    if(Hit(ray, t_min, t_max, hitRec))
    {
        hits.push_back(hitRec);
        return true;
    }
    return false;
}

//----------------------------------------------------

CHittableMesh::CHittableMesh(const glm::vec3 &origin, const std::shared_ptr<IMaterial> &material)
: m_origin(origin)
, m_triangles(std::make_shared<CHittableList>())
, m_isMeshLoaded(false)
{
    m_material = material;
}

//----------------------------------------------------

bool    CHittableMesh::Load(const char* file)
{
    // load obj
    tinyobj::attrib_t                   attrib;
    std::vector<tinyobj::shape_t>       shapes;
    std::vector<tinyobj::material_t>    materials;

    std::string     warn;
    std::string     err;

    printf("[Mesh] Loading obj \"%s\"\n", file);
    bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                                file, NULL, true);

    if (!warn.empty())
        printf("[Mesh] Warn: %s\n", warn.c_str());

    if (!err.empty())
        printf("[Mesh] Err: %s\n", err.c_str());
    
    if (!res)
    {
        printf("[Mesh] Failed to load obj \"%s\"\n", file);
        return false;
    }

    // TODO: Support multi-shape obj
    // We assume that the object structure has a single shape
    if (shapes.size() != 1)
        throw std::runtime_error("More than one shape found in the mesh!");

    printf("[Mesh] # of vertices  : %lu\n", attrib.vertices.size() / 3);
    printf("[Mesh] # of normals   : %lu\n", attrib.normals.size() / 3);
    printf("[Mesh] # of faces     : %lu\n", shapes[0].mesh.indices.size() / 3);

    std::vector<glm::vec3>  uniqueVertices;
    std::vector<uint32_t>   uniqueIndices;

    // copy vertices and indices
    for (const auto& shape : shapes) {
        // tiny obj loader did a triangulation for us, so we directly load them
        // into our member, with offset of 3
        for (const auto& index : shape.mesh.indices)
        {
            // vertex
            glm::vec3 v(
                static_cast<float>(attrib.vertices[3 * index.vertex_index + 0]),
                static_cast<float>(attrib.vertices[3 * index.vertex_index + 1]),
                static_cast<float>(attrib.vertices[3 * index.vertex_index + 2])
            );

            auto    it = std::find(uniqueVertices.begin(), uniqueVertices.end(), v);
            size_t  indicesIdx;
            // only keep unique vertices
            if (it == uniqueVertices.end())
            {
                uniqueVertices.push_back(v);
                uniqueIndices.push_back(static_cast<uint32_t>(m_vertices.size()));
                m_vertices.push_back(v);

                indicesIdx = uniqueVertices.size() - 1;
            }
            else
                indicesIdx = it - uniqueVertices.begin();

            // indices
            m_indices.push_back(uniqueIndices[indicesIdx]);
        }
    }

    // precompute triangle data
    for (size_t i = 0; i < m_indices.size() / 3; i++)
    {
        auto triangle = std::make_shared<CHittableTriangle>(m_vertices[m_indices[i * 3 + 0]],
                                                            m_vertices[m_indices[i * 3 + 1]],
                                                            m_vertices[m_indices[i * 3 + 2]],
                                                            m_material);
        m_triangles->Add(triangle);

        // AABB
        m_aabb.pMin.x = glm::min(m_aabb.pMin.x, triangle->m_aabb.pMin.x);
        m_aabb.pMin.y = glm::min(m_aabb.pMin.y, triangle->m_aabb.pMin.y);
        m_aabb.pMin.z = glm::min(m_aabb.pMin.z, triangle->m_aabb.pMin.z);
        m_aabb.pMax.x = glm::max(m_aabb.pMax.x, triangle->m_aabb.pMax.x);
        m_aabb.pMax.y = glm::max(m_aabb.pMax.y, triangle->m_aabb.pMax.y);
        m_aabb.pMax.z = glm::max(m_aabb.pMax.z, triangle->m_aabb.pMax.z);
    }

    m_triangles->BuildBVHTree();

    printf("[Mesh] Finished loading obj \"%s\"\n", file);

    m_isMeshLoaded = true;

    return m_isMeshLoaded;
}

//----------------------------------------------------

bool    CHittableMesh::Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec)
{
    if (!m_isMeshLoaded)
        return false;

    return m_triangles->Hit(ray, t_min, t_max, hitRec);
}

//----------------------------------------------------

bool    CHittableMesh::HitAll(const CRay &ray, float t_min, float t_max, VHits &hits)
{
    if (!m_isMeshLoaded)
        return false;

    return m_triangles->HitAll(ray, t_min, t_max, hits);
}

//----------------------------------------------------

_CD_NAMESPACE_END
