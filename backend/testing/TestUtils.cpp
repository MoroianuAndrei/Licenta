#include "TestUtils.h"
#include <iostream>
#include <iomanip>
#include <cmath>

void printSeparator(const std::string& title)
{
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "============================================================\n";
}

void printPath(const std::vector<int>& path)
{
    if (path.empty())
    {
        std::cout << "(drum gol)\n";
        return;
    }

    for (int i = 0; i < (int)path.size(); ++i)
    {
        std::cout << path[i];
        if (i + 1 < (int)path.size())
            std::cout << " -> ";
    }
    std::cout << "\n";
}

void printResult(const std::string& label,
    bool feasible,
    double cost,
    const std::vector<int>& path,
    double expected_cost,
    const std::string& expected_path)
{
    std::cout << "\n  [" << label << "]\n";
    std::cout << "  Fezabil:      " << (feasible ? "DA" : "NU") << "\n";

    if (feasible)
    {
        std::cout << "  Cost total:   " << std::fixed << std::setprecision(4) << cost << "\n";
        std::cout << "  Drum P*:      ";
        printPath(path);

        if (expected_cost >= 0.0)
        {
            std::cout << "  Asteptat:     cost = " << expected_cost;
            if (!expected_path.empty())
                std::cout << ", drum = " << expected_path;
            std::cout << "\n";
        }

        bool ok = (expected_cost < 0.0) || (std::abs(cost - expected_cost) < 0.01);
        std::cout << "  Status:       " << (ok ? "CORECT" : "DIFERIT DE ASTEPTAT") << "\n";
    }
}
