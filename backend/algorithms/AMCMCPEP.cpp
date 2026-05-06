#include "AMCMCPEP.h"
#include <limits>
#include <algorithm>
#include <queue>

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

AMCMCPEP_Result AMCMCPEP::solve(const Graph& G, double z)
{
    int n = G.getNodesCount();

    if (!isTargetReachable(G, z))
        return { false, 0.0, {} };

    auto mcpResult = G.findMCPWithPath(false);
    double mcpCap = mcpResult.first;

    if (mcpCap >= z)
        return { true, 0.0, mcpResult.second };

    M.assign(n + 1, std::numeric_limits<double>::infinity());
    S.assign(n + 1, std::numeric_limits<double>::infinity());
    R.assign(n + 1, 0);
    p.assign(n + 1, -1);

    M[G.getSource()] = 0.0;
    S[G.getSource()] = 0.0;

    while (R[G.getTarget()] == 0)
    {
        int u = -1;
        double minVal = std::numeric_limits<double>::infinity();
        for (int i = 1; i <= n; ++i)
            if (R[i] == 0 && (M[i] + S[i]) < minVal)
            {
                minVal = M[i] + S[i];
                u = i;
            }

        if (u == -1)
            return { false, 0.0, {} };

        R[u] = 1;

        for (const Edge& e : G.getEdgesFrom(u))
        {
            int v = e.getTo();
            if (R[v] != 0) continue;

            if (e.getMaxCapacity() < z) continue;

            double costU = (e.getCapacity() >= z) ? 0.0 : e.getMaxCost(z);
            double costB = (e.getCapacity() >= z) ? 0.0 : e.getSumCost(z);

            double newMax = std::max(M[u], costU);
            double newTotal = newMax + S[u] + costB;

            if (newTotal < M[v] + S[v])
            {
                M[v] = newMax;
                S[v] = S[u] + costB;
                p[v] = u;
            }
        }
    }

    std::vector<int> path = getPath(G.getSource(), G.getTarget());

    return { true, M[G.getTarget()] + S[G.getTarget()], path };
}