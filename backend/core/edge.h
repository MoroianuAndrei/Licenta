#pragma once
#include <functional>

class Edge
{
private:
    int    from;
    int    to;
    double c;          // Capacitatea curenta c(w)
    double c_hat;      // Capacitatea maxima admisa c_hat(w)
    bool   isNewEdge;  // true daca arcul apartine lui A^a (arc potential nou)
    double linearRate; // Rata r_w pentru cazul liniar b(w,z) = r_w*(z - c(w))

    std::function<double(double)> u; // Cost de tip max:  u(w, z)
    std::function<double(double)> b; // Cost de tip suma: b(w, z)

public:
    // -----------------------------------------------------------------------
    // Constructor GENERAL - functii de cost arbitrare (Algorithm 1 si 3)
    // u si b trebuie sa fie non-descrescatoare monotone pe [z, +inf)
    // -----------------------------------------------------------------------
    Edge(int from, int to, double capacity, double max_capacity, bool isNew,
        std::function<double(double)> func_u,
        std::function<double(double)> func_b);

    // -----------------------------------------------------------------------
    // Constructor LINIAR - cazul special din Section 3.1 (Algorithm 4)
    // b(w, z) = rate * (z - c(w)),  u(w, z) = 0
    // -----------------------------------------------------------------------
    Edge(int from, int to, double capacity, double max_capacity, bool isNew,
        double rate);

    int    getFrom()        const;
    int    getTo()          const;
    double getCapacity()    const;
    double getMaxCapacity() const;
    bool   isNew()          const;

    void setCapacity(double newCapacity);
    void setMaxCapacity(double newMaxCapacity);

    double getSumCost(double z)  const; // b(w, z)
    double getMaxCost(double z)  const; // u(w, z)
    double getLinearRate()       const; // r_w
};