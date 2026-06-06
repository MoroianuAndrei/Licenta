#include "node.h"

Node::Node(int id) : id(id)
{

}

int Node::getId() const
{
    return id;
}

void Node::setId(int newId)
{
    id = newId;
}
