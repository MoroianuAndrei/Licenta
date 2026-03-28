#include "edge.h"
#include <utility>

// ============================================================================
// Constructor GENERAL
// ============================================================================
Edge::Edge(int from, int to, double capacity, double max_capacity, bool isNew,
    std::function<double(double)> func_u,
    std::function<double(double)> func_b)
    : from(from), to(to), c(capacity), c_hat(max_capacity),
    isNewEdge(isNew), linearRate(0.0),
    u(std::move(func_u)), b(std::move(func_b))
{
}

// ============================================================================
// Constructor LINIAR - Section 3.1 din paper
// b(w, z) = rate * (z - c(w))
// u(w, z) = 0
// ============================================================================
Edge::Edge(int from, int to, double capacity, double max_capacity, bool isNew,
    double rate)
    : from(from), to(to), c(capacity), c_hat(max_capacity),
    isNewEdge(isNew), linearRate(rate)
{
    double cap = capacity;
    double r = rate;

    // b(w, z) = r * (z - c),  0 daca z <= c  (Formula 4 din paper)
    b = [cap, r](double z) -> double {
        return (z <= cap) ? 0.0 : r * (z - cap);
        };

    // u(w, z) = 0 in cazul liniar (Section 3.1)
    u = [](double) -> double { return 0.0; };
}

// ============================================================================
// Getteri / Setteri
// ============================================================================
int    Edge::getFrom()        const { return from; }
int    Edge::getTo()          const { return to; }
double Edge::getCapacity()    const { return c; }
double Edge::getMaxCapacity() const { return c_hat; }
bool   Edge::isNew()          const { return isNewEdge; }

void Edge::setCapacity(double newCapacity)
{
    if (newCapacity >= 0.0) c = newCapacity;
}

void Edge::setMaxCapacity(double newMaxCapacity)
{
    if (newMaxCapacity >= 0.0) c_hat = newMaxCapacity;
}

double Edge::getSumCost(double z) const { return b(z); }
double Edge::getMaxCost(double z) const { return u(z); }

// r_w: daca a fost setat explicit (constructor liniar) il returnam direct.
// Altfel il calculam prin diferenta finita - corect pentru orice b liniara.
double Edge::getLinearRate() const
{
    if (linearRate != 0.0) return linearRate;
    return getSumCost(c + 1.0) - getSumCost(c);
}