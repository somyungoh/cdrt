#include "bvh.h"
#include "hittable.h"

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

CBVHAccel::CBVHAccel()
{
}

//----------------------------------------------------

CBVHAccel::CBVHAccel(const std::vector<std::shared_ptr<IHittable>> &hittables, int maxHittablesInNode, EPartitionType partitionType)
: m_hittables(hittables)
, m_maxHittablesInNode(std::min(255, maxHittablesInNode))
, m_partitionMethod(partitionType)
{
    _BuildTree();
}

//----------------------------------------------------

CBVHAccel::~CBVHAccel()
{
    // delete[] m_nodes;    Why not working?
}

//----------------------------------------------------

bool CBVHAccel::Hit(const CRay &ray, float t_min, float t_max, SHitRec &hitRec) const
{
    SHitRec     hitTmp;
    bool        isHit = false;
    float       tClosest = t_max;

    // follow ray through BVH nodes to find primitive intersections
    int     toVisitOffset = 0;
    int     currentNodeIndex = 0;
    int     nodesToVisit[64];

    while (true) {
        const SLinearBVHNode    *node = &m_nodes[currentNodeIndex];

        // check ray against BVH node
        if (node->bounds.Hit(ray)) {
            if (node->nHittables > 0)
            {
                // intersect ray with primitives in leaf BVH node
                for (int i = 0; i < node->nHittables; i++) {
                    if (m_hittables[node->hittablesOffset + i]->Hit(ray, t_min, tClosest, hitTmp))
                    {
                        hitRec = hitTmp;
                        tClosest = hitTmp.t;
                        isHit = true;
                    }
                }
                if (toVisitOffset == 0)
                    break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            }
            else 
            {
                // put far BVH node on nodesToVisit stack, advance to near node
                const glm::vec3 invDir = 1.f / ray.m_dir;
                int             dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };

                if (dirIsNeg[node->axis])
                {
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                }
                else
                {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        }
        else {
            if (toVisitOffset == 0)
                break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }

    return isHit;
}

//----------------------------------------------------

bool CBVHAccel::HitAll(const CRay &ray, float t_min, float t_max, VHits &hits) const
{
    // follow ray through BVH nodes to find primitive intersections
    int     toVisitOffset = 0;
    int     currentNodeIndex = 0;
    int     nodesToVisit[64];

    while (true) {
        const SLinearBVHNode    *node = &m_nodes[currentNodeIndex];

        // check ray against BVH node
        if (node->bounds.Hit(ray)) {
            if (node->nHittables > 0)
            {
                // intersect ray with primitives in leaf BVH node
                for (int i = 0; i < node->nHittables; i++)
                    m_hittables[node->hittablesOffset + i]->HitAll(ray, t_min, t_max, hits);
                if (toVisitOffset == 0)
                    break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            }
            else
            {
                // put far BVH node on nodesToVisit stack, advance to near node
                const glm::vec3 invDir = 1.f / ray.m_dir;
                int             dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };

                if (dirIsNeg[node->axis])
                {
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                }
                else
                {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        }
        else {
            if (toVisitOffset == 0)
                break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }

    return hits.size() > 0;
}

//----------------------------------------------------

void    CBVHAccel::Clear()
{
    m_hittables.clear();
    delete[] m_nodes;
}

//----------------------------------------------------

bool   CBVHAccel:: _BuildTree()
{
    if (m_hittables.size() == 0)
        return true;

    // BVH-Tree construction
    printf("[BVH] Start bvh-tree construction...\n");

    // 1. initialize primitive info
    std::vector<SHittableInfo>     hittableInfo(m_hittables.size());
    for (size_t i = 0; i < hittableInfo.size(); i++)
    {
        hittableInfo[i] = { i, m_hittables[i]->m_aabb };
    };

    // 2. build BVH tree
    int     totalNodes = 0;
    std::vector<std::shared_ptr<IHittable>>     orderedHittables;
    orderedHittables.reserve(m_hittables.size());

    SBVHBuildNode   *root = _RecursiveBuild(hittableInfo, 0, m_hittables.size(), &totalNodes, orderedHittables);
    m_hittables.swap(orderedHittables);
    hittableInfo.resize(0);
    
    // 3. compute representation of depth-first traversal
    m_nodes = new SLinearBVHNode[totalNodes];
    int offset = 0;
    _FlattenBVHTree(root, &offset);

    if (this->IsEmpty())
    {
        printf("[BVH] Error: Failed to construct bvh-tree.\n");
        return false;
    }
    
    printf("[BVH] Done.\n");
    return true;
}

//----------------------------------------------------

CBVHAccel::SBVHBuildNode*   CBVHAccel::_RecursiveBuild(std::vector<SHittableInfo> &bvHHittableInfo, int start, int end, int *totalNodes, std::vector<std::shared_ptr<IHittable>> &orderedHittables)
{
    // create node
    SBVHBuildNode   *node = new SBVHBuildNode();
    (*totalNodes)++;
    
    // compute bounds for all hittables in BVH node
    CAABB   topBound;
    for (int i = start; i < end; i++) 
        topBound = topBound + bvHHittableInfo[i].bounds;
    
    int nHittables = end - start;

    if (nHittables == 1)
    {
        // create leaf node

        int firstPrimOffset = orderedHittables.size();
        for (int i = start; i < end; i++)
        {
            int hittableNum = bvHHittableInfo[i].hittableNum;
            orderedHittables.push_back(m_hittables[hittableNum]);
        }
        node->InitLeaf(firstPrimOffset, nHittables, topBound);

        return node;
    }
    else
    {
        // compute bound of hittable centroids, choose split dimension dim
        // the split axis is chosen by axis with the largest extent
        CAABB   centroidBounds;
        for (int i = start; i < end; i++)
        {
            centroidBounds = centroidBounds + bvHHittableInfo[i].centroid;
        }
        int dim = centroidBounds.MaxExtent();

        // partition hittables into two sets and build children
        int mid = (start + end) / 2;
        if (centroidBounds.pMax[dim] == centroidBounds.pMin[dim]) {
            // create leaf node
            int firstHittableOffset = orderedHittables.size();
            for (int i = start; i < end; i++)
            {
                int hittableNum = bvHHittableInfo[i].hittableNum;
                orderedHittables.push_back(m_hittables[hittableNum]);
            }
            node->InitLeaf(firstHittableOffset, nHittables, topBound);
            return node;
        }
        else 
        {
            // partition by method
            switch (m_partitionMethod) 
            {
                // partition hittables using midpoints
            case MIDPOINT:
            {
                float pmid = (centroidBounds.pMin[dim] + centroidBounds.pMax[dim]) / 2;
                SHittableInfo *midPtr =
                    std::partition(&bvHHittableInfo[start], &bvHHittableInfo[end - 1] + 1, [dim, pmid](const SHittableInfo &pi) { return pi.centroid[dim] < pmid; });
                mid = midPtr - &bvHHittableInfo[0];

                // if there is too many overlapping boxes, it may fail to construct.
                // in that case, we split using equally subset method.
                if (mid != start && mid != end) break;
            }
            case EQUALSUBSET:
            {
                mid = (start + end) / 2;
                std::nth_element(&bvHHittableInfo[start], &bvHHittableInfo[mid], &bvHHittableInfo[end - 1] + 1,
                    [dim](const SHittableInfo &a, const SHittableInfo &b) { return a.centroid[dim] < b.centroid[dim]; }
                );
                break;
            }
            case SAH:
            default:
            {
                if (nHittables <= 2)
                {
                    // partition primitives into equally sized subsets
                    mid = (start + end) / 2;
                    std::nth_element(&bvHHittableInfo[start], &bvHHittableInfo[mid], &bvHHittableInfo[end - 1] + 1,
                        [dim](const SHittableInfo &a, const SHittableInfo &b) { return a.centroid[dim] < b.centroid[dim]; });
                }
                else 
                {
                    // allocate BucketInfo for SAH partition buckets
                    constexpr int nBuckets = 12;
                    SBucketInfo buckets[nBuckets];

                    // init. BucketInfo for SAH partition buckets
                    for (int i = start; i < end; i++)
                    {
                        int b = nBuckets * centroidBounds.Offset(bvHHittableInfo[i].centroid)[dim];
                        
                        if (b == nBuckets)
                            b = nBuckets - 1;
                        buckets[b].count++;
                        buckets[b].bounds = buckets[b].bounds + bvHHittableInfo[i].bounds;
                    }

                    // Compute costs for splitting after each bucket
                    float   cost[nBuckets - 1];
                    for (int i = 0; i < nBuckets - 1; i++)
                    {
                        CAABB b0, b1;
                        int count0 = 0, count1 = 0;
                        for (int j = 0; j <= i; j++)
                        {
                            b0 = b0 + buckets[j].bounds;
                            count0 += buckets[j].count;
                        }
                        for (int j = i + 1; j < nBuckets; j++)
                        {
                            b1 = b1 + buckets[j].bounds;
                            count1 += buckets[j].count;
                        }
                        cost[i] = 1 + (count0 * b0.SurfaceArea() + count1 * b1.SurfaceArea()) / topBound.SurfaceArea();
                    }

                    // Find bucket to split at that minimizes SAH metric
                    float   minCost = cost[0];
                    int     minCostSplitBucket = 0;
                    for (int i = 1; i < nBuckets - 1; i++)
                    {
                        if(cost[i] < minCost)
                        {
                            minCost = cost[i];
                            minCostSplitBucket = i;
                        }
                    }

                    // Either create leaf or split primitives at selected SAH bucket
                    float   leafCost = nHittables;
                    if (nHittables > m_maxHittablesInNode || minCost < leafCost)
                    {
                        SHittableInfo *pmid = std::partition(&bvHHittableInfo[start], &bvHHittableInfo[end - 1] + 1, 
                            [=](const SHittableInfo &pi) {
                                int b = nBuckets * centroidBounds.Offset(pi.centroid)[dim]; 
                                if (b == nBuckets)
                                    b = nBuckets - 1;
                                return b <= minCostSplitBucket; });
                        mid = pmid - &bvHHittableInfo[0];
                    }
                    // create leaf BVHBuild node
                    else 
                    {
                        int     firstPrimOffset = orderedHittables.size();
                        for (int i = start; i < end; i++)
                        {
                            int hittableNum = bvHHittableInfo[i].hittableNum;
                            orderedHittables.push_back(m_hittables[hittableNum]);
                        }
                        node->InitLeaf(firstPrimOffset, nHittables, topBound);
                        return node;
                    }
                }
                break;
            }}

            // build nodes
            node->InitInterior(dim,
                               _RecursiveBuild(bvHHittableInfo, start, mid, totalNodes, orderedHittables),
                               _RecursiveBuild(bvHHittableInfo, mid, end, totalNodes, orderedHittables));
        }
    }

    return node;
};

//----------------------------------------------------

// this method converts BVH tree into compact structure
int CBVHAccel::_FlattenBVHTree(SBVHBuildNode *node, int *offset)
{
    SLinearBVHNode  *linearNode = &m_nodes[*offset];
    linearNode->bounds = node->bounds;
    int             myOffset = (*offset)++;

    if (node->nHittables > 0)
    {
        linearNode->hittablesOffset = node->firstHittableOffset;
        linearNode->nHittables = node->nHittables;
    }
    else
    {
        // create interior flattened BVH node
        linearNode->axis = node->splitAxis;
        linearNode->nHittables = 0;
        _FlattenBVHTree(node->children[0], offset);
        linearNode->secondChildOffset = _FlattenBVHTree(node->children[1], offset);
    }

    return myOffset;
}

//----------------------------------------------------
_CD_NAMESPACE_END
