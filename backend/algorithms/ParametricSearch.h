#pragma once
#include "graph.h"
#include "LinearFunc.h"
#include "AMCMCPEP.h"
#include <vector>

struct ParametricResult
{
    double maxCapacity;
    double finalCost;
    std::vector<int> path;
};

class ParametricSearch
{
public:
    ParametricResult solve(const Graph& G, double W);

private:
    AMCMCPEP oracle;

    std::vector<double> buildSortedCriticalValues(const Graph& G, double zmin, double zmax);


    std::pair<double, double> phaseI(const Graph& G, double zmin, double zmax, double W);


    ParametricResult phaseII(const Graph& G, double zk, double zk1, double W);
};

