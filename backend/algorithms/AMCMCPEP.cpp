#include "AMCMCPEP.h"
#include <limits>
#include <algorithm>
#include <queue>

// ============================================================================
// Pasul 3 - BFS: verifica daca t este accesibil din s in G^E
// ============================================================================
bool AMCMCPEP::isTargetReachable(const Graph& G, double z) const
{
    int n = G.getNodesCount();
    std::vector<bool> visited(n + 1, false);
    std::queue<int> q;

    q.push(G.getSource());
    visited[G.getSource()] = true;

    while (!q.empty())
    {
        int u = q.front();
        q.pop();

        if (u == G.getTarget())
            return true;

        for (const Edge& e : G.getEdgesFrom(u))
        {
            if (e.getMaxCapacity() < z) continue;

            int v = e.getTo();
            if (!visited[v])
            {
                visited[v] = true;
                q.push(v);
            }
        }
    }

    return false;
}


// ============================================================================
// Algorithm 2 (APF) - reconstruieste drumul din vectorul de predecesori
// ============================================================================
std::vector<int> AMCMCPEP::getPath(int sourceNode, int targetNode)
{
    std::vector<int> path;
    int j = targetNode;

    while (j != sourceNode && j != -1)
    {
        path.push_back(j);
        j = p[j];
    }

    if (j == sourceNode)
        path.push_back(sourceNode);

    std::reverse(path.begin(), path.end());

    return path;
}

// ============================================================================
// Algorithm 1 (AMCMCPEP) - complet, pasii 1-49
// ============================================================================
AMCMCPEP_Result AMCMCPEP::solve(const Graph& G, double z)
{
    int n = G.getNodesCount();

    // --- Pasii 3-6: fezabilitate ---
    if (!isTargetReachable(G, z))
        return { false, 0.0, {} };

    // --- Pasii 7-11: verifica daca G nu are deja nevoie de expansiune ---
    double mcpCap = G.findMCP(false); // Pasul 7: MCP in G original (fara arce noi)
    if (mcpCap >= z)
        return { true, 0.0, {} }; // Costul e 0, nu e nevoie de expansiune

    // --- Pasii 12-21: initializare ---
    M.assign(n + 1, std::numeric_limits<double>::infinity());
    S.assign(n + 1, std::numeric_limits<double>::infinity());
    R.assign(n + 1, 0);
    p.assign(n + 1, -1);

    M[G.getSource()] = 0.0;
    S[G.getSource()] = 0.0;

    // --- Pasii 22-32: bucla principala while R(t) = 0 ---
    while (R[G.getTarget()] == 0)
    {
        // Pasul 23: gasim i cu R(i)=0 si M(i)+S(i) minim
        int u = -1;
        double minVal = std::numeric_limits<double>::infinity();
        for (int i = 1; i <= n; ++i)
            if (R[i] == 0 && (M[i] + S[i]) < minVal)
            {
                minVal = M[i] + S[i];
                u = i;
            }

        if (u == -1)
            return { false, 0.0, {} }; // niciun nod accesibil

        R[u] = 1; // Pasul 24

        // Pasii 25-31: relaxare arce
        for (const Edge& e : G.getEdgesFrom(u))
        {
            int v = e.getTo();
            if (R[v] != 0) continue;

            // Filtru A^E: doar arcele cu c_hat(w) >= z
            if (e.getMaxCapacity() < z) continue;

            // Formula (4): cost 0 daca c(w) >= z
            double costU = (e.getCapacity() >= z) ? 0.0 : e.getMaxCost(z);
            double costB = (e.getCapacity() >= z) ? 0.0 : e.getSumCost(z);

            double newMax = std::max(M[u], costU);
            double newTotal = newMax + S[u] + costB;

            // Pasul 26: if max{M(u),u(w,z)}+S(u)+b(w,z) < M(v)+S(v)
            if (newTotal < M[v] + S[v])
            {
                M[v] = newMax;           // Pasul 27
                S[v] = S[u] + costB;     // Pasul 28
                p[v] = u;                // Pasul 29
            }
        }
    }

    // --- Pasul 33: APF(t) ---
    std::vector<int> path = getPath(G.getSource(), G.getTarget());

    return { true, M[G.getTarget()] + S[G.getTarget()], path };
}