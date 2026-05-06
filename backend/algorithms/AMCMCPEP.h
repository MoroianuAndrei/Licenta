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
    std::vector<double> M;
    std::vector<double> S;
    std::vector<int> R;
    std::vector<int> p;

    std::vector<int> getPath(int sourceNode, int targetNode);

    bool isTargetReachable(const Graph& G, double z) const;

public:
    AMCMCPEP_Result solve(const Graph& G, double z);
};