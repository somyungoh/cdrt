#pragma once

#include "hittable.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

class CBVHAccel;

//----------------------------------------------------

class CHittableList : public IHittable
{
public:
    inline void     Add(const std::shared_ptr<IHittable> &object) { m_hittables.push_back(object); };
    inline void     Clear();

    virtual bool    Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec) override;
    virtual bool    HitAll(const CRay &ray, float t_min, float t_max, VHits &hits) override;

    // Construct bvh-tree from the loaded hittables. Call this once all the
    // hittables are loaded in "m_hittables".
    bool            BuildBVHTree();

public:
    // Hittable list will first attempt to use "m_bvhAccel" is available, 
    // otherwise uses "m_hittables" which is a brute-force traversal.
    std::vector<std::shared_ptr<IHittable>>     m_hittables;
    std::shared_ptr<CBVHAccel>                  m_bvhAccel;     // bvh-tree acceleration
};

//----------------------------------------------------
_CD_NAMESPACE_END
