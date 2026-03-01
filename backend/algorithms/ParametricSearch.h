#pragma once
#include "../core/graph.h"
#include "LinearFunc.h"
#include "AMCMCPEP.h" // Algoritmul 1 folosit pe post de Oracol
#include <vector>

struct ParametricResult {
    double maxCapacity;    // z* optim (valoarea exacta)
    double finalCost;      // Costul final (ar trebui sa fie egal cu W)
    std::vector<int> path; // Drumul optim
};

class ParametricSearch {
private:
    AMCMCPEP oracle; // Instanta a Algoritmului 1

    // Returneaza z1 < z2 < ... < zl
    std::vector<double> getCriticalValues(const Graph& G);

public:
    ParametricResult solve(const Graph& G, double W);
};