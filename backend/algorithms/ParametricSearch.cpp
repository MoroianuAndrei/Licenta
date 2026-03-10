#include "ParametricSearch.h"
#include <algorithm>
#include <set>
#include <cmath>
#include <limits>

// ============================================================================
// Helper: findZmin - MCP in G original
// ============================================================================
double ParametricSearch::findZmin(const Graph& G)
{
    int n = G.getNodesCount();
    std::vector<double> cap(n + 1, -1.0);
    std::vector<bool>   vis(n + 1, false);
    cap[G.getSource()] = std::numeric_limits<double>::infinity();

    for (int iter = 0; iter < n; ++iter)
    {
        int u = -1; double mx = -1.0;
        for (int j = 1; j <= n; ++j)
            if (!vis[j] && cap[j] > mx) { mx = cap[j]; u = j; }
        if (u == -1 || u == G.getTarget()) break;
        vis[u] = true;
        for (const Edge& e : G.getEdgesFrom(u))
        {
            if (e.isNew()) continue;
            int v = e.getTo();
            if (!vis[v]) { double pc = std::min(cap[u], e.getCapacity()); if (pc > cap[v]) cap[v] = pc; }
        }
    }
    return cap[G.getTarget()];
}

// ============================================================================
// Helper: findZmax - MCP in G_hat
// ============================================================================
double ParametricSearch::findZmax(const Graph& G)
{
    int n = G.getNodesCount();
    std::vector<double> cap(n + 1, -1.0);
    std::vector<bool>   vis(n + 1, false);
    cap[G.getSource()] = std::numeric_limits<double>::infinity();

    for (int iter = 0; iter < n; ++iter)
    {
        int u = -1; double mx = -1.0;
        for (int j = 1; j <= n; ++j)
            if (!vis[j] && cap[j] > mx) { mx = cap[j]; u = j; }
        if (u == -1 || u == G.getTarget()) break;
        vis[u] = true;
        for (const Edge& e : G.getEdgesFrom(u))
        {
            int v = e.getTo();
            if (!vis[v]) { double pc = std::min(cap[u], e.getMaxCapacity()); if (pc > cap[v]) cap[v] = pc; }
        }
    }
    return cap[G.getTarget()];
}

// ============================================================================
// Helper: lista sortata de valori critice z1 < z2 < ... < zl
// U{c(w)} U U{c_hat(w)} U {zmax}
// ============================================================================
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

// ============================================================================
// Algorithm 4 - MCPEBCP sub costuri liniare
// ============================================================================
ParametricResult ParametricSearch::solve(const Graph& G, double W)
{
    int n = G.getNodesCount();

    double zmin = findZmin(G);
    double zmax = findZmax(G);

    if (zmin < 0 || zmax < 0) return { 0.0, 0.0, {} };
    if (zmin >= zmax)          return { zmin, 0.0, {} };

    // ========================================================================
    // FAZA I - Pasii 4-17 din Algorithm 4
    // Binary search pe lista valorilor critice z1 < z2 < ... < zl
    // Gasim k astfel incat z* in [Z[k], Z[k+1])
    // ========================================================================
    std::vector<double> Z = buildSortedCriticalValues(G, zmax);
    int l = (int)Z.size();

    int L = 0, U = l - 1;

    // Pasul 7: while U - L > 1 do
    while (U - L > 1)
    {
        // Pasul 8: k = floor((L + U) / 2)
        int k = (L + U) / 2;

        // Pasii 9-10: Call AMCMCPEP pentru Z[k]
        AMCMCPEP_Result res = oracle.solve(G, Z[k]);

        // Pasii 11-15: if satisfies budget constraint
        if (res.isFeasible && res.totalCost <= W) 
            L = k;
        else                                
            U = k;
    }

    // Pasul 17: k = L => intervalul optim este [Z[L], Z[U]) = [zk, zk+1)
    double zk = Z[L];
    double zk1 = Z[U];

    // ========================================================================
    // FAZA II - Pasii 18-42 din Algorithm 4
    // Megiddo parametric search in intervalul [zk, zk+1)
    // S[i] = functii liniare de forma f(z) = r*z + q (nu scalare!)
    // ========================================================================

    // Pasii 19-26: Initializare (analog cu Algorithm 1)
    std::vector<LinearFunc> S(n + 1, LinearFunc::Infinity());
    std::vector<int>        parent(n + 1, -1);
    std::vector<bool>       permanent(n + 1, false);

    S[G.getSource()] = LinearFunc(0.0, 0.0); // S(s) = 0

    double L_val = zk;
    double R_val = zk1;

    // Pasul 27: while R(t) = 0 do
    // Conform paper-ului: bucla continua cat timp nodul target
    // nu a devenit inca permanent (R(t) = 0)
    while (!permanent[G.getTarget()])
    {
        // ----------------------------------------------------------------
        // Pasul 28: Find i in V so that R(i) = 0 and i has the smallest S(i)
        // "Tournament of Lines" - comparam functii liniare in [L_val, R_val]
        // ----------------------------------------------------------------
        int u = -1;
        for (int i = 1; i <= n; ++i)
        {
            if (permanent[i] || S[i].isInfinity()) continue;

            if (u == -1) 
            {
                u = i;
                continue;
            }

            // Comparam S[u] cu S[i] ca functii liniare
            LinearFunc diff = S[u] - S[i]; // diff(z) = S[u](z) - S[i](z)

            if (std::abs(diff.r) < 1e-9)
            {
                // Linii paralele - comparam direct in L_val
                if (S[i].eval(L_val) < S[u].eval(L_val)) 
                    u = i;
            }
            else
            {
                // Punctul de intersectie: diff(z) = 0 => z_int = -q/r
                double z_int = -diff.q / diff.r;

                if (z_int <= L_val || z_int >= R_val)
                {
                    // Intersectia e in afara intervalului - comparam in L_val
                    if (S[i].eval(L_val) < S[u].eval(L_val)) 
                        u = i;
                }
                else
                {
                    // Intersectie relevanta in (L_val, R_val)
                    // Pasul 34: apelam oracolul AMCMCPEP la z_int
                    AMCMCPEP_Result res = oracle.solve(G, z_int);

                    if (res.isFeasible && res.totalCost <= W)
                    {
                        // z* este la dreapta intersectiei => L_val = z_int
                        L_val = z_int;
                        if (S[i].eval(L_val + 1e-9) < S[u].eval(L_val + 1e-9)) 
                            u = i;
                    }
                    else
                    {
                        // z* este la stanga intersectiei => R_val = z_int
                        R_val = z_int;
                        if (S[i].eval(R_val - 1e-9) < S[u].eval(R_val - 1e-9)) 
                            u = i;
                    }
                }
            }
        }

        // Daca nu mai exista niciun nod accesibil, iesim din bucla
        if (u == -1) break;

        // Pasul 29: R(u) = 1 — marcam nodul u ca permanent
        permanent[u] = true;

        // ----------------------------------------------------------------
        // Pasii 30-41: pentru toate arcele w=(u,j) din A^E cu R(j) = 0
        // Relaxare cu functii liniare de cost
        // ----------------------------------------------------------------
        for (const Edge& e : G.getEdgesFrom(u))
        {
            int v = e.getTo();

            // Pasul 30: for all w=(u,j) in A^E so that R(j) = 0
            if (permanent[v]) continue;

            // Conditia Ecuatiei (29) — arcul apartine lui A_bar:
            // c(w) <= zk AND c_hat(w) >= zk+1
            // Doar aceste arce au cost liniar variabil in intervalul [zk, zk+1)
            if (e.getCapacity() > zk + 1e-9) continue;
            if (e.getMaxCapacity() < zk1 - 1e-9) continue;

            // Pasul 32: S(j) in forma u - v*z (functie liniara)
            // Costul arcului ca functie liniara: b(w,z) = r_w*(z - c(w))
            //                                           = r_w*z - r_w*c(w)
            double     r = e.getLinearRate();
            double     base = -r * e.getCapacity(); // -r_w * c(w)
            LinearFunc edgeCost(r, base);               // f(z) = r_w*z - r_w*c(w)
            LinearFunc newS = S[u] + edgeCost;

            // Pasii 37-40: actualizam S(j) daca noul cost e mai mic
            if (S[v].isInfinity() || newS.eval(L_val) < S[v].eval(L_val))
            {
                S[v] = newS;
                parent[v] = u;
            }
        }
    }

    // ========================================================================
    // Pasii 43-44: Run Lines 33-44 of Algorithm 1
    // Rezolvam S(t)(z) = W pentru a gasi z* exact
    // S(t) este o functie liniara => r*z + q = W => z* = (W - q) / r
    // ========================================================================
    LinearFunc finalFunc = S[G.getTarget()];
    double optimal_z = zk;

    if (!finalFunc.isInfinity())
    {
        if (std::abs(finalFunc.r) > 1e-9)
            // S(t)(z*) = W => z* = (W - q) / r
            optimal_z = (W - finalFunc.q) / finalFunc.r;
        else
            // Cost constant (r=0) - luam maximul posibil din interval
            optimal_z = R_val;

        // Incadram in intervalul [L_val, R_val]
        if (optimal_z < L_val) 
            optimal_z = L_val;

        if (optimal_z > R_val) 
            optimal_z = R_val;
    }

    double finalCost = finalFunc.isInfinity() ? 0.0 : finalFunc.eval(optimal_z);

    // Reconstructia drumului P*
    std::vector<int> path;
    int curr = G.getTarget();
    while (curr != -1 && curr != G.getSource())
    {
        path.push_back(curr);
        curr = parent[curr];
    }
    if (curr == G.getSource()) path.push_back(G.getSource());
    std::reverse(path.begin(), path.end());

    return { optimal_z, finalCost, path };
}