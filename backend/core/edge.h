#pragma once
#include <functional>

class Edge
{
private:
    int from;
    int to;

    double c;
    double c_hat;
    bool isNewEdge;

    std::function<double(double)> u;
    std::function<double(double)> b;

public:
    Edge(int from, int to, double capacity, double max_capacity, bool isNew,
        std::function<double(double)> func_u, std::function<double(double)> func_b);

    int getFrom() const;
    int getTo() const;
    double getCapacity() const;
    double getMaxCapacity() const;
    bool isNew() const;

    void setCapacity(double newCapacity);
    void setMaxCapacity(double newMaxCapacity);

    double getSumCost(double z) const;

    double getMaxCost(double z) const;

    double getLinearRate() const;
};