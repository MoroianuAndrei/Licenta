#pragma once

class Node 
{
private:
    int id;

public:
    Node(int id);

    int getId() const;
    void setId(int newId);
};