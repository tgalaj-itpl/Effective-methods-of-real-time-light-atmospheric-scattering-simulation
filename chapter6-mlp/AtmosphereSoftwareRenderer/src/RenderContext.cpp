#include "RenderContext.h"
#include <utility>
#include "MathUtils.h"
#include "Edge.h"

RenderContext::RenderContext(int width, int height)
    : Bitmap       (width, height)
{
    m_zbuffer = new float[m_width * m_height];
    std::fill_n(m_zbuffer, m_width * m_height, 9999999999);
}

RenderContext::~RenderContext()
{
    if(m_zbuffer != nullptr)
    {
        delete[] m_zbuffer;
    }
}

void RenderContext::drawScanLine(const Edge& left, const Edge& right, int j, const Bitmap& texture) const
{
    int x_min = static_cast<int>(glm::ceil(left.getX()));
    int x_max = static_cast<int>(glm::ceil(right.getX()));
  
    float x_prestep = x_min - left.getX();

    float x_dist = right.getX() - left.getX();
    float texcoord_xx_step = (right.getTexcoordX() - left.getTexcoordX()) / x_dist;
    float texcoord_yx_step = (right.getTexcoordY() - left.getTexcoordY()) / x_dist;
    float one_over_z_step_x = (right.getOneOverZ() - left.getOneOverZ()) / x_dist;
    float depth_step_x = (right.getDepth() - left.getDepth()) / x_dist;
    float light_amt_step_x = (right.getLightAmt() - left.getLightAmt()) / x_dist;

    float texcoord_x = left.getTexcoordX() + texcoord_xx_step * x_prestep;
    float texcoord_y = left.getTexcoordY() + texcoord_yx_step * x_prestep;
    float one_over_z = left.getOneOverZ() + one_over_z_step_x * x_prestep;
    float depth = left.getDepth() + depth_step_x * x_prestep;
    float light_amt = left.getLightAmt() + depth_step_x * x_prestep;

    for (int i = x_min; i < x_max; ++i)
    {
        int index = i + j * m_width;

        if (depth < m_zbuffer[index])
        {
            m_zbuffer[index] = depth;

            float z = 1.0f / one_over_z;
            int src_x = static_cast<int>(z * texcoord_x * (texture.getWidth() - 1) + 0.5f);
            int src_y = static_cast<int>(z * texcoord_y * (texture.getHeight() - 1) + 0.5f);

            copyPixel(i, j, src_x, src_y, &texture, light_amt);
        }

        texcoord_x += texcoord_xx_step;
        texcoord_y += texcoord_yx_step;
        one_over_z += one_over_z_step_x;
        depth      += depth_step_x;
        light_amt  += light_amt_step_x;
    }
}

void RenderContext::scanEdges(Edge& a, Edge& b, bool handedness, const Bitmap& texture) const
{
    Edge* left  = &a;
    Edge* right = &b;

    if (handedness)
    {
        std::swap(left, right);
    }

    int y_start = b.getStart();
    int y_end   = b.getEnd();

    for (int j = y_start; j < y_end; ++j)
    {
        drawScanLine(*left, *right, j, texture);
        left->step();
        right->step();
    }
}

void RenderContext::scanTriangle(const Vertex& min_y, const Vertex& mid_y, const Vertex& max_y, bool handedness, const Bitmap& texture) const
{
    Gradients gradients  (min_y, mid_y, max_y);

    Edge top_to_bottom   (gradients, min_y, max_y, 0);
    Edge top_to_middle   (gradients, min_y, mid_y, 0);
    Edge middle_to_bottom(gradients, mid_y, max_y, 1);

    scanEdges(top_to_bottom, top_to_middle,    handedness, texture);
    scanEdges(top_to_bottom, middle_to_bottom, handedness, texture);
}

void RenderContext::clipPolygonComponent(const std::vector<Vertex>& vertices, int component_index, float component_factor, std::vector<Vertex>& result)
{
    Vertex previous_vertex = vertices[vertices.size() - 1];
    float previous_component = previous_vertex.get(component_index) * component_factor;
    bool previous_inside = previous_component <= previous_vertex.m_pos.w;

    for(auto& v : vertices)
    {
        Vertex current_vertex = v;
        float current_component = current_vertex.get(component_index) * component_factor;
        bool  current_inside = current_component <= current_vertex.m_pos.w;

        if(current_inside ^ previous_inside)
        {
            float lerp_amt = (previous_vertex.m_pos.w - previous_component) / 
                             ((previous_vertex.m_pos.w - previous_component) - (current_vertex.m_pos.w - current_component));

            result.push_back(previous_vertex.lerp(current_vertex, lerp_amt));
        }

        if(current_inside)
        {
            result.push_back(current_vertex);
        }

        previous_vertex = current_vertex;
        previous_component = current_component;
        previous_inside = current_inside;
    }
}

bool RenderContext::clipPolygonAxis(std::vector<Vertex>& vertices, std::vector<Vertex>& auxillary_list, int component_index)
{
    clipPolygonComponent(vertices, component_index, 1.0f, auxillary_list);
    vertices.clear();

    if(auxillary_list.empty())
    {
        return false;
    }

    clipPolygonComponent(auxillary_list, component_index, -1.0f, vertices);
    auxillary_list.clear();

    return !vertices.empty();
}

void RenderContext::fillTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Bitmap& texture) const
{
    glm::mat4 screen_space_transform = MathUtils::initScreenSpace(m_width / 2.0f, m_height / 2.0f);
    glm::mat4 identity;

    Vertex min_y = v1.transform(screen_space_transform, identity).perspectiveDivide();
    Vertex mid_y = v2.transform(screen_space_transform, identity).perspectiveDivide();
    Vertex max_y = v3.transform(screen_space_transform, identity).perspectiveDivide();

    if(min_y.triangleArea(max_y, mid_y) < 0.0f)
    {
        return;
    }

    if(max_y.m_pos.y < mid_y.m_pos.y)
    {
        std::swap(max_y, mid_y);
    }

    if (mid_y.m_pos.y < min_y.m_pos.y)
    {
        std::swap(mid_y, min_y);
    }

    if (max_y.m_pos.y < mid_y.m_pos.y)
    {
        std::swap(max_y, mid_y);
    }

    scanTriangle(min_y, mid_y, max_y, min_y.triangleArea(max_y, mid_y) >= 0.0f, texture);
}

void RenderContext::drawMesh(const Mesh & mesh, const glm::mat4 & trans, const glm::mat4 & view_proj, const Bitmap& texture)
{
    glm::mat4 mvp = view_proj * trans;
    for(auto i = 0; i < mesh.getIndicesCount(); i += 3)
    {
        drawTriangle(mesh.getVertex(mesh.getIndex(i + 0)).transform(mvp, trans),
                     mesh.getVertex(mesh.getIndex(i + 1)).transform(mvp, trans),
                     mesh.getVertex(mesh.getIndex(i + 2)).transform(mvp, trans),
                     texture);
    }
}

void RenderContext::clearDepthBuffer() const
{
    std::fill_n(m_zbuffer, m_width * m_height, 9999999999);
}

void RenderContext::drawTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Bitmap& texture)
{
    if(v1.isInsideViewFrustum() && v2.isInsideViewFrustum() && v3.isInsideViewFrustum())
    {
        fillTriangle(v1, v2, v3, texture);
        return;
    }

    std::vector<Vertex> vertices { v1, v2, v3 };
    std::vector<Vertex> auxillary_list;

    if(clipPolygonAxis(vertices, auxillary_list, 0) &&
       clipPolygonAxis(vertices, auxillary_list, 1) &&
       clipPolygonAxis(vertices, auxillary_list, 2))
    {
        Vertex init_vertex = vertices[0];
        for(int i = 0; i < vertices.size() - 1; ++i)
        {
            fillTriangle(init_vertex, vertices[i], vertices[i+1], texture);
        }
    }
}
