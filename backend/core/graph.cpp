#include "graph.h"

// Implementarea constructorului
Graph::Graph(int nodeCount, int source, int target)
    : n(nodeCount), s(source), t(target) {

    // Alocăm spațiu pentru vectorul de liste de adiacență.
    // Presupunând că id-urile nodurilor sunt indexate de la 1 la n (cum este 
    // standard în literatură și în exemplul tău din figura 1), alocăm n + 1.
    // Astfel, vom putea accesa direct adj[edge.from] fără să scădem 1.
    adj.resize(n + 1);
}

int Graph::getNodesCount() const { return n; }
int Graph::getSource() const { return s; }
int Graph::getTarget() const { return t; }

// Implementarea metodei de adăugare a arcelor
void Graph::addEdge(const Edge& edge) {
    // Fiind un graf direcționat, adăugăm arcul doar în lista nodului din care pleacă (from)
    adj[edge.getFrom()].push_back(edge);
}

const std::vector<Edge>& Graph::getEdgesFrom(int nodeId) const {
    return adj[nodeId];
}