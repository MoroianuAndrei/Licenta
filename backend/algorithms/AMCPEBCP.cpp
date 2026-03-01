#include "AMCPEBCP.h"
#include <algorithm>
#include <limits>
#include <iostream>

// Implementarea Algoritmului 3 propriu-zis
AMCPEBCP_Result AMCPEBCP::solve(const Graph& G, double W, double epsilon) {

    // Pasul 1: Calculeaza z_min (MCP in rețeaua originala G, fara a folosi arce noi)
    double z_min = findMCP(G, false);

    // Pasul 2: Calculeaza z_max (MCP in rețeaua extinsa G^E, folosind capacitatile maxime c_hat)
    double z_max = findMCP(G, true);

    AMCMCPEP solverAlg1; // Instantiem Algoritmul 1 pe care sa il folosim ca subrutina

    // Pasul 3: Verificam daca bugetul acopera direct capacitatea maxima posibila
    AMCMCPEP_Result res_max = solverAlg1.solve(G, z_max);
    if (res_max.isFeasible && res_max.totalCost <= W) {
        return { z_max, res_max.totalCost, res_max.path };
    }

    // Variabile pentru a salva cel mai bun rezultat gasit care se incadreaza in buget
    double best_z = z_min;
    double best_cost = 0.0;
    std::vector<int> best_path;

    // Rulam o data pentru z_min ca sa avem un drum initial valid (costul ar trebui sa fie 0)
    AMCMCPEP_Result initial_res = solverAlg1.solve(G, z_min);
    if (initial_res.isFeasible && initial_res.totalCost <= W) {
        best_cost = initial_res.totalCost;
        best_path = initial_res.path;
    }

    // Pasul 4: Cautarea binara (Bucla principala)
    while (z_max - z_min > epsilon) {
        double z = (z_min + z_max) / 2.0;

        // Calculam C(z) folosind Algoritmul 1
        AMCMCPEP_Result current_res = solverAlg1.solve(G, z);

        // Verificam daca exista drum si daca ne incadram in buget
        if (current_res.isFeasible && current_res.totalCost <= W) {
            // Ne permitem aceasta capacitate! Salvam rezultatul si incercam o capacitate si mai mare
            z_min = z;
            best_z = z;
            best_cost = current_res.totalCost;
            best_path = current_res.path;
        }
        else {
            // E prea scump (depaseste bugetul) sau infezabil. Trebuie sa scadem capacitatea.
            z_max = z;
        }
    }

    // Pasul 5: Returnam capacitatea maxima gasita si drumul ei
    return { best_z, best_cost, best_path };
}

// O varianta adaptata de Dijkstra pentru a gasi doar capacitatea drumului "cel mai lat" (Bottleneck Path)
double AMCPEBCP::findMCP(const Graph& G, bool useMaxCapacities) {
    int n = G.getNodesCount();
    std::vector<double> cap(n + 1, -1.0);
    std::vector<bool> visited(n + 1, false);

    cap[G.getSource()] = std::numeric_limits<double>::infinity();

    for (int i = 1; i <= n; ++i) {
        int u = -1;
        double max_c = -1.0;

        // Cautam nodul nevizitat cu cea mai mare capacitate pe drumul pana la el
        for (int j = 1; j <= n; ++j) {
            if (!visited[j] && cap[j] > max_c) {
                max_c = cap[j];
                u = j;
            }
        }

        if (u == -1 || u == G.getTarget()) break;
        visited[u] = true;

        for (const Edge& edge : G.getEdgesFrom(u)) {
            // Daca calculam z_min, ignoram arcele noi (care nu exista inca in G)
            if (!useMaxCapacities && edge.isNew()) continue;

            double edge_c = useMaxCapacities ? edge.getMaxCapacity() : edge.getCapacity();
            int v = edge.getTo();

            if (!visited[v]) {
                // Capacitatea drumului este minimul dintre capacitatea pana la u si capacitatea muchiei (u, v)
                double path_cap = std::min(cap[u], edge_c);
                if (path_cap > cap[v]) {
                    cap[v] = path_cap;
                }
            }
        }
    }

    return cap[G.getTarget()];
}