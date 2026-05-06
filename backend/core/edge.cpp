#include "edge.h"
#include <utility>

Edge::Edge(int from, int to, double capacity, double max_capacity, bool isNew,
    std::function<double(double)> func_u,
    std::function<double(double)> func_b)
    : from(from), to(to), c(capacity), c_hat(max_capacity),
    isNewEdge(isNew), linearRate(0.0),
    u(std::move(func_u)), b(std::move(func_b))
{
}

Edge::Edge(int from, int to, double capacity, double max_capacity, bool isNew,
    double rate)
    : from(from), to(to), c(capacity), c_hat(max_capacity),
    isNewEdge(isNew), linearRate(rate)
{
    double cap = capacity;
    double r = rate;

    b = [cap, r](double z) -> double {
        return (z <= cap) ? 0.0 : r * (z - cap);
        };

    u = [](double) -> double { return 0.0; };
}

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

double Edge::getLinearRate() const
{
    if (linearRate != 0.0) return linearRate;
    return getSumCost(c + 1.0) - getSumCost(c);
}