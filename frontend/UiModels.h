#pragma once

#include <QString>
#include <vector>

struct VisualEdge
{
    int from = 0;
    int to = 0;
    double capacity = 0.0;
    double maxCapacity = 0.0;
    bool isNew = false;
    double rate = 0.0;
};

struct AlgorithmViewState
{
    QString algorithm;
    double inputValue = 0.0;
    bool hasResult = false;
    double resultZ = 0.0;
    double finalCost = 0.0;
    std::vector<int> path;
};

inline bool edgeIsInPath(const VisualEdge& edge, const std::vector<int>& path)
{
    for (size_t i = 0; i + 1 < path.size(); ++i)
        if (path[i] == edge.from && path[i + 1] == edge.to)
            return true;
    return false;
}
