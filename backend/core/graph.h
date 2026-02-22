#pragma once
#include <vector>
#include "edge.h"

class Graph {
private:
    int n; // Numarul de noduri (|V|) 
    int s; // Nodul sursa s 
    int t; // Nodul sink t 

    // Lista de adiacenta: adj[i] contine toate arcele care pleaca din nodul i
    std::vector<std::vector<Edge>> adj;

public:
    Graph(int nodeCount, int source, int target);

    // Getteri
    int getNodesCount() const;
    int getSource() const;
    int getTarget() const;

    void addEdge(const Edge& edge);

    // Returnam lista de arce ca referinta constanta (pentru performanta)
    const std::vector<Edge>& getEdgesFrom(int nodeId) const;
};