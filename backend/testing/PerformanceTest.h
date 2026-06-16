#pragma once

#include "backend/core/graph.h"
#include <vector>
#include <string>

struct BenchmarkEntry
{
    int nodes;
    int edges;
    double W;

    double amcpebcp_time_ms;
    int amcpebcp_oracle_calls;
    double amcpebcp_result_z;
    double amcpebcp_result_cost;

    double parametric_time_ms;
    int parametric_oracle_calls;
    double parametric_result_z;
    double parametric_result_cost;

    double z_difference;
};

class PerformanceTest
{
public:
    BenchmarkEntry runSingle(int nodes, double W, int maxCapacityLimit = 100, unsigned int seed = 42);

    void saveToCSV(const std::vector<BenchmarkEntry>& results, const std::string& filename);
};

