#pragma once
#include "../core/graph.h"
#include "AMCMCPEP.h"
#include <vector>

// Structura pentru a returna rezultatul Algoritmului 3
struct AMCPEBCP_Result {
    double maxCapacity;    // Cea mai mare capacitate z pe care o putem obtine
    double finalCost;      // Costul real platit (<= W)
    std::vector<int> path; // Drumul optim P*
};

class AMCPEBCP {
private:
    // Functie helper pentru a gasi Maximum Capacity Path (MCP) - Pasul 1 si 2
    double findMCP(const Graph& G, bool useMaxCapacities);

public:
    // W = Bugetul maxim disponibil
    // epsilon = Marja de eroare pentru oprirea cautarii binare (ex: 0.001)
    AMCPEBCP_Result solve(const Graph& G, double W, double epsilon = 1e-4);
};