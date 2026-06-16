#include "ParametricSearch.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

std::vector<double> ParametricSearch::buildSortedCriticalValues(
    const Graph& G,
    double zmin,
    double zmax)
{
    std::set<double> vals;

    vals.insert(zmin);
    vals.insert(zmax);

    for (const Edge& e : G.getAllEdges())
    {
        double c = e.getCapacity();
        double cmax = e.getMaxCapacity();

        if (c >= zmin - 1e-9 && c <= zmax + 1e-9)
            vals.insert(c);

        if (cmax < 1e15 && cmax >= zmin - 1e-9 && cmax <= zmax + 1e-9)
            vals.insert(cmax);
    }

    return std::vector<double>(vals.begin(), vals.end());
}


std::pair<double, double> ParametricSearch::phaseI(
    const Graph& G,
    double zmin,
    double zmax,
    double W)
{
    std::vector<double> Z = buildSortedCriticalValues(G, zmin, zmax);

    int L = 0;
    int U = static_cast<int>(Z.size()) - 1;

    while (U - L > 1)
    {
        int k = (L + U) / 2;

        AMCMCPEP_Result res = oracle.solve(G, Z[k]);
        ++oracleCalls_;

        if (res.isFeasible && res.totalCost <= W)
            L = k;
        else
            U = k;
    }

    return { Z[L], Z[U] };
}


ParametricResult ParametricSearch::phaseII(
    const Graph& G,
    double zk,
    double zk1,
    double W)
{
    const int n = G.getNodesCount();

    std::vector<LinearFunc> S(n + 1, LinearFunc::Infinity());
    std::vector<int> parent(n + 1, -1);
    std::vector<bool> permanent(n + 1, false);

    S[G.getSource()] = LinearFunc(0.0, 0.0);

    double L_val = zk;
    double R_val = zk1;

    while (!permanent[G.getTarget()])
    {
        int u = -1;

        for (int i = 1; i <= n; ++i)
        {
            if (permanent[i] || S[i].isInfinity())
                continue;

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
                    ++oracleCalls_;

                    if (res.isFeasible && res.totalCost <= W)
                        L_val = z_int;
                    else
                        R_val = z_int;

                    if (S[i].eval(L_val + 1e-9) < S[u].eval(L_val + 1e-9))
                        u = i;
                }
            }
        }

        if (u == -1)
            break;

        permanent[u] = true;

        for (const Edge& e : G.getEdgesFrom(u))
        {
            int v = e.getTo();

            if (permanent[v])
                continue;

            if (e.getMaxCapacity() < zk1 - 1e-9)
                continue;

            LinearFunc edgeCost;
            double c_edge = e.getCapacity();

            if (c_edge >= zk1 - 1e-9)
            {
                edgeCost = LinearFunc(0.0, 0.0);
            }
            else if (c_edge <= zk + 1e-9)
            {
                double r = e.getLinearRate();
                edgeCost = LinearFunc(r, -r * c_edge);
            }
            else
            {
                continue;
            }

            LinearFunc newS = S[u] + edgeCost;

            if (S[v].isInfinity())
            {
                S[v] = newS;
                parent[v] = u;
            }
            else
            {
                LinearFunc diff = S[v] - newS;

                if (std::abs(diff.r) < 1e-9)
                {
                    if (newS.eval(L_val) < S[v].eval(L_val))
                    {
                        S[v] = newS;
                        parent[v] = u;
                    }
                }
                else
                {
                    double z_crit = -diff.q / diff.r;

                    if (z_crit <= L_val || z_crit >= R_val)
                    {
                        if (newS.eval(L_val) < S[v].eval(L_val))
                        {
                            S[v] = newS;
                            parent[v] = u;
                        }
                    }
                    else
                    {
                        AMCMCPEP_Result res = oracle.solve(G, z_crit);
                        ++oracleCalls_;

                        if (res.isFeasible && res.totalCost <= W)
                            L_val = z_crit;
                        else
                            R_val = z_crit;

                        if (newS.eval(L_val) < S[v].eval(L_val))
                        {
                            S[v] = newS;
                            parent[v] = u;
                        }
                    }
                }
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

        optimal_z = std::max(L_val, std::min(R_val, optimal_z));
    }

    double finalCost = finalFunc.isInfinity() ? 0.0 : finalFunc.eval(optimal_z);

    std::vector<int> path;
    AMCMCPEP_Result finalRes = oracle.solve(G, optimal_z);
    ++oracleCalls_;

    if (finalRes.isFeasible && !finalRes.path.empty())
    {
        path = finalRes.path;
        finalCost = finalRes.totalCost;
    }

    return { optimal_z, finalCost, path, oracleCalls_ };
}


ParametricResult ParametricSearch::solve(const Graph& G, double W)
{
    oracleCalls_ = 0;

    double zmin = G.findMCP(false);
    double zmax = G.findMCP(true);

    if (zmax < 0)
        return { 0.0, 0.0, {} };

    if (zmin < 0)
        zmin = 0.0;

    if (zmin >= zmax)
    {
        auto [cap, path] = G.findMCPWithPath(false);

        if (!path.empty())
            return { zmin, 0.0, path };

        return { zmax, 0.0, G.findMCPWithPath(true).second };
    }

    auto [zk, zk1] = phaseI(G, zmin, zmax, W);

    return phaseII(G, zk, zk1, W);
}
