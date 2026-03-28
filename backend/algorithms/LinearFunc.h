#pragma once
#include <limits>

// f(z) = r * z + q
struct LinearFunc {
    double r; // panta
    double q; // termen liber

    LinearFunc(double rate = 0.0, double base = 0.0);
    double eval(double z) const;
    LinearFunc operator+(const LinearFunc& o) const;
    LinearFunc operator-(const LinearFunc& o) const;
    bool isInfinity() const;
    static LinearFunc Infinity();
};