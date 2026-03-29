#include "ValidationTests.h"
#include "TestUtils.h"
#include "BruteForceValidator.h"

#include "../core/graph.h"
#include "../algorithms/AMCMCPEP.h"

#include <iostream>
#include <cmath>
#include <string>

static void compareAMCMCPEPWithBruteForce(const Graph& G, double z, const std::string& label)
{
    printSeparator("VALIDARE BRUTE-FORCE - " + label);

    AMCMCPEP solver;
    BruteForceValidator brute;

    AMCMCPEP_Result alg = solver.solve(G, z);
    BruteForceResult bf = brute.solveAMCMCPEP(G, z);

    std::cout << "\n  [Algoritm]\n";
    std::cout << "  Fezabil:      " << (alg.isFeasible ? "DA" : "NU") << "\n";
    if (alg.isFeasible)
    {
        std::cout << "  Cost total:   " << alg.totalCost << "\n";
        std::cout << "  Drum P*:      ";
        printPath(alg.path);
    }

    std::cout << "\n  [Brute-force]\n";
    std::cout << "  Fezabil:      " << (bf.feasible ? "DA" : "NU") << "\n";
    if (bf.feasible)
    {
        std::cout << "  Cost total:   " << bf.totalCost << "\n";
        std::cout << "  Drum optim:   ";
        printPath(bf.path);
    }

    bool sameFeasibility = (alg.isFeasible == bf.feasible);
    bool sameCost = (!alg.isFeasible && !bf.feasible) ||
        (std::abs(alg.totalCost - bf.totalCost) < 1e-6);

    std::cout << "\n  Verdict:      "
        << ((sameFeasibility && sameCost) ? "CORECT" : "NECONCORDANTA")
        << "\n";
}

static void validation_SmallGraph_1()
{
    Graph G(4, 1, 4);
    const double z = 10.0;

    auto B = [](double c) {
        return [c](double x) { return x <= c ? 0.0 : 2.0 * (x - c); };
        };

    auto U = [](double c) {
        return [c](double x) { return x <= c ? 0.0 : 1.0 * (x - c) * (x - c); };
        };

    G.addEdge(Edge(1, 2, 7, 15, false, U(7), B(7)));
    G.addEdge(Edge(2, 4, 8, 15, false, U(8), B(8)));
    G.addEdge(Edge(1, 3, 9, 15, false, U(9), B(9)));
    G.addEdge(Edge(3, 4, 6, 15, false, U(6), B(6)));
    G.addEdge(Edge(2, 3, 10, 15, false, U(10), B(10)));

    compareAMCMCPEPWithBruteForce(G, z, "graf mic custom 1");
}

static void validation_NoExpansionNeeded()
{
    Graph G(3, 1, 3);
    const double z = 20.0;
    const double INF = 1e9;

    auto B = [](double c) {
        return [c](double x) { return x <= c ? 0.0 : 5.0 * (x - c); };
        };

    auto U = [](double c) {
        return [c](double x) { return x <= c ? 0.0 : 2.0 * (x - c) * (x - c); };
        };

    G.addEdge(Edge(1, 2, 30, INF, false, U(30), B(30)));
    G.addEdge(Edge(2, 3, 25, INF, false, U(25), B(25)));

    compareAMCMCPEPWithBruteForce(G, z, "fara expansiune");
}

static void validation_Infeasible()
{
    Graph G(3, 1, 3);
    const double z = 15.0;

    auto B = [](double c) {
        return [c](double x) { return x <= c ? 0.0 : 5.0 * (x - c); };
        };

    auto U = [](double c) {
        return [c](double x) { return x <= c ? 0.0 : 2.0 * (x - c) * (x - c); };
        };

    G.addEdge(Edge(1, 2, 5, 20, false, U(5), B(5)));

    compareAMCMCPEPWithBruteForce(G, z, "infezabil");
}

void runAllValidationTests()
{
    std::cout << "\n";
    std::cout << "************************************************************\n";
    std::cout << "   VALIDARE CU BRUTE-FORCE                                 \n";
    std::cout << "************************************************************\n";

    validation_SmallGraph_1();
    validation_NoExpansionNeeded();
    validation_Infeasible();

    std::cout << "\n************************************************************\n";
    std::cout << "   VALIDAREA CU BRUTE-FORCE S-A INCHEIAT\n";
    std::cout << "************************************************************\n\n";
}