#include "hittable_list.h"
#include "bvh.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

inline void     CHittableList::Clear()
{
    m_hittables.clear(); 
    m_bvhAccel->Clear();
}

//----------------------------------------------------

bool    CHittableList::Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec) const
{
    // BVH-Acceleration
    if (!m_bvhAccel->IsEmpty())
    {
        return m_bvhAccel->Hit(ray, t_min, t_max, hitRec);
    }

    // Brute-Force
    SHitRec hitTmp;
    bool    isHit = false;
    float   tClosest = _INFINITY;

    for (const auto &obj : m_hittables) {
        if (obj->Hit(ray, t_min, tClosest, hitTmp))
        {
            hitRec = hitTmp;
            tClosest = hitTmp.t;
            isHit = true;
        }
    }

    return isHit;
}

//----------------------------------------------------

bool    CHittableList::BuildBVHTree()
{
    // FIXME: not calling BuildBVHTree() will cause crash, because "m_bvhAccel" is not
    // instanced at all. Perhaps create a "CBVHAccel::Construct" function that separtes
    // the build process from instantiation.
    m_bvhAccel = std::make_shared<CBVHAccel>(CBVHAccel(m_hittables, 32, CBVHAccel::SAH));

    // clear local hittable list which now is a dublicate data with the one in bvh-tree.
    if (!m_bvhAccel->IsEmpty())
        m_hittables.clear();

    return true;
}

//----------------------------------------------------
_CD_NAMESPACE_END
