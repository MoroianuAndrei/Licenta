#include "AMCMCPEP.h"
#include <limits>
#include <algorithm>

AMCMCPEP_Result AMCMCPEP::solve(const Graph& G, double z) {
    int n = G.getNodesCount(); // <-- Folosim getter

    M.assign(n + 1, std::numeric_limits<double>::infinity());
    S.assign(n + 1, std::numeric_limits<double>::infinity());
    R.assign(n + 1, 0);
    p.assign(n + 1, -1);

    M[G.getSource()] = 0.0; // <-- Folosim getter
    S[G.getSource()] = 0.0; // <-- Folosim getter

    while (R[G.getTarget()] == 0) { // <-- Folosim getter
        int u = -1;
        double min_val = std::numeric_limits<double>::infinity();

        for (int i = 1; i <= n; ++i) {
            if (R[i] == 0 && (M[i] + S[i]) < min_val) {
                min_val = M[i] + S[i];
                u = i;
            }
        }

        if (u == -1) {
            return { false, 0.0, {} };
        }

        R[u] = 1;

        // <-- Aici folosim G.getEdgesFrom() in loc de G.adj[u]
        for (const Edge& edge : G.getEdgesFrom(u)) {
            int v = edge.getTo(); // <-- Folosim getter

            if (R[v] == 0) {
                if (edge.getMaxCapacity() >= z) { // <-- Folosim getter

                    // <-- Verificam capacitatea folosind getter-ul
                    double cost_u = (edge.getCapacity() >= z) ? 0.0 : edge.getMaxCost(z);
                    double cost_b = (edge.getCapacity() >= z) ? 0.0 : edge.getSumCost(z);

                    double current_max = std::max(M[u], cost_u);
                    if (current_max + S[u] + cost_b < M[v] + S[v]) {
                        M[v] = current_max;
                        S[v] = S[u] + cost_b;
                        p[v] = u;
                    }
                }
            }
        }
    }

    // <-- Folosim getterii pentru s si t
    std::vector<int> path = getPath(G.getSource(), G.getTarget());
    return { true, M[G.getTarget()] + S[G.getTarget()], path };
}

std::vector<int> AMCMCPEP::getPath(int sourceNode, int targetNode) {
    std::vector<int> path;
    int j = targetNode;

    while (j != sourceNode && j != -1) {
        path.push_back(j);
        j = p[j];
    }

    if (j == sourceNode) {
        path.push_back(sourceNode);
    }

    std::reverse(path.begin(), path.end());
    return path;
}