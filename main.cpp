#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <cmath>

#include "backend/core/Graph.h"
// Includem ambii algoritmi
#include "backend/algorithms/AMCMCPEP.h" 
#include "backend/algorithms/AMCPEBCP.h" 

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    qDebug() << "--- INCEPEM TESTUL DE BACKEND PENTRU ALGORITMUL 3 ---";

    // Instantiem graful cu 7 noduri, sursa = 1, destinatia = 7
    Graph G(7, 1, 7);

    // --- FORMULELE MATEMATICE DIN LUCRARE ---
    auto u_exist = [](double d, double c, double x) { return d * std::pow(x - c, 2); };
    auto b_exist = [](double d, double c, double x) { return 10.0 * d * (x - c); };
    auto u_new = [](double d, double x) { return d * std::pow(x - 10.0, 2); };
    auto b_new = [](double d, double x) { return 4.0 * d * x; };

    // 1. Arce cu capacitate curenta >= z 
    G.addEdge(Edge(1, 3, 23.0, 100.0, false, [](double) {return 0; }, [](double) {return 0; }));
    G.addEdge(Edge(4, 6, 20.0, 100.0, false, [](double) {return 0; }, [](double) {return 0; }));
    G.addEdge(Edge(4, 5, 21.0, 100.0, false, [](double) {return 0; }, [](double) {return 0; }));

    // 2. Arce care NU pot atinge o capacitate mare (c_hat = 10.0)
    G.addEdge(Edge(1, 2, 8.0, 10.0, false, [](double) {return 0; }, [](double) {return 0; }));
    G.addEdge(Edge(6, 5, 8.0, 10.0, false, [](double) {return 0; }, [](double) {return 0; }));

    // 3. ARCE EXISTENTE CARE TREBUIE EXTINSE 
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

    // 4. ARCE POTENTIAL NOI (A^a) 
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


    // ==========================================================
    // --- RULAM ALGORITMUL 3 (MCPEBCP cu constrangere de buget) ---
    // ==========================================================

    // Setam un buget maxim disponibil
    // Din Algoritmul 1 stiam ca pentru z=20, costul era 411.
    // Daca avem un buget de doar 300, algoritmul va cauta o capacitate z < 20.
    double buget_W = 300.0;

    AMCPEBCP solverBuget;
    AMCPEBCP_Result rezultat_buget = solverBuget.solve(G, buget_W);

    qDebug() << "Buget alocat (W):" << buget_W;

    if (rezultat_buget.maxCapacity > 0) {
        qDebug() << "-> Capacitatea MAXIMA obtinuta (z):" << rezultat_buget.maxCapacity;
        qDebug() << "-> Costul final platit (trebuie sa fie <= W):" << rezultat_buget.finalCost;

        QString pathStrB;
        for (int i = 0; i < rezultat_buget.path.size(); ++i) {
            pathStrB += QString::number(rezultat_buget.path[i]);
            if (i < rezultat_buget.path.size() - 1) pathStrB += " -> ";
        }
        qDebug() << "-> Pe drumul (P*):" << pathStrB;
    }
    else {
        qDebug() << "Nu se poate asigura nicio conexiune de la S la T (retea intrerupta complet).";
    }

    qDebug() << "--- TEST FINALIZAT ---";

    MainWindow w;
    w.show();
    return a.exec();
}