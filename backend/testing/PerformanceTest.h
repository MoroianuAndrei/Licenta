#pragma once

#include "backend/core/graph.h"
#include <vector>
#include <string>

struct BenchmarkEntry {
    int    nodes;
    int    edges;
    double W;

    // AMCPEBCP (Algorithm 3) cu cost liniar
    double amcpebcp_time_ms;
    int    amcpebcp_oracle_calls;
    double amcpebcp_result_z;
    double amcpebcp_result_cost;

    // Parametric Search (Algorithm 4) cu cost liniar
    double parametric_time_ms;
    int    parametric_oracle_calls;
    double parametric_result_z;
    double parametric_result_cost;

    // Diferenta rezultate (pentru corectitudine)
    double z_difference;
};

class PerformanceTest {
public:
    // Ruleaza benchmark pentru o singura dimensiune de graf
    // nodes = numarul de noduri
    // W = bugetul
    // seed = pentru reproductibilitate
    BenchmarkEntry runSingle(int nodes, double W, unsigned int seed = 42);

    // Salveaza rezultatele in CSV
    void saveToCSV(
        const std::vector<BenchmarkEntry>& results,
        const std::string& filename
    );
};