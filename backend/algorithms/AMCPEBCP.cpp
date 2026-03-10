#include "AMCPEBCP.h"
#include <algorithm>
#include <limits>
#include <cmath>

AMCPEBCP_Result AMCPEBCP::solve(const Graph& G, double W, double epsilon)
{
    // Pasul 2: zmin = MCP in G original (fara arce noi)
    int zmin = (int)G.findMCP(false);
    // Pasul 4: zmax = MCP in G_hat (cu capacitatile maxime)
    int zmax = (int)G.findMCP(true);

    if (zmin < 0 || zmax < 0) return { 0, 0.0, {} };
    if (zmin >= zmax)          return { zmin, 0.0, {} };

    AMCMCPEP solver;

    // Verifica daca bugetul acopera direct zmax
    AMCMCPEP_Result resMax = solver.solve(G, zmax);
    if (resMax.isFeasible && resMax.totalCost <= W)
        return { zmax, resMax.totalCost, resMax.path };

    // Rezultatul initial: z = zmin, cost = 0
    int              bestZ = zmin;
    double           bestCost = 0.0;
    std::vector<int> bestPath;

    AMCMCPEP_Result resMin = solver.solve(G, zmin);
    if (resMin.isFeasible)
    {
        bestCost = resMin.totalCost;
        bestPath = resMin.path;
    }

    // Pasii 9-18: binary search pe valori intregi
    // Conditia e zmax - zmin > 1 (nu epsilon) deoarece lucram cu intregi
    while (zmax - zmin > 1)
    {
        // Pasul 10: z = ceil((zmin + zmax) / 2) — intotdeauna intreg
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