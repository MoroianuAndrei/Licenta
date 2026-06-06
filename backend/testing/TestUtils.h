#pragma once
#include <string>
#include <vector>

void printSeparator(const std::string& title);
void printPath(const std::vector<int>& path);

void printResult(const std::string& label,
    bool feasible,
    double cost,
    const std::vector<int>& path,
    double expected_cost = -1.0,
    const std::string& expected_path = "");
