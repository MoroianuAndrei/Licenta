#pragma once
#include "graph.h"
#include "LinearFunc.h"
#include "AMCMCPEP.h"
#include <vector>

struct ParametricResult {
    double           maxCapacity; // z* optim
    double           finalCost;   // Costul final
    std::vector<int> path;        // Drumul P*
};

class ParametricSearch {
private:
    AMCMCPEP oracle;

    double findZmin(const Graph& G);
    double findZmax(const Graph& G);
    std::vector<double> buildSortedCriticalValues(const Graph& G, double zmax);

public:
    // Algorithm 4 - Megiddo parametric search, cazul liniar
    ParametricResult solve(const Graph& G, double W);
};