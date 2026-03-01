#include "ParametricSearch.h"
#include <algorithm>
#include <set>
#include <iostream>

std::vector<double> ParametricSearch::getCriticalValues(const Graph& G) {
    std::set<double> values;
    values.insert(0.0);

    // Extragem toate capacitatile c(w) si c_hat(w)
    for (const auto& edge : G.getAllEdges()) {
        values.insert(edge.getCapacity());
        if (edge.getMaxCapacity() < 1000000) { // Ignoram infinitul
            values.insert(edge.getMaxCapacity());
        }
    }

    return std::vector<double>(values.begin(), values.end());
}

ParametricResult ParametricSearch::solve(const Graph& G, double W) {
    int n = G.getNodesCount();

    // =========================================================
    // FAZA 1: Cautare Binara pe valorile critice z1...zl
    // =========================================================
    std::vector<double> Z = getCriticalValues(G);

    double lower_z = Z.front();
    double upper_z = Z.back();

    int left_idx = 0;
    int right_idx = Z.size() - 1;

    while (left_idx <= right_idx) {
        int mid = left_idx + (right_idx - left_idx) / 2;
        double z_test = Z[mid];

        // Intrebam Oracolul
        AMCMCPEP_Result res = oracle.solve(G, z_test);

        if (res.isFeasible && res.totalCost <= W) {
            lower_z = z_test;   // Ne permitem, poate putem si mai mult
            left_idx = mid + 1;
        }
        else {
            upper_z = z_test;   // Prea scump, coboram limita superioara
            right_idx = mid - 1;
        }
    }

    // Intervalul sigur este acum [L, R]
    double L = lower_z;
    double R = upper_z;

    // =========================================================
    // FAZA 2: Cautarea Parametrica (Megiddo) in intervalul [L, R]
    // =========================================================
    std::vector<LinearFunc> S(n + 1, LinearFunc::Infinity());
    std::vector<int> parent(n + 1, 0);
    std::vector<bool> permanent(n + 1, false);

    S[G.getSource()] = LinearFunc(0.0, 0.0); // Cost 0 pentru nodul sursa

    for (int iter = 1; iter <= n; ++iter) {

        // --- A. Alegerea nodului cu S(i) minim (Turneul Dreptelor) ---
        int u = -1;

        for (int i = 1; i <= n; ++i) {
            if (!permanent[i] && !S[i].isInfinity()) {
                if (u == -1) {
                    u = i;
                }
                else {
                    // Comparam S[u] cu S[i]
                    LinearFunc diff = S[u] - S[i]; // Forma: u - vz

                    if (std::abs(diff.r) < 1e-9) {
                        // Linii paralele -> comparam pur si simplu in punctul L
                        if (S[i].eval(L) < S[u].eval(L)) u = i;
                    }
                    else {
                        // Exista un punct de intersectie
                        double z_int = -diff.q / diff.r;

                        // Daca intersectia e in afara intervalului nostru de interes
                        if (z_int <= L || z_int >= R) {
                            if (S[i].eval(L) < S[u].eval(L)) u = i;
                        }
                        else {
                            // Intersectia este RELEVANTA! Intrebam Oracolul la z_int
                            AMCMCPEP_Result res = oracle.solve(G, z_int);

                            if (res.isFeasible && res.totalCost <= W) {
                                L = z_int; // Solutia e mai la dreapta
                                if (S[i].eval(L + 0.0001) < S[u].eval(L + 0.0001)) u = i;
                            }
                            else {
                                R = z_int; // Solutia e mai la stanga
                                if (S[i].eval(R - 0.0001) < S[u].eval(R - 0.0001)) u = i;
                            }
                        }
                    }
                }
            }
        }

        if (u == -1 || u == G.getTarget()) break;
        permanent[u] = true;

        // --- B. Relaxarea muchiilor (Filtrarea A-bar) ---
        for (const Edge& edge : G.getEdgesFrom(u)) {

            // Conditia Ecuatiei (29): c(w) <= zk si c_hat(w) >= zk+1
            if (edge.getCapacity() <= L + 1e-9 && edge.getMaxCapacity() >= R - 1e-9) { //

                // Calculam ecuatia muchiei curente. 
                // In cazul liniar, costul e r * (z - c) => r * z - r * c
                // Presupunem ca ai o metoda in clasa Edge sa iti dea panta (Rate)
                double r = edge.getLinearRate(); // AICI TREBUIE SA AI METODA ASTA IN EDGE
                double base = -r * edge.getCapacity();

                LinearFunc edgeCost(r, base);
                LinearFunc newPathCost = S[u] + edgeCost;

                int v = edge.getTo();

                // Update daca e mai bun in intervalul nostru
                if (S[v].isInfinity() || newPathCost.eval(L) < S[v].eval(L)) {
                    S[v] = newPathCost;
                    parent[v] = u;
                }
            }
        }
    }

    // =========================================================
    // FINAL: Rezolvarea ecuatiei exacte pentru W
    // =========================================================
    LinearFunc finalFunc = S[G.getTarget()];
    double optimal_z = 0;
    double final_cost = 0;

    if (!finalFunc.isInfinity()) {
        if (std::abs(finalFunc.r) > 1e-9) {
            // R * z + Q = W => z = (W - Q) / R
            optimal_z = (W - finalFunc.q) / finalFunc.r;
        }
        else {
            optimal_z = R; // Cost constant, luam maximul posibil din interval
        }

        // Asiguram incadrarea in interval din cauza erorilor de double
        if (optimal_z < L) optimal_z = L;
        if (optimal_z > R) optimal_z = R;

        final_cost = finalFunc.eval(optimal_z);
    }

    // Reconstructia drumului
    std::vector<int> path;
    int curr = G.getTarget();
    while (curr != 0 && curr != G.getSource()) {
        path.push_back(curr);
        curr = parent[curr];
    }
    path.push_back(G.getSource());
    std::reverse(path.begin(), path.end());

    return { optimal_z, final_cost, path };
}