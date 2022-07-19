#include "OBJModel.h"
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/glm.hpp>

OBJModel::OBJModel(const std::string& file_name)
    : m_is_good(true)
{
    Assimp::Importer importer;

    const aiScene * scene = importer.ReadFile(file_name, aiProcess_Triangulate           |
                                                         aiProcess_JoinIdenticalVertices |
                                                         aiProcess_FlipUVs               |
                                                         aiProcess_GenSmoothNormals      |
                                                         aiProcess_CalcTangentSpace);

    if(!scene)
    {
        std::cerr << "Assimp Error:\n" << importer.GetErrorString() << std::endl << std::endl;
        return;
    }

    /* Process the scene */
    auto aiZeroVec3 = aiVector3D(0.0f, 0.0f, 0.0f);

    for(unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh * model = scene->mMeshes[i];

        for(unsigned int vertex = 0; vertex < model->mNumVertices; ++vertex)
        {
           auto pos      = model->mVertices[vertex];
           auto texcoord = model->HasTextureCoords(0) ? model->mTextureCoords[0][vertex] : aiZeroVec3;
           auto normal   = model->mNormals[vertex];
           auto tangent  = model->mTangents[vertex];

           m_positions.push_back(glm::vec4(pos.x,      pos.y,      pos.z,     1.0f));
           m_normals.push_back  (glm::vec4(normal.x,   normal.y,   normal.z,  0.0f));
           m_texcoords.push_back(glm::vec4(texcoord.x, texcoord.y, 0.0f,      0.0f));
           m_tangents.push_back (glm::vec4(tangent.x,  tangent.y,  tangent.z, 0.0f));
        }

        for(unsigned int f = 0; f < model->mNumFaces; ++f)
        {
           auto face = model->mFaces[f];

           m_indices.push_back(face.mIndices[0]);
           m_indices.push_back(face.mIndices[1]);
           m_indices.push_back(face.mIndices[2]);
        }
    }
}

void OBJModel::calcNormals()
{
    for (size_t i = 0; i < m_indices.size(); i += 3)
    {
        int i0 = m_indices[i];
        int i1 = m_indices[i + 1];
        int i2 = m_indices[i + 2];

        glm::vec4 v1 = m_positions[i1] - m_positions[i0];
        glm::vec4 v2 = m_positions[i2] - m_positions[i0];

        glm::vec4 normal = glm::vec4(glm::normalize(glm::cross(glm::vec3(v1), glm::vec3(v2))), 0.0f);

        m_normals[i0] = m_normals[i0] + normal;
        m_normals[i1] = m_normals[i1] + normal;
        m_normals[i2] = m_normals[i2] + normal;
    }

    for (size_t i = 0; i < m_normals.size(); i++)
        m_normals[i] = glm::normalize(m_normals[i]);
}

void OBJModel::calcTangents()
{
    for (size_t i = 0; i < m_indices.size(); i += 3)
    {
        int i0 = m_indices[i];
        int i1 = m_indices[i + 1];
        int i2 = m_indices[i + 2];

        glm::vec4 edge1 = m_positions[i1] - m_positions[i0];
        glm::vec4 edge2 = m_positions[i2] - m_positions[i0];

        float deltaU1 = m_texcoords[i1].x - m_texcoords[i0].x;
        float deltaV1 = m_texcoords[i1].y - m_texcoords[i0].y;
        float deltaU2 = m_texcoords[i2].x - m_texcoords[i0].x;
        float deltaV2 = m_texcoords[i2].y - m_texcoords[i0].y;

        float dividend = (deltaU1*deltaV2 - deltaU2*deltaV1);
        float f = dividend == 0 ? 0.0f : 1.0f / dividend;

        glm::vec4 tangent = glm::vec4(
            f * (deltaV2 * edge1.x - deltaV1 * edge2.x),
            f * (deltaV2 * edge1.y - deltaV1 * edge2.y),
            f * (deltaV2 * edge1.z - deltaV1 * edge2.z),
            0);

        m_tangents[i0] = m_tangents[i0] + tangent;
        m_tangents[i1] = m_tangents[i1] + tangent;
        m_tangents[i2] = m_tangents[i2] + tangent;
    }

    for (size_t i = 0; i < m_tangents.size(); i++)
        m_tangents[i] = glm::normalize(m_tangents[i]);
}
