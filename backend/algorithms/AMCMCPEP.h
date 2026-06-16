#pragma once
#include <vector>
#include "graph.h"

struct AMCMCPEP_Result
{
    bool isFeasible;
    double totalCost;
    std::vector<int> path;
};

class AMCMCPEP
{
private:
    std::vector<int> reconstructPath(int source, int target, const std::vector<int>& parent);

    bool isTargetReachable(const Graph& G, double z) const;

public:
    AMCMCPEP_Result solve(const Graph& G, double z);
};
