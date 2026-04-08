#include "ValidationTests.h"
#include "TestUtils.h"
#include "BruteForceValidator.h"

#include "../core/graph.h"
#include "../algorithms/AMCMCPEP.h"

#include <iostream>
#include <cmath>
#include <string>
#include <random>

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

static Graph generateRandomSmallGraph(int n, double edgeProb, std::mt19937& rng)
{
    Graph G(n, 1, n);

    std::uniform_real_distribution<double> probDist(0.0, 1.0);
    std::uniform_int_distribution<int> capDist(1, 10);
    std::uniform_int_distribution<int> extraDist(0, 8);
    std::uniform_int_distribution<int> alphaDist(1, 4);
    std::uniform_int_distribution<int> betaDist(1, 3);

    for (int i = 1; i <= n; ++i)
    {
        for (int j = 1; j <= n; ++j)
        {
            if (i == j) continue;

            if (probDist(rng) > edgeProb) continue;

            double c = capDist(rng);
            double c_hat = c + extraDist(rng);

            int alpha = alphaDist(rng);
            int beta = betaDist(rng);

            auto B = [c, alpha](double z) {
                return (z <= c) ? 0.0 : alpha * (z - c);
                };

            auto U = [c, beta](double z) {
                return (z <= c) ? 0.0 : beta * (z - c) * (z - c);
                };

            G.addEdge(Edge(i, j, c, c_hat, false, U, B));
        }
    }

    return G;
}

static double chooseRandomZ(const Graph& G, std::mt19937& rng)
{
    double zmin = G.findMCP(false);
    double zmax = G.findMCP(true);

    if (zmax < 0) return 1.0;
    if (zmin < 0) zmin = 1.0;
    if (zmax < zmin) zmax = zmin;

    int lo = (int)std::max(1.0, std::floor(zmin));
    int hi = (int)std::ceil(zmax + 2.0);

    std::uniform_int_distribution<int> dist(lo, std::max(lo, hi));
    return (double)dist(rng);
}

static void runRandomBruteForceValidation(int testCount)
{
    printSeparator("VALIDARE RANDOM AMCMCPEP vs BRUTE-FORCE");

    std::mt19937 rng(42);
    int passed = 0;

    for (int t = 1; t <= testCount; ++t)
    {
        int n = 4 + (t % 3); // 4,5,6 noduri

        Graph G = generateRandomSmallGraph(n, 0.35, rng);
        double z = chooseRandomZ(G, rng);

        AMCMCPEP solver;
        BruteForceValidator brute;

        AMCMCPEP_Result alg = solver.solve(G, z);
        BruteForceResult bf = brute.solveAMCMCPEP(G, z);

        bool sameFeasible = (alg.isFeasible == bf.feasible);
        bool sameCost = (!alg.isFeasible && !bf.feasible) ||
            (std::abs(alg.totalCost - bf.totalCost) < 1e-6);

        if (sameFeasible && sameCost)
        {
            passed++;
        }
        else
        {
            std::cout << "\n❌ Test esuat #" << t << "\n";
            std::cout << "n=" << n << ", z=" << z << "\n";

            std::cout << "Algoritm: feasible=" << alg.isFeasible
                << ", cost=" << alg.totalCost << "\n";

            std::cout << "Brute-force: feasible=" << bf.feasible
                << ", cost=" << bf.totalCost << "\n";

            std::cout << "Drum algoritm: ";
            printPath(alg.path);

            std::cout << "Drum brute-force: ";
            printPath(bf.path);

            break;
        }
    }

    std::cout << "\nRezultat: " << passed << " / " << testCount << " teste corecte\n";
}

void runAllValidationTests()
{
    printSeparator("VALIDARE CU BRUTE-FORCE");

    runRandomBruteForceValidation(50);
}