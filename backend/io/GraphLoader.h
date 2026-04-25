#pragma once

#include "backend/core/graph.h"
#include <string>

// CostModel stabileste cum sunt construite functiile de cost b(w,z) si u(w,z)
// pentru arcele citite din CSV.
enum class CostModel
{
    ArticleGeneral, // Pentru Algoritmul 1 si 3: functiile neliniare din articol
    Linear          // Pentru Algoritmul 4: b(w,z)=rate*max(0,z-c(w)), u(w,z)=0
};

Graph loadGraphFromCSV(
    const std::string& filename,
    CostModel costModel = CostModel::ArticleGeneral
);
