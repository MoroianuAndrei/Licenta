#include "AMCPEBCP.h"
#include <algorithm>
#include <limits>
#include <cmath>

double AMCPEBCP::findMCP(const Graph& G, bool useMaxCapacities)
{
    int n = G.getNodesCount();
    std::vector<double> cap(n + 1, -1.0);
    std::vector<bool>   visited(n + 1, false);

    cap[G.getSource()] = std::numeric_limits<double>::infinity();

    for (int iter = 0; iter < n; ++iter)
    {
        int u = -1; 
        double maxC = -1.0;
        for (int j = 1; j <= n; ++j)
            if (!visited[j] && cap[j] > maxC) 
            { 
                maxC = cap[j]; 
                u = j; 
            }

        if (u == -1 || u == G.getTarget()) break;
        visited[u] = true;

        for (const Edge& e : G.getEdgesFrom(u))
        {
            if (!useMaxCapacities && e.isNew()) continue;
            int    v = e.getTo();
            double edgeCap = useMaxCapacities ? e.getMaxCapacity() : e.getCapacity();

            if (!visited[v])
            {
                double pc = std::min(cap[u], edgeCap);
                if (pc > cap[v]) cap[v] = pc;
            }
        }
    }
    return cap[G.getTarget()];
}

AMCPEBCP_Result AMCPEBCP::solve(const Graph& G, double W, double epsilon)
{
    // Pasul 2: zmin = MCP in G
    double zmin = findMCP(G, false);
    // Pasul 4: zmax = MCP in G_hat
    double zmax = findMCP(G, true);

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