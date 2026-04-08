#pragma once
#include "graph.h"
#include <vector>

struct BruteForceResult
{
    bool feasible;
    double totalCost;
    std::vector<int> path;
};

class BruteForceValidator
{
private:
    double bestCost;
    std::vector<int> bestPath;

    void dfsAllPaths(
        const Graph& G,
        int current,
        int target,
        double z,
        std::vector<bool>& visited,
        std::vector<int>& currentPath
    );

    double computePathCost(const Graph& G, const std::vector<int>& path, double z) const;
    const Edge* findEdge(const Graph& G, int from, int to) const;

public:
    BruteForceResult solveAMCMCPEP(const Graph& G, double z);
};