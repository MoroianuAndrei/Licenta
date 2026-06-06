#include "ManualTests.h"
#include "TestUtils.h"

#include "../core/graph.h"
#include "../algorithms/AMCMCPEP.h"
#include "../algorithms/AMCPEBCP.h"
#include "../algorithms/ParametricSearch.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>

static void test_Algorithm1_Fig1()
{
    printSeparator("TEST 1: Algorithm 1 (AMCMCPEP) - Fig.1 din paper");
    std::cout << "  Retea: 7 noduri, s=1, t=7\n";
    std::cout << "  Scop: extinde reteaua astfel incat MCP sa aiba capacitatea z=20\n";
    std::cout << "  Costuri: b(w,x) = 10*|j-i|*(x-c), u(w,x) = |j-i|*(x-c)^2\n";

    Graph G(7, 1, 7);
    const double z = 20.0;
    const double INF = 1e9;

    auto B = [](int i, int j, double c) {
        return [i, j, c](double x) -> double {
            return (x <= c) ? 0.0 : 10.0 * std::abs(j - i) * (x - c);
            };
        };

    auto U = [](int i, int j, double c) {
        return [i, j, c](double x) -> double {
            return (x <= c) ? 0.0 : (double)std::abs(j - i) * (x - c) * (x - c);
            };
        };

    G.addEdge(Edge(1, 3, 23, INF, false, U(1, 3, 23), B(1, 3, 23)));
    G.addEdge(Edge(1, 4, 12, INF, false, U(1, 4, 12), B(1, 4, 12)));
    G.addEdge(Edge(3, 2, 10, INF, false, U(3, 2, 10), B(3, 2, 10)));
    G.addEdge(Edge(3, 6, 14, INF, false, U(3, 6, 14), B(3, 6, 14)));
    G.addEdge(Edge(2, 4, 16, INF, false, U(2, 4, 16), B(2, 4, 16)));
    G.addEdge(Edge(2, 5, 15, INF, false, U(2, 5, 15), B(2, 5, 15)));
    G.addEdge(Edge(4, 6, 20, INF, false, U(4, 6, 20), B(4, 6, 20)));
    G.addEdge(Edge(6, 7, 9, INF, false, U(6, 7, 9), B(6, 7, 9)));

    AMCMCPEP solver;
    AMCMCPEP_Result res = solver.solve(G, z);

    printResult("Alg1 / Fig.1 / z=20",
        res.isFeasible,
        res.totalCost,
        res.path,
        411.0,
        "1->3->2->4->6->7");
}

static void test_Algorithm1_NoExpansionNeeded()
{
    printSeparator("TEST 2: Algorithm 1 - Reteaua nu are nevoie de expansiune");
    std::cout << "  Retea: 3 noduri, s=1, t=3\n";
    std::cout << "  Toate arcele au c(w) >= z => cost = 0\n";

    Graph G(3, 1, 3);
    const double INF = 1e9;

    auto B = [](double c) {
        return [c](double x) { return x <= c ? 0.0 : 5.0 * (x - c); };
        };

    auto U = [](double c) {
        return [c](double x) { return x <= c ? 0.0 : 2.0 * (x - c) * (x - c); };
        };

    G.addEdge(Edge(1, 2, 30, INF, false, U(30), B(30)));
    G.addEdge(Edge(2, 3, 25, INF, false, U(25), B(25)));

    AMCMCPEP solver;
    AMCMCPEP_Result res = solver.solve(G, 20.0);

    printResult("Alg1 / fara expansiune / z=20",
        res.isFeasible,
        res.totalCost,
        res.path,
        0.0);
}

static void test_Algorithm1_Infeasible()
{
    printSeparator("TEST 3: Algorithm 1 - Problema infezabila");
    std::cout << "  Retea: 3 noduri, s=1, t=3, dar t nu e accesibil din s\n";
    std::cout << "  Asteptat: isFeasible = false\n";

    Graph G(3, 1, 3);

    auto B = [](double c) {
        return [c](double x) { return x <= c ? 0.0 : 5.0 * (x - c); };
        };

    auto U = [](double c) {
        return [c](double x) { return x <= c ? 0.0 : 2.0 * (x - c) * (x - c); };
        };

    G.addEdge(Edge(1, 2, 5, 20, false, U(5), B(5)));

    AMCMCPEP solver;
    AMCMCPEP_Result res = solver.solve(G, 15.0);

    std::cout << "\n  [Alg1 / infezabil]\n";
    std::cout << "  Fezabil:  " << (res.isFeasible ? "DA" : "NU") << "\n";
    std::cout << "  Status:   " << (!res.isFeasible ? "CORECT (infezabil asteptat)" : "GRESIT") << "\n";
}

static void test_Algorithm3_Fig5()
{
    printSeparator("TEST 4: Algorithm 3 (AMCPEBCP) - Fig.5 din paper");
    std::cout << "  Retea: 5 noduri, s=1, t=5, buget W=24\n";
    std::cout << "  Costuri liniare: b(w,z) = r*(z-c)\n";
    std::cout << "  Asteptat: z* = 13 (intreg), P* = 1->2->3->5\n";

    Graph G(5, 1, 5);

    G.addEdge(Edge(1, 2, 12, 15, false, 5.0));
    G.addEdge(Edge(1, 4, 8, 15, false, 4.0));
    G.addEdge(Edge(2, 3, 10, 15, false, 4.0));
    G.addEdge(Edge(3, 5, 14, 15, false, 6.0));
    G.addEdge(Edge(4, 5, 8, 15, false, 8.0));
    G.addEdge(Edge(2, 4, 6, 15, false, 6.0));

    AMCPEBCP solver;
    AMCPEBCP_Result res = solver.solve(G, 24.0);

    std::cout << "\n  [Alg3 / Fig.5 / W=24]\n";
    std::cout << "  z* =          " << res.maxCapacity << "\n";
    std::cout << "  Cost:         " << std::fixed << std::setprecision(4) << res.finalCost << "\n";
    std::cout << "  Drum P*:      ";
    printPath(res.path);
    std::cout << "  Asteptat:     z* = 13, drum = 1->2->3->5\n";

    bool ok = (std::abs(res.maxCapacity - 13.0) < 0.01) && !res.path.empty();
    std::cout << "  Status:       " << (ok ? "CORECT" : "DIFERIT DE ASTEPTAT") << "\n";
}

static void test_Algorithm3_ZeroBudget()
{
    printSeparator("TEST 5: Algorithm 3 - Buget W=0");
    std::cout << "  Asteptat: z* = zmin (capacitatea initiala a MCP)\n";

    Graph G(4, 1, 4);
    G.addEdge(Edge(1, 2, 10, 20, false, 3.0));
    G.addEdge(Edge(2, 3, 8, 20, false, 2.0));
    G.addEdge(Edge(3, 4, 12, 20, false, 4.0));
    G.addEdge(Edge(1, 3, 5, 20, false, 1.0));

    AMCPEBCP solver;
    AMCPEBCP_Result res = solver.solve(G, 0.0);

    std::cout << "\n  [Alg3 / W=0]\n";
    std::cout << "  z* =      " << std::fixed << std::setprecision(4) << res.maxCapacity << "\n";
    std::cout << "  Cost:     " << res.finalCost << "\n";
    std::cout << "  Drum P*:  ";
    printPath(res.path);
    std::cout << "  Status:   " << (res.finalCost <= 0.001 ? "CORECT (cost = 0)" : "GRESIT") << "\n";
}

static void test_Algorithm4_ExactSolution()
{
    printSeparator("TEST 6: Algorithm 4 (ParametricSearch) - Solutie exacta verificabila");
    std::cout << "  Retea: 3 noduri, s=1, t=3, W=30\n";
    std::cout << "  arc(1,2): c=5, r=3;  arc(2,3): c=8, r=4\n";
    std::cout << "  S(t)(z) = 3*(z-5) + 4*(z-8) = 7z - 47 = 30  =>  z* = 11.0\n";

    Graph G(3, 1, 3);
    G.addEdge(Edge(1, 2, 5, 20, false, 3.0));
    G.addEdge(Edge(2, 3, 8, 20, false, 4.0));

    ParametricSearch solver;
    ParametricResult res = solver.solve(G, 30.0);

    std::cout << "\n  [Alg4 / retea simpla / W=30]\n";
    std::cout << "  z* =          " << std::fixed << std::setprecision(6) << res.maxCapacity << "\n";
    std::cout << "  Cost:         " << std::setprecision(4) << res.finalCost << "\n";
    std::cout << "  Drum P*:      ";
    printPath(res.path);
    std::cout << "  Asteptat:     z* = 11.0, cost = 30.0, drum = 1->2->3\n";

    bool ok = std::abs(res.maxCapacity - 11.0) < 0.01;
    std::cout << "  Status:       " << (ok ? "CORECT" : "DIFERIT DE ASTEPTAT") << "\n";
}

static void test_Comparison_Alg3_vs_Alg4()
{
    printSeparator("TEST 7: Comparatie Algorithm 3 vs Algorithm 4 - Retea custom");
    std::cout << "  Retea: 4 noduri, s=1, t=4, W=50\n";
    std::cout << "  Verificam ca Alg3 si Alg4 dau acelasi z*\n";

    auto buildGraph = []() {
        Graph G(4, 1, 4);
        G.addEdge(Edge(1, 2, 5, 20, false, 3.0));
        G.addEdge(Edge(1, 3, 8, 20, false, 2.0));
        G.addEdge(Edge(2, 4, 6, 20, false, 4.0));
        G.addEdge(Edge(3, 4, 7, 20, false, 2.0));
        G.addEdge(Edge(2, 3, 4, 20, false, 1.0));
        return G;
        };

    Graph G3 = buildGraph();
    Graph G4 = buildGraph();

    AMCPEBCP solver3;
    ParametricSearch solver4;

    AMCPEBCP_Result res3 = solver3.solve(G3, 50.0);
    ParametricResult res4 = solver4.solve(G4, 50.0);

    std::cout << "\n  Algorithm 3:  z* = " << std::fixed << std::setprecision(4)
        << res3.maxCapacity << ",  cost = " << res3.finalCost << "\n";
    std::cout << "  Drum Alg3:    ";
    printPath(res3.path);

    std::cout << "\n  Algorithm 4:  z* = " << std::fixed << std::setprecision(4)
        << res4.maxCapacity << ",  cost = " << res4.finalCost << "\n";
    std::cout << "  Drum Alg4:    ";
    printPath(res4.path);

    bool sameZ = std::abs(res3.maxCapacity - res4.maxCapacity) < 0.1;
    std::cout << "\n  Status: z* concordant intre Alg3 si Alg4? "
        << (sameZ ? "DA" : "NU") << "\n";
}

static void test_NewArc()
{
    printSeparator("TEST 8: Algorithm 3 - Retea cu arc nou potential (A^a)");
    std::cout << "  Reteaua initiala: 1->2->3 cu capacitate bottleneck = 5\n";
    std::cout << "  Arc nou potential: 1->3 direct (isNew=true)\n";
    std::cout << "  Cu buget W=200, algoritmul poate folosi arcul nou\n";

    Graph G(3, 1, 3);

    auto B = [](double c) {
        return [c](double x) { return x <= c ? 0.0 : 2.0 * (x - c); };
        };

    auto U = [](double c) {
        return [c](double x) { return x <= c ? 0.0 : 0.5 * (x - c) * (x - c); };
        };

    G.addEdge(Edge(1, 2, 5, 15, false, U(5), B(5)));
    G.addEdge(Edge(2, 3, 5, 15, false, U(5), B(5)));
    G.addEdge(Edge(1, 3, 0, 15, true, U(0), B(0)));

    AMCPEBCP solver;
    AMCPEBCP_Result res = solver.solve(G, 200.0);

    std::cout << "\n  [Alg3 / arc nou / W=200]\n";
    std::cout << "  z* =      " << std::fixed << std::setprecision(4) << res.maxCapacity << "\n";
    std::cout << "  Cost:     " << res.finalCost << "\n";
    std::cout << "  Drum P*:  ";
    printPath(res.path);
    std::cout << "  Asteptat: z* = 15.0 (c_hat maxim in retea)\n";

    bool ok = std::abs(res.maxCapacity - 15.0) < 0.01;
    std::cout << "  Status:   " << (ok ? "CORECT" : "DIFERIT DE ASTEPTAT") << "\n";
}

void runAllManualTests()
{
    std::cout << "\n";
    std::cout << "************************************************************\n";
    std::cout << "   TESTE MANUALE - Licenta (Deaconu & Tayyebi, 2024)       \n";
    std::cout << "************************************************************\n";

    test_Algorithm1_Fig1();
    test_Algorithm1_NoExpansionNeeded();
    test_Algorithm1_Infeasible();
    test_Algorithm3_Fig5();
    test_Algorithm3_ZeroBudget();
    test_Algorithm4_ExactSolution();
    test_Comparison_Alg3_vs_Alg4();
    test_NewArc();

    std::cout << "\n************************************************************\n";
    std::cout << "   TOATE TESTELE MANUALE FINALIZATE\n";
    std::cout << "************************************************************\n\n";
}
