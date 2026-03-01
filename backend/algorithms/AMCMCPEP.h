#pragma once
#include <vector>
#include "../core/graph.h"

// O structură curată pentru a returna rezultatele algoritmului
struct AMCMCPEP_Result
{
    bool isFeasible;       // true dacă există o cale, false dacă nu se poate atinge z
    double totalCost;      // Costul total de expansiune: M(t) + S(t)
    std::vector<int> path; // Calea găsită P*
};

class AMCMCPEP
{
private:
    // Structurile de date interne ale algoritmului (Liniile 12-21) 
    std::vector<double> M; // Costul de tip maxim
    std::vector<double> S; // Costul de tip sumă
    std::vector<int> R;    // Eticheta binară (0 = nevizitat, 1 = vizitat)
    std::vector<int> p;    // Vectorul de predecesori

    // Subrutina internă: Algoritmul 2 (APF - Algorithm Path Find) [cite: 168, 178]
    std::vector<int> getPath(int sourceNode, int targetNode);

public:
    // Funcția principală care rulează Algoritmul 1 pe un graf G pentru o capacitate dorită z
    AMCMCPEP_Result solve(const Graph& G, double z);
};