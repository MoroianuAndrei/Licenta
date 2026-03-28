#pragma once
#include <vector>
#include <utility>
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

    // Dijkstra bottleneck - Maximum Capacity Path
    // useMaxCapacities = false => MCP in G original (ignora arcele noi, foloseste c(w))
    // useMaxCapacities = true  => MCP in G_hat      (foloseste c_hat(w) pe toate arcele)
    double findMCP(bool useMaxCapacities) const;

    // Intoarce {capacitate MCP, drumul MCP}
    std::pair<double, std::vector<int>> findMCPWithPath(bool useMaxCapacities) const;
};