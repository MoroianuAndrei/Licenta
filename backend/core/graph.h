#pragma once
#include <vector>
#include "edge.h"

class Graph
{
private:
    int n;
    int s;
    int t;

    // Lista de adiacenta: adj[i] contine toate arcele care pleaca din nodul i
    std::vector<std::vector<Edge>> adj;

public:
    Graph(int nodeCount, int source, int target);

    int getNodesCount() const;
    int getSource() const;
    int getTarget() const;

    void addEdge(const Edge& edge);

    const std::vector<Edge>& getEdgesFrom(int nodeId) const;

    std::vector<Edge> getAllEdges() const;
};