#include "ParametricSearch.h"
#include <algorithm>
#include <set>
#include <cmath>
#include <limits>

std::vector<double> ParametricSearch::buildSortedCriticalValues(const Graph& G, double zmax)
{
    std::set<double> vals;
    for (const Edge& e : G.getAllEdges())
    {
        vals.insert(e.getCapacity());
        if (e.getMaxCapacity() < 1e15) vals.insert(e.getMaxCapacity());
    }
    vals.insert(zmax);
    return std::vector<double>(vals.begin(), vals.end());
}

ParametricResult ParametricSearch::solve(const Graph& G, double W)
{
    int n = G.getNodesCount();

    double zmin = G.findMCP(false);
    double zmax = G.findMCP(true);

    if (zmin < 0 || zmax < 0) return { 0.0, 0.0, {} };
    if (zmin >= zmax)          return { zmin, 0.0, {} };

    std::vector<double> Z = buildSortedCriticalValues(G, zmax);
    int l = (int)Z.size();

    int L = 0, U = l - 1;

    while (U - L > 1)
    {
        int k = (L + U) / 2;

        AMCMCPEP_Result res = oracle.solve(G, Z[k]);

        if (res.isFeasible && res.totalCost <= W)
            L = k;
        else
            U = k;
    }

    double zk = Z[L];
    double zk1 = Z[U];

    std::vector<LinearFunc> S(n + 1, LinearFunc::Infinity());
    std::vector<int>        parent(n + 1, -1);
    std::vector<bool>       permanent(n + 1, false);

    S[G.getSource()] = LinearFunc(0.0, 0.0);

    double L_val = zk;
    double R_val = zk1;

    while (!permanent[G.getTarget()])
    {
        int u = -1;
        for (int i = 1; i <= n; ++i)
        {
            if (permanent[i] || S[i].isInfinity()) continue;

            if (u == -1)
            {
                u = i;
                continue;
            }

            LinearFunc diff = S[u] - S[i];

            if (std::abs(diff.r) < 1e-9)
            {
                if (S[i].eval(L_val) < S[u].eval(L_val))
                    u = i;
            }
            else
            {
                double z_int = -diff.q / diff.r;

                if (z_int <= L_val || z_int >= R_val)
                {
                    if (S[i].eval(L_val) < S[u].eval(L_val))
                        u = i;
                }
                else
                {
                    AMCMCPEP_Result res = oracle.solve(G, z_int);

                    if (res.isFeasible && res.totalCost <= W)
                    {
                        L_val = z_int;
                        if (S[i].eval(L_val + 1e-9) < S[u].eval(L_val + 1e-9))
                            u = i;
                    }
                    else
                    {
                        R_val = z_int;
                        if (S[i].eval(R_val - 1e-9) < S[u].eval(R_val - 1e-9))
                            u = i;
                    }
                }
            }
        }

        if (u == -1) break;

        permanent[u] = true;

        for (const Edge& e : G.getEdgesFrom(u))
        {
            int v = e.getTo();

            if (permanent[v]) continue;

            if (e.getCapacity() > zk + 1e-9) continue;
            if (e.getMaxCapacity() < zk1 - 1e-9) continue;

            double     r = e.getLinearRate();
            double     base = -r * e.getCapacity();
            LinearFunc edgeCost(r, base);
            LinearFunc newS = S[u] + edgeCost;

            if (S[v].isInfinity() || newS.eval(L_val) < S[v].eval(L_val))
            {
                S[v] = newS;
                parent[v] = u;
            }
        }
    }

    LinearFunc finalFunc = S[G.getTarget()];
    double optimal_z = zk;

    if (!finalFunc.isInfinity())
    {
        if (std::abs(finalFunc.r) > 1e-9)
            optimal_z = (W - finalFunc.q) / finalFunc.r;
        else
            optimal_z = R_val;

        if (optimal_z < L_val)
            optimal_z = L_val;

        if (optimal_z > R_val)
            optimal_z = R_val;
    }

    double finalCost = finalFunc.isInfinity() ? 0.0 : finalFunc.eval(optimal_z);

    std::vector<int> path;
    AMCMCPEP_Result finalRes = oracle.solve(G, optimal_z);

    if (finalRes.isFeasible && !finalRes.path.empty())
    {
        path = finalRes.path;
        finalCost = finalRes.totalCost;
    }

    return { optimal_z, finalCost, path };
}