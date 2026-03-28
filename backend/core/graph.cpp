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

std::pair<double, std::vector<int>> Graph::findMCPWithPath(bool useMaxCapacities) const
{
    std::vector<double> cap(n + 1, -1.0);
    std::vector<bool> vis(n + 1, false);
    std::vector<int> parent(n + 1, -1);

    cap[s] = std::numeric_limits<double>::infinity();

    for (int iter = 0; iter < n; ++iter)
    {
        int u = -1;
        double mx = -1.0;

        for (int j = 1; j <= n; ++j)
        {
            if (!vis[j] && cap[j] > mx)
            {
                mx = cap[j];
                u = j;
            }
        }

        if (u == -1 || u == t)
            break;

        vis[u] = true;

        for (const Edge& e : adj[u])
        {
            // In G original ignoram arcele noi
            if (!useMaxCapacities && e.isNew())
                continue;

            int v = e.getTo();
            double edgeCap = useMaxCapacities ? e.getMaxCapacity() : e.getCapacity();

            if (!vis[v])
            {
                double pathCap = std::min(cap[u], edgeCap);
                if (pathCap > cap[v])
                {
                    cap[v] = pathCap;
                    parent[v] = u;
                }
            }
        }
    }

    std::vector<int> path;
    if (cap[t] < 0)
        return { cap[t], path };

    int curr = t;
    while (curr != -1)
    {
        path.push_back(curr);
        if (curr == s)
            break;
        curr = parent[curr];
    }

    if (path.empty() || path.back() != s)
        return { cap[t], {} };

    std::reverse(path.begin(), path.end());
    return { cap[t], path };
}

double Graph::findMCP(bool useMaxCapacities) const
{
    return findMCPWithPath(useMaxCapacities).first;
}