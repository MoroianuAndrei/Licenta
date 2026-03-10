#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>

// Include-urile din backend (calea relativa din radacina proiectului)
#include "backend/core/graph.h"
#include "backend/algorithms/AMCMCPEP.h"
#include "backend/algorithms/AMCPEBCP.h"
#include "backend/algorithms/ParametricSearch.h"

// ============================================================================
// Utilitare de afisare
// ============================================================================
void printSeparator(const std::string& title)
{
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "============================================================\n";
}

void printPath(const std::vector<int>& path)
{
    if (path.empty()) { std::cout << "(drum gol)\n"; return; }
    for (int i = 0; i < (int)path.size(); ++i)
    {
        std::cout << path[i];
        if (i + 1 < (int)path.size()) std::cout << " -> ";
    }
    std::cout << "\n";
}

void printResult(const std::string& label, bool feasible,
    double cost, const std::vector<int>& path,
    double expected_cost = -1, const std::string& expected_path = "")
{
    std::cout << "\n  [" << label << "]\n";
    std::cout << "  Fezabil:      " << (feasible ? "DA" : "NU") << "\n";
    if (feasible)
    {
        std::cout << "  Cost total:   " << std::fixed << std::setprecision(4) << cost << "\n";
        std::cout << "  Drum P*:      "; printPath(path);
        if (expected_cost >= 0)
            std::cout << "  Asteptat:     cost = " << expected_cost
            << (expected_path.empty() ? "" : ", drum = " + expected_path) << "\n";

        bool ok = (expected_cost < 0) || (std::abs(cost - expected_cost) < 0.01);
        std::cout << "  Status:       " << (ok ? "CORECT" : "DIFERIT DE ASTEPTAT") << "\n";
    }
}

// ============================================================================
// TEST 1 - Algorithm 1 (AMCMCPEP)
// Exemplul din Fig.1 + Table 1 din paper
// Retea: 7 noduri, s=1, t=7, z=20
// Rezultat asteptat: cost = 411, P* = 1->3->2->4->6->7
// ============================================================================
void test_Algorithm1_Fig1()
{
    printSeparator("TEST 1: Algorithm 1 (AMCMCPEP) - Fig.1 din paper");
    std::cout << "  Retea: 7 noduri, s=1, t=7\n";
    std::cout << "  Scop: extinde reteaua astfel incat MCP sa aiba capacitatea z=20\n";
    std::cout << "  Costuri: b(w,x) = 10*|j-i|*(x-c), u(w,x) = |j-i|*(x-c)^2\n";

    Graph G(7, 1, 7);
    const double z = 20.0;
    const double INF = 1e9;

    // Functii de cost pentru arcele existente (reconstituite din Table 1)
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

    // Arcele reconstituite exact din Table 1 (verificate manual):
    // arc(1,3): c=23 >= z=20  => cost 0 ✓
    // arc(1,4): c=12, u=|4-1|*(20-12)^2=192, b=10*3*8=240 ✓
    // arc(3,2): c=10, u=|2-3|*(20-10)^2=100, b=10*1*10=100 ✓
    // arc(3,6): c=14, u=|6-3|*(20-14)^2=108, b=10*3*6=180 ✓
    // arc(2,4): c=16, u=|4-2|*(20-16)^2=32<=100, b=10*2*4=80  (max{M,u}=100) ✓
    // arc(2,5): c=15, u=|5-2|*(20-15)^2=75<=100, b=10*3*5=150 ✓
    // arc(4,6): c=20 >= z=20  => cost 0 ✓
    // arc(6,7): c=9,  u=|7-6|*(20-9)^2=121, b=10*1*11=110 ✓
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

    printResult("Alg1 / Fig.1 / z=20", res.isFeasible, res.totalCost, res.path,
        411.0, "1->3->2->4->6->7");
}

// ============================================================================
// TEST 2 - Algorithm 1: retea deja suficienta (nu are nevoie de expansiune)
// ============================================================================
void test_Algorithm1_NoExpansionNeeded()
{
    printSeparator("TEST 2: Algorithm 1 - Reteaua nu are nevoie de expansiune");
    std::cout << "  Retea: 3 noduri, s=1, t=3\n";
    std::cout << "  Toate arcele au c(w) >= z => cost = 0\n";

    Graph G(3, 1, 3);
    const double INF = 1e9;

    auto B = [](double c) { return [c](double x) { return x <= c ? 0.0 : 5.0 * (x - c); }; };
    auto U = [](double c) { return [c](double x) { return x <= c ? 0.0 : 2.0 * (x - c) * (x - c); }; };

    G.addEdge(Edge(1, 2, 30, INF, false, U(30), B(30))); // c=30 >= z=20
    G.addEdge(Edge(2, 3, 25, INF, false, U(25), B(25))); // c=25 >= z=20

    AMCMCPEP solver;
    AMCMCPEP_Result res = solver.solve(G, 20.0);

    printResult("Alg1 / fara expansiune / z=20", res.isFeasible, res.totalCost, res.path, 0.0);
}

// ============================================================================
// TEST 3 - Algorithm 1: problema infezabila (t inaccesibil din s)
// ============================================================================
void test_Algorithm1_Infeasible()
{
    printSeparator("TEST 3: Algorithm 1 - Problema infezabila");
    std::cout << "  Retea: 3 noduri, s=1, t=3, dar t nu e accesibil din s\n";
    std::cout << "  Asteptat: isFeasible = false\n";

    Graph G(3, 1, 3);
    auto B = [](double c) { return [c](double x) { return x <= c ? 0.0 : 5.0 * (x - c); }; };
    auto U = [](double c) { return [c](double x) { return x <= c ? 0.0 : 2.0 * (x - c) * (x - c); }; };

    // Arc (1->2) exista, dar nu exista arc spre nodul 3
    G.addEdge(Edge(1, 2, 5, 20, false, U(5), B(5)));

    AMCMCPEP solver;
    AMCMCPEP_Result res = solver.solve(G, 15.0);

    std::cout << "\n  [Alg1 / infezabil]\n";
    std::cout << "  Fezabil:  " << (res.isFeasible ? "DA" : "NU") << "\n";
    std::cout << "  Status:   " << (!res.isFeasible ? "CORECT (infezabil asteptat)" : "GRESIT") << "\n";
}

// ============================================================================
// TEST 4 - Algorithm 3 (AMCPEBCP)
// Exemplul din Fig.5 din paper: 5 noduri, s=1, t=5, W=24, costuri liniare
// Rezultat asteptat: z* ≈ 13.7778, P* = 1->2->3->5
// ============================================================================
void test_Algorithm3_Fig5()
{
    printSeparator("TEST 4: Algorithm 3 (AMCPEBCP) - Fig.5 din paper");
    std::cout << "  Retea: 5 noduri, s=1, t=5, buget W=24\n";
    std::cout << "  Costuri liniare: b(w,z) = r*(z-c)\n";
    std::cout << "  Asteptat: z* = 13.7778, P* = 1->2->3->5\n";

    Graph G(5, 1, 5);

    // Date reconstituite din Table 2 si Table 3 ale paper-ului:
    // arc(1,2): c=12, c_hat=15, r=5  => b=5*(z-12)=5z-60  ✓ (s(2)=5z-60)
    // arc(1,4): c=8,  c_hat=15, r=4  => b=4*(z-8)=4z-32   ✓ (s(4)=4z-32)
    // arc(2,3): c=10, c_hat=15, r=4  => b=4*(z-10)=4z-40, s(2)+b = 9z-100 ✓
    // arc(3,5): c=14, c_hat=15, r=6  => in [12,14): b=0 (c=14>z), deci s(5)=s(3) ✓
    // arc(4,5): c=8,  c_hat=15, r=8  => b=8*(z-8)
    // arc(2,4): c=6,  c_hat=15, r=6  => pentru conectivitate
    G.addEdge(Edge(1, 2, 12, 15, false, 5.0));
    G.addEdge(Edge(1, 4, 8, 15, false, 4.0));
    G.addEdge(Edge(2, 3, 10, 15, false, 4.0));
    G.addEdge(Edge(3, 5, 14, 15, false, 6.0));
    G.addEdge(Edge(4, 5, 8, 15, false, 8.0));
    G.addEdge(Edge(2, 4, 6, 15, false, 6.0));

    AMCPEBCP solver;
    AMCPEBCP_Result res = solver.solve(G, 24.0);

    std::cout << "\n  [Alg3 / Fig.5 / W=24]\n";
    std::cout << "  z* =          " << std::fixed << std::setprecision(4) << res.maxCapacity << "\n";
    std::cout << "  Cost:         " << res.finalCost << "\n";
    std::cout << "  Drum P*:      "; printPath(res.path);
    std::cout << "  Asteptat:     z* = 13.7778, drum = 1->2->3->5\n";
    bool ok = std::abs(res.maxCapacity - 13.7778) < 0.01;
    std::cout << "  Status:       " << (ok ? "CORECT" : "DIFERIT DE ASTEPTAT") << "\n";
}

// ============================================================================
// TEST 5 - Algorithm 3: buget zero (nu putem imbunatati nimic)
// ============================================================================
void test_Algorithm3_ZeroBudget()
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
    std::cout << "  Drum P*:  "; printPath(res.path);
    // zmin = min(10,8,12) pe drumul 1->2->3->4 = 8
    std::cout << "  Status:   " << (res.finalCost <= 0.001 ? "CORECT (cost = 0)" : "GRESIT") << "\n";
}

// ============================================================================
// TEST 6 - Algorithm 4 (ParametricSearch)
// Retea simpla unde Faza II poate calcula exact z*
// arc(1,2): c=5, c_hat=20, r=3  => b=3*(z-5)
// arc(2,3): c=8, c_hat=20, r=4  => b=4*(z-8)
// W=30: S(3) = 3*(z-5)+4*(z-8) = 7z-47 = 30 => z*=(30+47)/7 = 11
// ============================================================================
void test_Algorithm4_ExactSolution()
{
    printSeparator("TEST 6: Algorithm 4 (ParametricSearch) - Solutie exacta verificabila");
    std::cout << "  Retea: 3 noduri, s=1, t=3, W=30\n";
    std::cout << "  arc(1,2): c=5, r=3;  arc(2,3): c=8, r=4\n";
    std::cout << "  S(t)(z) = 3*(z-5) + 4*(z-8) = 7z - 47 = 30  =>  z* = 77/7 = 11.0\n";

    Graph G(3, 1, 3);
    // zmin = min(5,8) = 5, zmax = 20
    // La z=11: cost = 3*(11-5)+4*(11-8) = 18+12 = 30 = W ✓
    G.addEdge(Edge(1, 2, 5, 20, false, 3.0));
    G.addEdge(Edge(2, 3, 8, 20, false, 4.0));

    ParametricSearch solver;
    ParametricResult res = solver.solve(G, 30.0);

    std::cout << "\n  [Alg4 / retea simpla / W=30]\n";
    std::cout << "  z* =          " << std::fixed << std::setprecision(6) << res.maxCapacity << "\n";
    std::cout << "  Cost:         " << std::setprecision(4) << res.finalCost << "\n";
    std::cout << "  Drum P*:      "; printPath(res.path);
    std::cout << "  Asteptat:     z* = 11.0, cost = 30.0, drum = 1->2->3\n";
    bool ok = std::abs(res.maxCapacity - 11.0) < 0.01;
    std::cout << "  Status:       " << (ok ? "CORECT" : "DIFERIT DE ASTEPTAT") << "\n";
}

// ============================================================================
// TEST 7 - Comparatie Alg3 vs Alg4 pe o retea custom
// Verifica ca ambii algoritmi dau acelasi z* pe date noi
// ============================================================================
void test_Comparison_Alg3_vs_Alg4()
{
    printSeparator("TEST 7: Comparatie Algorithm 3 vs Algorithm 4 - Retea custom");
    std::cout << "  Retea: 4 noduri, s=1, t=4, W=50\n";
    std::cout << "  Verificam ca Alg3 si Alg4 dau acelasi z*\n";

    // Construim aceeasi retea pentru ambii
    auto buildGraph = []() {
        Graph G(4, 1, 4);
        // arc(1,2): c=5, c_hat=20, r=3
        // arc(1,3): c=8, c_hat=20, r=2
        // arc(2,4): c=6, c_hat=20, r=4
        // arc(3,4): c=7, c_hat=20, r=2
        // arc(2,3): c=4, c_hat=20, r=1
        G.addEdge(Edge(1, 2, 5, 20, false, 3.0));
        G.addEdge(Edge(1, 3, 8, 20, false, 2.0));
        G.addEdge(Edge(2, 4, 6, 20, false, 4.0));
        G.addEdge(Edge(3, 4, 7, 20, false, 2.0));
        G.addEdge(Edge(2, 3, 4, 20, false, 1.0));
        return G;
        };

    Graph G3 = buildGraph();
    Graph G4 = buildGraph();

    AMCPEBCP       solver3;
    ParametricSearch solver4;

    AMCPEBCP_Result  res3 = solver3.solve(G3, 50.0);
    ParametricResult res4 = solver4.solve(G4, 50.0);

    std::cout << "\n  Algorithm 3:  z* = " << std::fixed << std::setprecision(4)
        << res3.maxCapacity << ",  cost = " << res3.finalCost << "\n";
    std::cout << "  Drum Alg3:    "; printPath(res3.path);

    std::cout << "\n  Algorithm 4:  z* = " << std::fixed << std::setprecision(4)
        << res4.maxCapacity << ",  cost = " << res4.finalCost << "\n";
    std::cout << "  Drum Alg4:    "; printPath(res4.path);

    bool sameZ = std::abs(res3.maxCapacity - res4.maxCapacity) < 0.1;
    std::cout << "\n  Status: z* concordant intre Alg3 si Alg4? "
        << (sameZ ? "DA" : "NU") << "\n";
}

// ============================================================================
// TEST 8 - Retea cu arc nou (A^a)
// arc nou (1->3) cu b(w,z) = 4*z (cost de a-l adauga)
// arc(1,2) si arc(2,3) cu b(w,x) = 4*(x-c)
// W=200 acopera z*=15 pe drumul direct (1->3)
// ============================================================================
void test_NewArc()
{
    printSeparator("TEST 8: Algorithm 3 - Retea cu arc nou potential (A^a)");
    std::cout << "  Reteaua initiala: 1->2->3 cu capacitate bottleneck = 5\n";
    std::cout << "  Arc nou potential: 1->3 direct (isNew=true)\n";
    std::cout << "  Cu buget W=200, algoritmul poate folosi arcul nou\n";

    Graph G(3, 1, 3);
    auto B = [](double c) { return [c](double x) { return x <= c ? 0.0 : 2.0 * (x - c); }; };
    auto U = [](double c) { return [c](double x) { return x <= c ? 0.0 : 0.5 * (x - c) * (x - c); }; };

    // Arce existente: 1->2->3 cu capacitate 5, c_hat=15
    G.addEdge(Edge(1, 2, 5, 15, false, U(5), B(5)));  // b=2*(z-5)
    G.addEdge(Edge(2, 3, 5, 15, false, U(5), B(5)));  // b=2*(z-5)
    // Arc nou potential: 1->3 direct, c=0, c_hat=15
    G.addEdge(Edge(1, 3, 0, 15, true, U(0), B(0)));  // b=2*z

    // La z=15 pe drumul (1->2->3): cost = 2*(15-5)+2*(15-5) = 40 <= 200
    // La z=15 pe drumul (1->3) nou: cost = 2*(15-0) = 30 <= 200
    AMCPEBCP solver;
    AMCPEBCP_Result res = solver.solve(G, 200.0);

    std::cout << "\n  [Alg3 / arc nou / W=200]\n";
    std::cout << "  z* =      " << std::fixed << std::setprecision(4) << res.maxCapacity << "\n";
    std::cout << "  Cost:     " << res.finalCost << "\n";
    std::cout << "  Drum P*:  "; printPath(res.path);
    std::cout << "  Asteptat: z* = 15.0 (c_hat maxim in retea)\n";
    bool ok = std::abs(res.maxCapacity - 15.0) < 0.01;
    std::cout << "  Status:   " << (ok ? "CORECT" : "DIFERIT DE ASTEPTAT") << "\n";
}

// ============================================================================
// MAIN
// ============================================================================
int main()
{
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n";
    std::cout << "************************************************************\n";
    std::cout << "   TESTE ALGORITMI - Licenta (Deaconu & Tayyebi, 2024)     \n";
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
    std::cout << "   TOATE TESTELE FINALIZATE\n";
    std::cout << "************************************************************\n\n";

    return 0;
}