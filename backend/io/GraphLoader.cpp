#include "GraphLoader.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace
{
    struct CsvEdge
    {
        int from{};
        int to{};
        double c{};
        double cHat{};
        bool isNew{};
        double rate{};
    };

    double positivePart(double value)
    {
        return std::max(0.0, value);
    }

    double nodeDistance(int from, int to)
    {
        return static_cast<double>(std::abs(to - from));
    }

    Edge buildArticleEdge(const CsvEdge& edge)
    {
        const int from = edge.from;
        const int to = edge.to;
        const double c = edge.c;
        const double distance = nodeDistance(from, to);

        if (edge.isNew)
        {
            // Arc nou din A^a:
            // b(w,z) = 4 * |j-i| * z
            // u(w,z) = |j-i| * (z - 10)^2
            // Am folosit max(0,z-10) pentru ca functia sa ramana monotona si pentru z < 10.
            auto u = [distance](double z) {
                const double delta = positivePart(z - 10.0);
                return distance * delta * delta;
            };

            auto b = [distance](double z) {
                return 4.0 * distance * positivePart(z);
            };

            return Edge(from, to, c, edge.cHat, true, u, b);
        }

        // Arc existent din A:
        // b(w,z) = 10 * |j-i| * max(0, z - c(w))
        // u(w,z) = |j-i| * max(0, z - c(w))^2
        auto u = [distance, c](double z) {
            const double delta = positivePart(z - c);
            return distance * delta * delta;
        };

        auto b = [distance, c](double z) {
            return 10.0 * distance * positivePart(z - c);
        };

        return Edge(from, to, c, edge.cHat, false, u, b);
    }

    Edge buildLinearEdge(const CsvEdge& edge)
    {
        // Constructorul liniar trebuie sa seteze intern:
        // b(w,z) = rate * max(0, z - c(w)) si u(w,z) = 0.
        // Pentru arce noi, c(w) este de obicei 0, deci b(w,z)=rate*z.
        return Edge(edge.from, edge.to, edge.c, edge.cHat, edge.isNew, edge.rate);
    }
}

Graph loadGraphFromCSV(const std::string& filename, CostModel costModel)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Nu pot deschide fisierul CSV.");
    }

    std::string line;

    // Header asteptat:
    // from,to,c,c_hat,is_new,rate
    std::getline(file, line);

    std::vector<CsvEdge> edges;
    int maxNode = 0;

    int lineNumber = 1;
    while (std::getline(file, line))
    {
        ++lineNumber;

        if (line.empty())
        {
            continue;
        }

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> values;

        while (std::getline(ss, token, ','))
        {
            values.push_back(token);
        }

        if (values.size() < 6)
        {
            throw std::runtime_error("Linie CSV invalida la linia " + std::to_string(lineNumber) +
                                     ". Format asteptat: from,to,c,c_hat,is_new,rate");
        }

        CsvEdge edge;
        edge.from = std::stoi(values[0]);
        edge.to = std::stoi(values[1]);
        edge.c = std::stod(values[2]);
        edge.cHat = std::stod(values[3]);
        edge.isNew = (std::stoi(values[4]) != 0);
        edge.rate = std::stod(values[5]);

        if (edge.from <= 0 || edge.to <= 0)
        {
            throw std::runtime_error("Nodurile trebuie numerotate de la 1 in sus. Eroare la linia " +
                                     std::to_string(lineNumber));
        }

        if (edge.c < 0.0 || edge.cHat < 0.0 || edge.rate < 0.0)
        {
            throw std::runtime_error("Capacitatile si rata trebuie sa fie valori nenegative. Eroare la linia " +
                                     std::to_string(lineNumber));
        }

        if (edge.c > edge.cHat)
        {
            throw std::runtime_error("Capacitatea curenta nu poate depasi c_hat. Eroare la linia " +
                                     std::to_string(lineNumber));
        }

        edges.push_back(edge);
        maxNode = std::max(maxNode, std::max(edge.from, edge.to));
    }

    if (edges.empty())
    {
        throw std::runtime_error("Fisierul CSV nu contine arce.");
    }

    Graph graph(maxNode, 1, maxNode);

    for (const CsvEdge& edge : edges)
    {
        if (costModel == CostModel::Linear)
        {
            graph.addEdge(buildLinearEdge(edge));
        }
        else
        {
            graph.addEdge(buildArticleEdge(edge));
        }
    }

    return graph;
}
