#include "RandomGraphGenerator.h"

#include <fstream>
#include <algorithm>
#include <chrono>
#include <stdexcept>

RandomGraphGenerator::RandomGraphGenerator(RandomGraphParams params)
    : params(params)
{
    if (params.nodes < 2)
    {
        throw std::runtime_error("Random graph must have at least 2 nodes.");
    }

    if (params.seed == 0)
    {
        params.seed = static_cast<unsigned int>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count()
            );
        this->params.seed = params.seed;
    }

    rng.seed(params.seed);
    adjacency.assign(params.nodes + 1, std::vector<int>(params.nodes + 1, 0));
}

int RandomGraphGenerator::randomInt(int minValue, int maxValue)
{
    std::uniform_int_distribution<int> dist(minValue, maxValue);
    return dist(rng);
}

double RandomGraphGenerator::randomDouble()
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

std::vector<int> RandomGraphGenerator::shuffledNodes(int start, int end)
{
    std::vector<int> nodes;
    for (int i = start; i <= end; ++i)
    {
        nodes.push_back(i);
    }

    std::shuffle(nodes.begin(), nodes.end(), rng);
    return nodes;
}

void RandomGraphGenerator::addArc(int from, int to)
{
    if (from != to)
    {
        adjacency[from][to] = 1;
    }
}

bool RandomGraphGenerator::hasArc(int from, int to) const
{
    return adjacency[from][to] == 1;
}

void RandomGraphGenerator::generateRandomSTPath()
{
    const int s = params.source;
    const int t = (params.target > 0) ? params.target : params.nodes;

    std::vector<int> middleNodes;
    for (int i = 1; i <= params.nodes; ++i)
    {
        if (i != s && i != t)
            middleNodes.push_back(i);
    }
    std::shuffle(middleNodes.begin(), middleNodes.end(), rng);

    const int maxMiddle = static_cast<int>(middleNodes.size());
    const int middleCount = maxMiddle > 0 ? randomInt(0, maxMiddle) : 0;

    std::vector<int> path;
    path.push_back(s);

    for (int i = 0; i < middleCount; ++i)
    {
        path.push_back(middleNodes[i]);
    }

    path.push_back(t);

    for (size_t i = 0; i + 1 < path.size(); ++i)
    {
        addArc(path[i], path[i + 1]);
    }
}

void RandomGraphGenerator::generateRandomCycle()
{
    auto nodes = shuffledNodes(1, params.nodes);

    int cycleLength = randomInt(2, params.nodes);

    for (int i = 0; i < cycleLength - 1; ++i)
    {
        addArc(nodes[i], nodes[i + 1]);
    }

    addArc(nodes[cycleLength - 1], nodes[0]);
}

void RandomGraphGenerator::addRandomExtraArcs()
{
    for (int i = 1; i <= params.nodes; ++i)
    {
        for (int j = 1; j <= params.nodes; ++j)
        {
            if (i == j)
            {
                continue;
            }

            if (!hasArc(i, j) && randomDouble() < params.extraArcProbability)
            {
                addArc(i, j);
            }
        }
    }
}

void RandomGraphGenerator::generateToCSV(const std::string& filePath)
{
    for (int i = 0; i < params.paths; ++i)
    {
        generateRandomSTPath();
    }

    for (int i = 0; i < params.cycles; ++i)
    {
        generateRandomCycle();
    }

    addRandomExtraArcs();
    writeCSV(filePath);
}

void RandomGraphGenerator::writeCSV(const std::string& filePath)
{
    std::ofstream out(filePath);

    if (!out.is_open())
    {
        throw std::runtime_error("Cannot open file for writing: " + filePath);
    }

    out << "from,to,capacity,max_capacity,isNew,rate\n";

    for (int i = 1; i <= params.nodes; ++i)
    {
        for (int j = 1; j <= params.nodes; ++j)
        {
            if (!hasArc(i, j))
            {
                continue;
            }

            bool isNew = randomDouble() < params.newArcProbability;

            int capacity = isNew ? 0 : randomInt(params.minCapacity, params.maxCapacity);
            int maxCapacity = randomInt(std::max(capacity, params.maxCapacity), params.maxCapacityLimit);
            int rate = randomInt(params.minRate, params.maxRate);

            out << i << ","
                << j << ","
                << capacity << ","
                << maxCapacity << ","
                << (isNew ? 1 : 0) << ","
                << rate << "\n";
        }
    }
}
