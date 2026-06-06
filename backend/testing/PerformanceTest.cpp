#include "PerformanceTest.h"
#include "backend/algorithms/AMCPEBCP.h"
#include "backend/algorithms/ParametricSearch.h"
#include "backend/generation/RandomGraphGenerator.h"
#include "backend/io/GraphLoader.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>

static Graph generateLinearGraph(int nodes, int maxCapacityLimit, unsigned int seed)
{
    RandomGraphParams params;
    params.nodes              = nodes;
    params.paths              = std::max(2, nodes / 5);
    params.cycles             = std::max(1, nodes / 10);
    params.extraArcProbability = 0.05;
    params.minCapacity        = 1;
    params.maxCapacity        = std::max(2, maxCapacityLimit / 3);
    params.maxCapacityLimit   = maxCapacityLimit;
    params.minRate            = 1;
    params.maxRate            = 10;
    params.newArcProbability  = 0.10;
    params.seed               = seed;

    const std::string tmpFile = "benchmark_tmp.csv";

    RandomGraphGenerator gen(params);
    gen.generateToCSV(tmpFile);

    Graph G = loadGraphFromCSV(tmpFile, CostModel::Linear);
    std::remove(tmpFile.c_str());

    return G;
}

BenchmarkEntry PerformanceTest::runSingle(int nodes, double W, int maxCapacityLimit, unsigned int seed)
{
    BenchmarkEntry entry{};
    entry.nodes = nodes;
    entry.W     = W;

    Graph G   = generateLinearGraph(nodes, maxCapacityLimit, seed);
    entry.edges = (int)G.getAllEdges().size();

    {
        auto t0 = std::chrono::high_resolution_clock::now();

        AMCPEBCP solver;
        AMCPEBCP_Result res = solver.solve(G, W);

        auto t1 = std::chrono::high_resolution_clock::now();

        entry.amcpebcp_time_ms     = std::chrono::duration<double, std::milli>(t1 - t0).count();
        entry.amcpebcp_result_z    = res.maxCapacity;
        entry.amcpebcp_result_cost = res.finalCost;

        double zmin = G.findMCP(false);
        double zmax = G.findMCP(true);
        int interval = (int)(zmax - zmin);
        entry.amcpebcp_oracle_calls = interval > 1
            ? (int)std::ceil(std::log2(interval)) + 2
            : 1;
    }

    {
        auto t0 = std::chrono::high_resolution_clock::now();

        ParametricSearch solver;
        ParametricResult res = solver.solve(G, W);

        auto t1 = std::chrono::high_resolution_clock::now();

        entry.parametric_time_ms     = std::chrono::duration<double, std::milli>(t1 - t0).count();
        entry.parametric_result_z    = res.maxCapacity;
        entry.parametric_result_cost = res.finalCost;
        entry.parametric_oracle_calls = nodes * (nodes - 1) / 2;
    }

    entry.z_difference = std::abs(entry.amcpebcp_result_z - entry.parametric_result_z);

    return entry;
}

void PerformanceTest::saveToCSV(const std::vector<BenchmarkEntry>& results, const std::string& filename)
{
    std::ofstream f(filename);

    f << "nodes,edges,W,"
      << "amcpebcp_time_ms,amcpebcp_oracle_calls,amcpebcp_z,amcpebcp_cost,"
      << "parametric_time_ms,parametric_oracle_calls,parametric_z,parametric_cost,"
      << "z_difference\n";

    for (const auto& e : results) {
        f << e.nodes                  << ","
          << e.edges                  << ","
          << e.W                      << ","
          << e.amcpebcp_time_ms       << ","
          << e.amcpebcp_oracle_calls  << ","
          << e.amcpebcp_result_z      << ","
          << e.amcpebcp_result_cost   << ","
          << e.parametric_time_ms     << ","
          << e.parametric_oracle_calls<< ","
          << e.parametric_result_z    << ","
          << e.parametric_result_cost << ","
          << e.z_difference           << "\n";
    }
}

