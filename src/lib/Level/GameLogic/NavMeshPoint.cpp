#include "NavMeshPoint.h"

NavMeshPoint::NavMeshPoint()
    : name(""), position(cf_v2(0, 0)), polygon_index(-1)
{
}

NavMeshPoint::NavMeshPoint(const std::string &n, CF_V2 pos)
    : name(n), position(pos), polygon_index(-1)
{
}

NavMeshPoint::NavMeshPoint(const std::string &n, CF_V2 pos, int poly_idx)
    : name(n), position(pos), polygon_index(poly_idx)
{
}
