#pragma once
#include "graph.h"
#include "LinearFunc.h"
#include "AMCMCPEP.h"
#include <vector>

struct ParametricResult {
    double           maxCapacity;
    double           finalCost;
    std::vector<int> path;
};

class ParametricSearch {
private:
    AMCMCPEP oracle;

    std::vector<double> buildSortedCriticalValues(const Graph& G, double zmax);

public:
    ParametricResult solve(const Graph& G, double W);
};