#pragma once

/*************************************************************************
*
*		bvh.h
*
*		Implementation of Bounding Volume Hierarchies (BVH)
*		acceleration structure. This is primitive based
*		partition and consists 3 different partition algorithm - 
*		midpoint, equal subset, surface area heuristic (sah).
*
*		This code referenced and modified the book "Physically Based
*		Rendering" chaper4.3, Bounding Volume Hierarchies.
*		https://www.pbrt.org/
*
**************************************************************************/

#include "common.h"
#include "aabb.h"
#include "ray.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

struct SHitRec;
class IHittable;
typedef std::vector<SHitRec> VHits;

//----------------------------------------------------

class CBVHAccel
{
    struct SHittableInfo
    {
        SHittableInfo() {}
        SHittableInfo(size_t hittableNum, const CAABB &bounds)
        : hittableNum(hittableNum)
        , bounds(bounds)
        , centroid((bounds.pMin + bounds.pMax) * 0.5f)
        {
        }

        size_t      hittableNum;
        glm::vec3   centroid;
        CAABB       bounds;
    };

    // node structure for BVH build
    struct SBVHBuildNode
    {
        void InitLeaf(int first, int n, const CAABB &b)
        {
            firstHittableOffset = first;
            nHittables = n;
            bounds = b;
            children[0] = children[1] = nullptr;	// leaf node is determined by nullptr
        }

        void InitInterior(int axis, SBVHBuildNode *c0, SBVHBuildNode *c1)
        {
            children[0] = c0;
            children[1] = c1;
            bounds = c0->bounds + c1->bounds;
            splitAxis = axis;
            nHittables = 0;
        }

        SBVHBuildNode   *children[2];
        int             splitAxis, firstHittableOffset, nHittables;
        CAABB           bounds;
    };

    struct SLinearBVHNode
    {
        union 
        {
            int hittablesOffset;    // leaf
            int secondChildOffset;  // interior
        };
        uint16_t    nHittables;     // 0 -> interior nodes
        uint8_t     axis;           // interior node : xyz
        uint8_t     pad[1];         // ensure 32 byte total size
        CAABB       bounds;
    };

    struct SBucketInfo
    {
        int     count = 0;
        CAABB   bounds;
    };

public:
    enum EPartitionType { MIDPOINT, EQUALSUBSET, SAH };

    //constructor
    CBVHAccel();
    CBVHAccel(const std::vector<std::shared_ptr<IHittable>> &hittables, int maxHittablesInNode, EPartitionType partitionType);
    ~CBVHAccel();

    bool            Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec) const;
    bool            HitAll(const CRay &ray, float t_min, float t_max, VHits &hits) const;
    inline bool     IsEmpty() const { return (m_nodes == nullptr); }
    void            Clear();

private:
    bool            _BuildTree();
    SBVHBuildNode*  _RecursiveBuild(std::vector<SHittableInfo> &hittableInfo, int start, int end, int *totalNodes, std::vector<std::shared_ptr<IHittable>> &orderedHittables);
    int             _FlattenBVHTree(SBVHBuildNode *node, int *offset);

    int                                     m_maxHittablesInNode;
    EPartitionType                          m_partitionMethod;
    std::vector<std::shared_ptr<IHittable>> m_hittables;
    SLinearBVHNode*                         m_nodes = nullptr;

};

//----------------------------------------------------
_CD_NAMESPACE_END
