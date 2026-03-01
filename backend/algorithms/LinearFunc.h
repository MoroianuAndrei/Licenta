#pragma once
#include <cmath>
#include <limits>

// Reprezinta o functie liniara de cost: f(z) = r * z + q
struct LinearFunc {
    double r; // Panta (Rate / costul per unitate)
    double q; // Termenul liber (Baza)

    // Constructor
    LinearFunc(double rate = 0.0, double base = 0.0);

    // Evalueaza functia pentru o valoare z (returneaza costul in acel punct)
    double eval(double z) const;

    // Adunarea a doua functii (S(i) + b(w, z))
    LinearFunc operator+(const LinearFunc& other) const;

    // Scaderea a doua functii (pentru a afla forma u - vz)
    LinearFunc operator-(const LinearFunc& other) const;
    
    // Verificam daca functia reprezinta "infinit" (nod inaccesibil)
    bool isInfinity() const;
    
    // Creaza o functie setata la infinit
    static LinearFunc Infinity();
};