#include "AMCPEBCP.h"
#include <algorithm>
#include <limits>
#include <cmath>


AMCPEBCP_Result AMCPEBCP::solve(const Graph& G, double W)
{
    int zmin = (int)G.findMCP(false);
    int zmax = (int)G.findMCP(true);


    if (zmax < 0) return { 0.0, 0.0, {} };


    if (zmin < 0) zmin = 0;


    if (zmin >= zmax)
    {
        auto [cap, path] = G.findMCPWithPath(false);
        if (!path.empty())
            return { (double)zmin, 0.0, path };


        return { (double)zmax, 0.0, G.findMCPWithPath(true).second };
    }

    AMCMCPEP solver;

    AMCMCPEP_Result resMax = solver.solve(G, zmax);
    if (resMax.isFeasible && resMax.totalCost <= W)
        return { (double)zmax, resMax.totalCost, resMax.path };

    double bestZ = (double)zmin;
    double bestCost = 0.0;
    std::vector<int> bestPath;

    AMCMCPEP_Result resMin = solver.solve(G, zmin);
    if (resMin.isFeasible)
    {
        bestCost = resMin.totalCost;
        bestPath = resMin.path;
    }

    while (zmax - zmin > 1)
    {
        int z = (int)std::ceil((zmin + zmax) / 2.0);

        AMCMCPEP_Result res = solver.solve(G, z);

        if (res.isFeasible && res.totalCost <= W)
        {
            zmin = z;
            bestZ = z;
            bestCost = res.totalCost;
            bestPath = res.path;
        }
        else
        {
            zmax = z;
        }
    }
    return { bestZ, bestCost, bestPath };
}
