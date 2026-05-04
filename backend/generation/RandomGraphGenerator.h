#pragma once

#include <string>
#include <vector>
#include <random>

struct RandomGraphParams {
    int nodes = 50;
    int paths = 10;
    int cycles = 5;
    double extraArcProbability = 0.05;

    int minCapacity = 10;
    int maxCapacity = 100;
    int maxCapacityLimit = 200;

    int minRate = 1;
    int maxRate = 20;

    double newArcProbability = 0.10;

    unsigned int seed = 0;
};

class RandomGraphGenerator {
public:
    explicit RandomGraphGenerator(RandomGraphParams params);

    void generateToCSV(const std::string& filePath);

private:
    RandomGraphParams params;
    std::mt19937 rng;
    std::vector<std::vector<int>> adjacency;

    int randomInt(int minValue, int maxValue);
    double randomDouble();

    std::vector<int> shuffledNodes(int start, int end);

    void addArc(int from, int to);
    bool hasArc(int from, int to) const;

    void generateRandomSTPath();
    void generateRandomCycle();
    void addRandomExtraArcs();

    void writeCSV(const std::string& filePath);
};