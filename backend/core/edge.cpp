#include "edge.h"
#include <utility>

Edge::Edge(int from, int to, double capacity, double max_capacity, bool isNew,
    std::function<double(double)> func_u,
    std::function<double(double)> func_b)
    : from_(from), to_(to), capacity_(capacity), maxCapacity_(max_capacity),
    isNew_(isNew), linearRate_(0.0),
    u_(std::move(func_u)), b_(std::move(func_b))
{
}

Edge::Edge(int from, int to, double capacity, double max_capacity, bool isNew,
    double rate)
    : from_(from), to_(to), capacity_(capacity), maxCapacity_(max_capacity),
    isNew_(isNew), linearRate_(rate)
{
    double cap = capacity;
    double r = rate;

    b_ = [cap, r](double z) -> double {
        return (z <= cap) ? 0.0 : r * (z - cap);
        };

    u_ = [](double) -> double { return 0.0; };
}

int Edge::getFrom() const { return from_; }
int Edge::getTo() const { return to_; }
double Edge::getCapacity() const { return capacity_; }
double Edge::getMaxCapacity() const { return maxCapacity_; }
bool Edge::isNew() const { return isNew_; }

double Edge::getSumCost(double z) const { return b_(z); }
double Edge::getMaxCost(double z) const { return u_(z); }

double Edge::getLinearRate() const
{
    if (linearRate_ != 0.0) return linearRate_;
    return getSumCost(capacity_ + 1.0) - getSumCost(capacity_);
}
