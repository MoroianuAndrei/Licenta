#pragma once
#include "graph.h"
#include "AMCMCPEP.h"
#include <vector>

struct AMCPEBCP_Result
{
    double maxCapacity;

    double finalCost;
    std::vector<int> path;
};

class AMCPEBCP
{
public:
    AMCPEBCP_Result solve(const Graph& G, double W);
};
