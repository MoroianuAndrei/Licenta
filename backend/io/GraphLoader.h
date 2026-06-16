#pragma once

#include "backend/core/graph.h"
#include <string>

enum class CostModel
{
    ArticleGeneral,
    Linear
};

Graph loadGraphFromCSV(
    const std::string& filename,
    CostModel costModel = CostModel::ArticleGeneral,
    int source = 1,
    int target = -1
);

