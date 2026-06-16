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

std::vector<int> AMCMCPEP::reconstructPath(int source, int target, const std::vector<int>& parent)
{
    std::vector<int> path;
    int j = target;

    while (j != source && j != -1)
    {
        path.push_back(j);
        j = parent[j];
    }

    if (j == source)
        path.push_back(source);

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

    std::vector<double> maxCost(n + 1, std::numeric_limits<double>::infinity());
    std::vector<double> sumCost(n + 1, std::numeric_limits<double>::infinity());
    std::vector<bool> permanent(n + 1, false);
    std::vector<int> parent(n + 1, -1);

    maxCost[G.getSource()] = 0.0;
    sumCost[G.getSource()] = 0.0;

    while (!permanent[G.getTarget()])
    {
        int u = -1;
        double minVal = std::numeric_limits<double>::infinity();
        for (int i = 1; i <= n; ++i)
            if (!permanent[i] && (maxCost[i] + sumCost[i]) < minVal)
            {
                minVal = maxCost[i] + sumCost[i];
                u = i;
            }

        if (u == -1)
            return { false, 0.0, {} };

        permanent[u] = true;

        for (const Edge& e : G.getEdgesFrom(u))
        {
            int v = e.getTo();
            if (permanent[v]) continue;

            if (e.getMaxCapacity() < z) continue;

            double edgeMaxCost = (e.getCapacity() >= z) ? 0.0 : e.getMaxCost(z);
            double edgeSumCost = (e.getCapacity() >= z) ? 0.0 : e.getSumCost(z);

            double newMax = std::max(maxCost[u], edgeMaxCost);
            double newTotal = newMax + sumCost[u] + edgeSumCost;

            if (newTotal < maxCost[v] + sumCost[v])
            {
                maxCost[v] = newMax;
                sumCost[v] = sumCost[u] + edgeSumCost;
                parent[v] = u;
            }
        }
    }

    std::vector<int> path = reconstructPath(G.getSource(), G.getTarget(), parent);

    return { true, maxCost[G.getTarget()] + sumCost[G.getTarget()], path };
}
