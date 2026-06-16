#pragma once
#include "graph.h"
#include "AMCMCPEP.h"
#include <vector>

struct AMCPEBCP_Result
{
    double maxCapacity;
    double finalCost;
    std::vector<int> path;
    int oracleCalls = 0;
};

class AMCPEBCP
{
public:
    AMCPEBCP_Result solve(const Graph& G, double W);
};
