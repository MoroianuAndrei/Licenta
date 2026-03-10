#pragma once
#include "graph.h"
#include "AMCMCPEP.h"
#include <vector>

struct AMCPEBCP_Result {
    double           maxCapacity; // z* optim
    double           finalCost;   // Costul platit (<= W)
    std::vector<int> path;        // Drumul P*
};

class AMCPEBCP {
public:
    // Algorithm 3 - binary search pe [zmin, zmax]
    AMCPEBCP_Result solve(const Graph& G, double W, double epsilon = 1e-4);
};