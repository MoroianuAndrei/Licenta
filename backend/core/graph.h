#pragma once
#include <vector>
#include "edge.h"

class Graph
{
private:
    int nodeCount_;
    int source_;
    int target_;

    std::vector<std::vector<Edge>> adj_;

public:
    Graph(int nodeCount, int source, int target);

    int getNodesCount() const;
    int getSource() const;
    int getTarget() const;

    void addEdge(const Edge& edge);

    const std::vector<Edge>& getEdgesFrom(int nodeId) const;

    std::vector<Edge> getAllEdges() const;

    double findMCP(bool useMaxCapacities) const;

    std::pair<double, std::vector<int>> findMCPWithPath(bool useMaxCapacities) const;
};
