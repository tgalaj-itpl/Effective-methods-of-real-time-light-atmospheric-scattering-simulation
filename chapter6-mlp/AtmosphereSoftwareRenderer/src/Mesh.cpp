#include "Mesh.h"
#include "OBJModel.h"

Mesh::Mesh(const std::string& file_name)
{
    OBJModel model(file_name);

    for(size_t i = 0; i < model.m_positions.size(); ++i)
    {
        m_vertices.push_back(Vertex(model.m_positions[i], 
                                    model.m_texcoords[i],
                                    model.m_normals[i]));

        m_indices = model.m_indices;
    }
}
