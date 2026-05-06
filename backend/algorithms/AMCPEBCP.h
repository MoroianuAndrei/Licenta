#pragma once
#include "graph.h"
#include "AMCMCPEP.h"
#include <vector>

struct AMCPEBCP_Result {
    int           maxCapacity;
    double           finalCost;
    std::vector<int> path;
};

class AMCPEBCP {
public:
    AMCPEBCP_Result solve(const Graph& G, double W, double epsilon = 1e-4);
};