#include "node.h"

// Implementarea constructorului
Node::Node(int id) : id(id) {
    // Aici vom putea initializa ulterior elemente specifice Qt
}

int Node::getId() const {
    return id;
}

void Node::setId(int newId) {
    id = newId;
}