#pragma once
#include <vector>
#include "graph.h"

struct AMCMCPEP_Result
{
    bool isFeasible; // true daca problema e fezabila
    double totalCost;  // M(t) + S(t) - costul total de expansiune
    std::vector<int> path;   // Drumul P* gasit
};

class AMCMCPEP
{
private:
    std::vector<double> M; // Costul de tip max  pe drumul pana la nodul i
    std::vector<double> S; // Costul de tip suma pe drumul pana la nodul i
    std::vector<int> R; // R(i) = 1 daca nodul i a fost "checked"
    std::vector<int> p; // Predecesorul nodului i pe drumul optim

    // Algorithm 2 (APF) - reconstruieste drumul din vectorul p
    std::vector<int> getPath(int sourceNode, int targetNode);

    // Pasul 3: BFS - verifica daca t e accesibil din s in G^E
    bool isTargetReachable(const Graph& G) const;

public:
    // Algorithm 1 complet (pasii 1-49 din paper, inclusiv 3-11)
    AMCMCPEP_Result solve(const Graph& G, double z);
};