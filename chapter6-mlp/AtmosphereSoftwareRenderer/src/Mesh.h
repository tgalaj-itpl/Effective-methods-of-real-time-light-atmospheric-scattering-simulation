#pragma once
#include <vector>
#include <string>
#include "Vertex.h"

class Mesh
{
public:
    explicit Mesh(const std::string& file_name);

    Vertex getVertex(int loc) const { return m_vertices[loc]; }

    int getIndex(int loc) const { return m_indices[loc]; }
    int getIndicesCount() const { return m_indices.size(); }

private:
    std::vector<Vertex> m_vertices;
    std::vector<int>    m_indices;
};
