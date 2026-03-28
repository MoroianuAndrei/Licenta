#include "graph.h"
#include <limits>

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

double Graph::findMCP(bool useMaxCapacities) const
{
    std::vector<double> cap(n + 1, -1.0);
    std::vector<bool>   vis(n + 1, false);
    cap[s] = std::numeric_limits<double>::infinity();

    for (int iter = 0; iter < n; ++iter)
    {
        // Selectam nodul nevizitat cu capacitatea maxima
        int u = -1; double mx = -1.0;
        for (int j = 1; j <= n; ++j)
            if (!vis[j] && cap[j] > mx) { mx = cap[j]; u = j; }

        if (u == -1 || u == t) break;
        vis[u] = true;

        for (const Edge& e : adj[u])
        {
            // useMaxCapacities=false: G original => ignoram arcele noi
            if (!useMaxCapacities && e.isNew()) continue;

            int    v = e.getTo();
            double edgeCap = useMaxCapacities ? e.getMaxCapacity() : e.getCapacity();

            if (!vis[v])
            {
                double pc = std::min(cap[u], edgeCap);
                if (pc > cap[v]) cap[v] = pc;
            }
        }
    }
    return cap[t];
}