#include "graph.h"

Graph::Graph(int nodeCount, int source, int target)
    : n(nodeCount), s(source), t(target)
{
    adj.resize(n + 1);
}

int Graph::getNodesCount() const { return n; }
int Graph::getSource() const { return s; }
int Graph::getTarget() const { return t; }

void Graph::addEdge(const Edge& edge)
{
    adj[edge.getFrom()].push_back(edge);
}

const std::vector<Edge>& Graph::getEdgesFrom(int nodeId) const
{
    return adj[nodeId];
}

std::vector<Edge> Graph::getAllEdges() const
{
    std::vector<Edge> allEdges;

    for (int i = 1; i <= n; ++i)
    {
        for (const auto& edge : adj[i])
        {
            allEdges.push_back(edge);
        }
    }
    return allEdges;
}