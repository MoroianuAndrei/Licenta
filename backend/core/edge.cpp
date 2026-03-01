#include "edge.h"
#include <utility>

Edge::Edge(int from, int to, double capacity, double max_capacity, bool isNew,
           std::function<double(double)> func_u, std::function<double(double)> func_b)
    : from(from), to(to), c(capacity), c_hat(max_capacity), isNewEdge(isNew), 
      u(std::move(func_u)), b(std::move(func_b)) {}

int Edge::getFrom() const { return from; }
int Edge::getTo() const { return to; }
double Edge::getCapacity() const { return c; }
double Edge::getMaxCapacity() const { return c_hat; }
bool Edge::isNew() const { return isNewEdge; }

void Edge::setCapacity(double newCapacity)
{
    if (newCapacity >= 0.0)
    {
        c = newCapacity;
    }
}

void Edge::setMaxCapacity(double newMaxCapacity)
{
    if (newMaxCapacity >= 0.0)
    {
        c_hat = newMaxCapacity;
    }
}

double Edge::getSumCost(double z) const { return b(z); }
double Edge::getMaxCost(double z) const { return u(z); }

double Edge::getLinearRate() const
{
    // Pentru a afla panta 'r', calculam cat ne costa sa extindem cu exact 1 unitate
    // dincolo de capacitatea gratuita curenta (c).
    // b(c + 1) - b(c) ne da mereu panta intr-o ecuatie liniara.
    return getSumCost(c + 1.0) - getSumCost(c);
}