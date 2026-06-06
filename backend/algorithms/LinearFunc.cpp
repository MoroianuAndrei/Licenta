#include "LinearFunc.h"

LinearFunc::LinearFunc(double rate, double base) : r(rate), q(base) {}

double LinearFunc::eval(double z) const { return r * z + q; }

LinearFunc LinearFunc::operator+(const LinearFunc& o) const
{
    return { r + o.r, q + o.q };
}

LinearFunc LinearFunc::operator-(const LinearFunc& o) const
{
    return { r - o.r, q - o.q };
}

bool LinearFunc::isInfinity() const
{
    return r == std::numeric_limits<double>::infinity()
        || q == std::numeric_limits<double>::infinity();
}

LinearFunc LinearFunc::Infinity()
{
    return { std::numeric_limits<double>::infinity(),
             std::numeric_limits<double>::infinity() };
}
