#pragma once
#include "Bitmap.h"
#include "Vertex.h"
#include "Edge.h"
#include "Mesh.h"

class RenderContext : public Bitmap
{
public:
    RenderContext(int width, int height);
    virtual ~RenderContext();

    void drawMesh(const Mesh& mesh, const glm::mat4& trans, const glm::mat4 & view_proj, const Bitmap& texture);
    void clearDepthBuffer() const;

    void drawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Bitmap& texture);

private:
    float* m_zbuffer;

    void drawScanLine(const Edge& left, const Edge& right, int j, const Bitmap& texture) const;
    void scanEdges(Edge& a, Edge& b, bool handedness, const Bitmap& texture) const;
    void scanTriangle(const Vertex& min_y, const Vertex& mid_y, const Vertex& max_y, bool handedness, const Bitmap& texture) const;
    void fillTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Bitmap& texture) const;
    void clipPolygonComponent(const std::vector<Vertex>& vertices, int component_index, float component_factor, std::vector<Vertex>& result);
    bool clipPolygonAxis(std::vector<Vertex>& vertices, std::vector<Vertex>& auxillary_list, int component_index);
};
