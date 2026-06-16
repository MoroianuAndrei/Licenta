#pragma once
#include <functional>

class Edge
{
private:
    int from_;
    int to_;
    double capacity_;
    double maxCapacity_;
    bool isNew_;
    double linearRate_;

    std::function<double(double)> u_;
    std::function<double(double)> b_;

public:
    Edge(int from, int to, double capacity, double max_capacity, bool isNew,
        std::function<double(double)> func_u,
        std::function<double(double)> func_b);

    Edge(int from, int to, double capacity, double max_capacity, bool isNew,
        double rate);

    int getFrom() const;
    int getTo() const;
    double getCapacity() const;
    double getMaxCapacity() const;
    bool isNew() const;

    double getSumCost(double z) const;
    double getMaxCost(double z) const;
    double getLinearRate() const;
};
