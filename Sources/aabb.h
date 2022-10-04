#pragma once

#include "common.h"
#include "ray.h"

#include <math.h>

_CD_NAMESPACE_BEGIN
//----------------------------------------------------

class CAABB
{
public:
    CAABB()
    : CAABB(glm::vec3(std::numeric_limits<float>::max()), glm::vec3(std::numeric_limits<float>::lowest()))
    {
    }
    CAABB(const glm::vec3 &p)
    : CAABB(p, p)
    {
    }
    CAABB(const glm::vec3 &pMin_, const glm::vec3 &pMax_)
    : pMin(pMin_)
    , pMax(pMax_)
    {
    }

    //----------------------------------------------------
    // accessors
    //
    inline glm::vec3   Centroid() const         { return (pMin + pMax) * 0.5f; }
    inline glm::vec3   Corner(int corner) const { return glm::vec3((*this)[corner & 1].x, (*this)[(corner & 2) ? 1 : 0].y, (*this)[(corner & 4) ? 1 : 0].z); }
    inline glm::vec3   Diagonal() const         { return pMax - pMin; }
    inline float       SurfaceArea() const      { glm::vec3 d = Diagonal(); return 2 * (d.x * d.y + d.x * d.z + d.y * d.z); }
    inline float       Volume() const           { glm::vec3 d = Diagonal(); return d.x * d.y * d.z; }
    inline int	       MaxExtent() const
    {
        glm::vec3 d = Diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    };

    //----------------------------------------------------
    // ray intersection
    //
    bool    Hit(const CRay& r, float &t) const
    {
        const glm::vec3     sign(r.m_dir.x < 0, r.m_dir.y < 0, r.m_dir.z < 0);
        const glm::vec3     orig = r.m_origin;
        const glm::vec3     invDir = 1.f / r.m_dir;

        float tmin, tmax, tymin, tymax, tzmin, tzmax;
        tmin  = ((*this)[(bool)sign[0]].x - orig.x) * invDir.x;
        tmax  = ((*this)[1 - (bool)sign[0]].x - orig.x) * invDir.x;
        tymin = ((*this)[(bool)sign[1]].y - orig.y) * invDir.y;
        tymax = ((*this)[1 - (bool)sign[1]].y - orig.y) * invDir.y;

        if ((tmin > tymax) || (tymin > tmax))
            return false;

        if (tymin > tmin)
            tmin = tymin;
        if (tymax < tmax)
            tmax = tymax;

        tzmin = ((*this)[(bool)sign[2]].z - orig.z) * invDir.z;
        tzmax = ((*this)[1 - (bool)sign[2]].z - orig.z) * invDir.z;

        if ((tmin > tzmax) || (tzmin > tmax))
            return false;

        if (tzmin > tmin)
            tmin = tzmin;
        if (tzmax < tmax)
            tmax = tzmax;

        t = tmin;

        return true;
    };

    inline bool    Hit(const CRay& r) const { float f; return Hit(r, f); }

    //----------------------------------------------------
    // methods
    //
    inline bool    Overlaps(const CAABB &b) const
    {
        bool x = (this->pMax.x >= b.pMin.x) && (this->pMin.x <= b.pMax.x);
        bool y = (this->pMax.y >= b.pMin.y) && (this->pMin.x <= b.pMax.y);
        bool z = (this->pMax.z >= b.pMin.z) && (this->pMin.x <= b.pMax.z);

        return (x && y && z);
    }
    inline bool    IsInside(const glm::vec3 &p) const
    {
        return (p.x >= this->pMin.x && p.x <= this->pMax.x &&
                p.y >= this->pMin.y && p.y <= this->pMax.y &&
                p.z >= this->pMin.z && p.z <= this->pMax.z);
    }
    inline bool    IsInsideExclusive(const glm::vec3 &p) const
    {
        return (p.x >= this->pMin.x && p.x < this->pMax.x &&
                p.y >= this->pMin.y && p.y < this->pMax.y &&
                p.z >= this->pMin.z && p.z < this->pMax.z);
    };
    glm::vec3   Offset(const glm::vec3 &p) const
    {
        glm::vec3 o = p - pMin;
        if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
        if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
        if (pMax.z > pMin.z) o.z /= pMax.z - pMin.z;

        return o;
    }

    inline CAABB   Expand(float ds) { return CAABB(pMin - glm::vec3(ds), pMax + glm::vec3(ds)); }

    //----------------------------------------------------
    // operators
    //
    // access pMin/pMax by index
    inline const glm::vec3&    operator[] (int i) const { return i > 0 ? pMax : pMin; }
    inline glm::vec3&          operator[] (int i) { return i > 0 ? pMax : pMin; }

    // union(expand) with input point
    CAABB   operator+ (const glm::vec3 &p)
    {
        glm::vec3 newMin(std::min(this->pMin.x, p.x),
                         std::min(this->pMin.y, p.y),
                         std::min(this->pMin.z, p.z));
        glm::vec3 newMax(std::max(this->pMax.x, p.x),
                         std::max(this->pMax.y, p.y),
                         std::max(this->pMax.z, p.z));

        return CAABB(newMin, newMax);
    }
    // union(expand) with another bounding box
    CAABB   operator+ (const CAABB &b)
    {
        glm::vec3 newMin(std::min(this->pMin.x, b.pMin.x),
                         std::min(this->pMin.y, b.pMin.y),
                         std::min(this->pMin.z, b.pMin.z));
        glm::vec3 newMax(std::max(this->pMax.x, b.pMax.x),
                         std::max(this->pMax.y, b.pMax.y),
                         std::max(this->pMax.z, b.pMax.z));

        return CAABB(newMin, newMax);
    }
    CAABB   operator- (const CAABB &b)
    {
        glm::vec3 newMin(std::max(this->pMin.x, b.pMin.x),
                         std::max(this->pMin.y, b.pMin.y),
                         std::max(this->pMin.z, b.pMin.z));
        glm::vec3 newMax(std::min(this->pMax.x, b.pMax.x),
                         std::min(this->pMax.y, b.pMax.y),
                         std::min(this->pMax.z, b.pMax.z));

        return CAABB(newMin, newMax);
    }

public:
    // member variables
    glm::vec3   pMin, pMax;
};

//----------------------------------------------------
_CD_NAMESPACE_END
