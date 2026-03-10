#include "AMCPEBCP.h"
#include <algorithm>
#include <limits>
#include <cmath>

AMCPEBCP_Result AMCPEBCP::solve(const Graph& G, double W, double epsilon)
{
    // Pasul 2: zmin = MCP in G original (fara arce noi)
    double zmin = G.findMCP(false);
    // Pasul 4: zmax = MCP in G_hat (cu capacitatile maxime)
    double zmax = G.findMCP(true);

    if (zmin < 0 || zmax < 0) return { 0.0, 0.0, {} };
    if (std::abs(zmin - zmax) < epsilon) return { zmin, 0.0, {} };

    AMCMCPEP solver;

    // Verifica daca bugetul acopera direct zmax
    AMCMCPEP_Result resMax = solver.solve(G, zmax);
    if (resMax.isFeasible && resMax.totalCost <= W)
        return { zmax, resMax.totalCost, resMax.path };

    // Rezultatul initial: z = zmin, cost = 0
    double           bestZ = zmin;
    double           bestCost = 0.0;
    std::vector<int> bestPath;

    AMCMCPEP_Result resMin = solver.solve(G, zmin);
    if (resMin.isFeasible)
    {
        bestCost = resMin.totalCost;
        bestPath = resMin.path;
    }

    // Pasii 9-18: binary search
    while (zmax - zmin > epsilon)
    {
        double z = std::ceil((zmin + zmax) / 2.0);
        if (z <= zmin || z >= zmax)
            z = (zmin + zmax) / 2.0;

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