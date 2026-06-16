#include "graph.h"
#include <limits>

Graph::Graph(int nodeCount, int source, int target)
    : nodeCount_(nodeCount), source_(source), target_(target)
{
    adj_.resize(nodeCount_ + 1);
}

int Graph::getNodesCount() const { return nodeCount_; }
int Graph::getSource() const { return source_; }
int Graph::getTarget() const { return target_; }

void Graph::addEdge(const Edge& edge)
{
    adj_[edge.getFrom()].push_back(edge);
}

const std::vector<Edge>& Graph::getEdgesFrom(int nodeId) const
{
    return adj_[nodeId];
}

std::vector<Edge> Graph::getAllEdges() const
{
    std::vector<Edge> allEdges;

    for (int i = 1; i <= nodeCount_; ++i)
    {
        for (const auto& edge : adj_[i])
        {
            allEdges.push_back(edge);
        }
    }
    return allEdges;
}

std::pair<double, std::vector<int>> Graph::findMCPWithPath(bool useMaxCapacities) const
{
    std::vector<double> cap(nodeCount_ + 1, -1.0);
    std::vector<bool> vis(nodeCount_ + 1, false);
    std::vector<int> parent(nodeCount_ + 1, -1);

    cap[source_] = std::numeric_limits<double>::infinity();

    for (int iter = 0; iter < nodeCount_; ++iter)
    {
        int u = -1;
        double mx = -1.0;

        for (int j = 1; j <= nodeCount_; ++j)
        {
            if (!vis[j] && cap[j] > mx)
            {
                mx = cap[j];
                u = j;
            }
        }

        if (u == -1 || u == target_)
            break;

        vis[u] = true;

        for (const Edge& e : adj_[u])
        {
            if (!useMaxCapacities && e.isNew())
                continue;

            int v = e.getTo();
            double edgeCap = useMaxCapacities ? e.getMaxCapacity() : e.getCapacity();

            if (!vis[v])
            {
                double pc = std::min(cap[u], edgeCap);
                if (pc > cap[v])
                {
                    cap[v] = pc;
                    parent[v] = u;
                }
            }
        }
    }

    std::vector<int> path;
    if (cap[target_] < 0)
        return { cap[target_], path };

    int curr = target_;
    while (curr != -1)
    {
        path.push_back(curr);
        if (curr == source_)
            break;
        curr = parent[curr];
    }

    if (path.empty() || path.back() != source_)
        return { cap[target_], {} };

    std::reverse(path.begin(), path.end());
    return { cap[target_], path };
}

double Graph::findMCP(bool useMaxCapacities) const
{
    return findMCPWithPath(useMaxCapacities).first;
}
