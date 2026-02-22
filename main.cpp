#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <cmath>

#include "backend/core/Graph.h"
#include "backend/algorithms/AMCMCPEP.h" // Asigura-te ca path-ul e corect (core sau algorithms, depinde cum l-ai salvat)

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    qDebug() << "--- INCEPEM TESTUL DE BACKEND PENTRU ALGORITMUL 1 ---";

    // Instantiem graful cu 7 noduri, sursa = 1, destinatia = 7
    Graph G(7, 1, 7);
    double z = 20.0;

    // --- FORMULELE MATEMATICE DIN LUCRARE ---
    // 'd' reprezinta factorul de distanta |j-i|, dedus matematic din Figura 2 [cite: 286-289, 304-322]

    // Functii de cost pentru arcele existente (c < z) [cite: 286]
    auto u_exist = [](double d, double c, double x) { return d * std::pow(x - c, 2); };
    auto b_exist = [](double d, double c, double x) { return 10.0 * d * (x - c); };

    // Functii de cost pentru arcele potential noi adaugate (din A^a) [cite: 288]
    auto u_new = [](double d, double x) { return d * std::pow(x - 10.0, 2); };
    auto b_new = [](double d, double x) { return 4.0 * d * x; };


    // 1. Arce cu capacitate curenta >= z (costul va fi automat 0) [cite: 90, 91, 304-322]
    G.addEdge(Edge(1, 3, 23.0, 100.0, false, [](double) {return 0; }, [](double) {return 0; }));
    G.addEdge(Edge(4, 6, 20.0, 100.0, false, [](double) {return 0; }, [](double) {return 0; }));
    G.addEdge(Edge(4, 5, 21.0, 100.0, false, [](double) {return 0; }, [](double) {return 0; }));

    // 2. Arce care NU pot atinge z=20 (liniile punctate din lucrare) [cite: 258, 259, 304-322]
    // Setam capacitatea maxima (c_hat) la 10.0 ca sa fie filtrate din constructia lui G^E [cite: 82-84]
    G.addEdge(Edge(1, 2, 8.0, 10.0, false, [](double) {return 0; }, [](double) {return 0; }));
    G.addEdge(Edge(6, 5, 8.0, 10.0, false, [](double) {return 0; }, [](double) {return 0; }));

    // 3. ARCE EXISTENTE CARE TREBUIE EXTINSE (Liniile solide cu capacitate < 20) 
    G.addEdge(Edge(1, 4, 12.0, 100.0, false,
        [=](double x) { return u_exist(3.0, 12.0, x); },
        [=](double x) { return b_exist(3.0, 12.0, x); }
    ));

    G.addEdge(Edge(3, 2, 10.0, 100.0, false,
        [=](double x) { return u_exist(1.0, 10.0, x); },
        [=](double x) { return b_exist(1.0, 10.0, x); }
    ));

    G.addEdge(Edge(3, 4, 11.0, 100.0, false,
        [=](double x) { return u_exist(2.0, 11.0, x); },
        [=](double x) { return b_exist(2.0, 11.0, x); }
    ));

    // Pentru arcul (3, 6), din calculele autorilor, distantele pentru u si b difera usor 
    G.addEdge(Edge(3, 6, 16.0, 100.0, false,
        [=](double x) { return u_exist(6.75, 16.0, x); },
        [=](double x) { return b_exist(4.5, 16.0, x); }
    ));

    G.addEdge(Edge(2, 5, 15.0, 100.0, false,
        [=](double x) { return u_exist(3.0, 15.0, x); },
        [=](double x) { return b_exist(3.0, 15.0, x); }
    ));

    G.addEdge(Edge(6, 7, 11.0, 100.0, false,
        [=](double x) { return u_exist(2.0, 11.0, x); },
        [=](double x) { return b_exist(2.0, 11.0, x); }
    ));

    G.addEdge(Edge(5, 7, 9.0, 100.0, false,
        [=](double x) { return u_exist(1.0, 9.0, x); },
        [=](double x) { return b_exist(1.0, 9.0, x); }
    ));


    // 4. ARCE POTENTIAL NOI (A^a) adaugate - Liniile intrerupte [cite: 98-105, 304-322]
    G.addEdge(Edge(2, 4, 0.0, 100.0, true,
        [=](double x) { return u_new(1.0, x); },
        [=](double x) { return b_new(1.0, x); }
    ));

    G.addEdge(Edge(5, 6, 0.0, 100.0, true,
        [=](double x) { return u_new(1.44, x); },
        [=](double x) { return b_new(1.5, x); }
    ));

    G.addEdge(Edge(6, 5, 0.0, 100.0, true,
        [=](double x) { return u_new(1.0, x); },
        [=](double x) { return b_new(1.0, x); }
    ));


    // --- RULAM ALGORITMUL 1 ---
    AMCMCPEP solver;
    AMCMCPEP_Result rezultat = solver.solve(G, z);

    // --- AFISARE REZULTATE IN CONSOLA ---
    if (rezultat.isFeasible) {
        qDebug() << "Succes! Drum gasit.";
        qDebug() << "Costul total (M(t) + S(t)) calculat:" << rezultat.totalCost;

        QString pathStr;
        for (int i = 0; i < rezultat.path.size(); ++i) {
            pathStr += QString::number(rezultat.path[i]);
            if (i < rezultat.path.size() - 1) pathStr += " -> ";
        }
        qDebug() << "Calea gasita (P*):" << pathStr;
    }
    else {
        qDebug() << "Infezabil: Nu se poate ajunge la z=" << z << " cu bugetul/limitele actuale.";
    }

    qDebug() << "--- TEST FINALIZAT ---";

    MainWindow w;
    w.show();
    return a.exec();
}