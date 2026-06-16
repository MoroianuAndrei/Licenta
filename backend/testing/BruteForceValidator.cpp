#include "BruteForceValidator.h"
#include <limits>
#include <algorithm>
#include <cmath>

const Edge* BruteForceValidator::findEdge(const Graph& G, int from, int to) const
{
    for (const Edge& e : G.getEdgesFrom(from))
    {
        if (e.getTo() == to)
            return &e;
    }
    return nullptr;
}

double BruteForceValidator::computePathCost(const Graph& G, const std::vector<int>& path, double z) const
{
    double maxU = 0.0;
    double sumB = 0.0;

    for (int i = 0; i + 1 < (int)path.size(); ++i)
    {
        int u = path[i];
        int v = path[i + 1];

        const Edge* e = findEdge(G, u, v);
        if (e == nullptr)
            return std::numeric_limits<double>::infinity();

        if (e->getMaxCapacity() < z)
            return std::numeric_limits<double>::infinity();

        double costU = (e->getCapacity() >= z) ? 0.0 : e->getMaxCost(z);
        double costB = (e->getCapacity() >= z) ? 0.0 : e->getSumCost(z);

        maxU = std::max(maxU, costU);
        sumB += costB;
    }

    return maxU + sumB;
}

void BruteForceValidator::dfsAllPaths(
    const Graph& G,
    int current,
    int target,
    double z,
    std::vector<bool>& visited,
    std::vector<int>& currentPath
)
{
    if (current == target)
    {
        double cost = computePathCost(G, currentPath, z);
        if (cost < bestCost_)
        {
            bestCost_ = cost;
            bestPath_ = currentPath;
        }
        return;
    }

    for (const Edge& e : G.getEdgesFrom(current))
    {
        int next = e.getTo();

        if (e.getMaxCapacity() < z)
            continue;

        if (visited[next])
            continue;

        visited[next] = true;
        currentPath.push_back(next);

        dfsAllPaths(G, next, target, z, visited, currentPath);

        currentPath.pop_back();
        visited[next] = false;
    }
}

BruteForceResult BruteForceValidator::solveAMCMCPEP(const Graph& G, double z)
{
    bestCost_ = std::numeric_limits<double>::infinity();
    bestPath_.clear();

    std::vector<bool> visited(G.getNodesCount() + 1, false);
    std::vector<int> currentPath;

    int s = G.getSource();
    int t = G.getTarget();

    visited[s] = true;
    currentPath.push_back(s);

    dfsAllPaths(G, s, t, z, visited, currentPath);

    if (bestPath_.empty())
        return { false, 0.0, {} };

    return { true, bestCost_, bestPath_ };
}
