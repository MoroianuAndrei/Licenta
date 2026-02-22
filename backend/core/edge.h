#pragma once
#include <functional>

class Edge {
private:
    int from;        // Nodul sursa i
    int to;          // Nodul destinatie j

    double c;        // Capacitatea curenta c(w)
    double c_hat;    // Capacitatea maxima admisa ĉ(w)
    bool isNewEdge;      // true daca este un arc adaugat (A^a), false daca este arc existent (A)

    // Functiile de cost u(w, z) si b(w, z) 
    // Primesc ca parametru capacitatea dorita 'z' si returneaza costul
    std::function<double(double)> u;
    std::function<double(double)> b;

public:
    // Constructor
    Edge(int from, int to, double capacity, double max_capacity, bool isNew,
        std::function<double(double)> func_u, std::function<double(double)> func_b);

    // Getteri
    int getFrom() const;
    int getTo() const;
    double getCapacity() const;
    double getMaxCapacity() const;
    bool isNew() const;

    // Setteri cu validare
    void setCapacity(double newCapacity);
    void setMaxCapacity(double newMaxCapacity);

    // Metoda utilitara pentru a calcula costul de sumare pt un anumit z
    double getSumCost(double z) const;

    // Metoda utilitara pentru a calcula costul maxim pt un anumit z
    double getMaxCost(double z) const;
};