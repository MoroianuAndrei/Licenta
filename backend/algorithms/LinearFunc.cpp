#include "LinearFunc.h"

// Constructor
LinearFunc::LinearFunc(double rate, double base) : r(rate), q(base) {}

// Evaluarea in punctul z
double LinearFunc::eval(double z) const {
    return r * z + q;
}

// Operatorul de adunare
LinearFunc LinearFunc::operator+(const LinearFunc& other) const {
    return LinearFunc(r + other.r, q + other.q);
}

// Operatorul de scadere
LinearFunc LinearFunc::operator-(const LinearFunc& other) const {
    return LinearFunc(r - other.r, q - other.q);
}

// Verificare daca este infinit
bool LinearFunc::isInfinity() const {
    return r == std::numeric_limits<double>::infinity() ||
        q == std::numeric_limits<double>::infinity();
}

// Returneaza un obiect setat pe infinit
LinearFunc LinearFunc::Infinity() {
    return LinearFunc(std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::infinity());
}