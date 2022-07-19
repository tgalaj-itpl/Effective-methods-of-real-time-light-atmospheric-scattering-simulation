#include "Bitmap.h"
#include <string>
#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Bitmap::Bitmap(size_t width, size_t height)
    : m_width(width), m_height(height)
{
    /* RGBA format */
    m_pixel_components = new Uint32[m_width * m_height];
    std::fill_n(m_pixel_components, m_width * m_height, 0x00);
}

Bitmap::Bitmap(const std::string & file_name)
{
    int w, h, n;
    unsigned char* data = stbi_load(file_name.c_str(), &w, &h, &n, 4);

    m_width  = w;
    m_height = h;

    m_pixel_components = new Uint32[m_width * m_height];

    for(auto i = 0; i < m_width * m_height; i++)
    {
        auto r = data[i * 4 + 0];
        auto g = data[i * 4 + 1];
        auto b = data[i * 4 + 2];
        auto a = data[i * 4 + 3];

        m_pixel_components[i] = ((r & 0xFF) << 24) + ((g & 0xFF) << 16) + ((b & 0xFF) << 8) + (a & 0xFF);;
    }

    stbi_image_free(data);
}

Bitmap::~Bitmap()
{
    delete[] m_pixel_components;
}

void Bitmap::clear(Uint32 shade) const
{
    std::fill_n(m_pixel_components, m_width * m_height, shade);
}

void Bitmap::drawPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) const
{
    int index = (x + y * m_width);
    m_pixel_components[index] = ((r & 0xFF) << 24) + ((g & 0xFF) << 16) + ((b & 0xFF) << 8) + (a & 0xFF);
}

void Bitmap::copyPixel(int dst_x, int dst_y, int src_x, int src_y, const Bitmap * src, float light_amt) const
{
    int dst_index = (dst_x + dst_y * m_width);
    int src_index = (src_x + src_y * src->getWidth());

    Uint32 comp = src->getComponent(src_index);

    int a = comp & 0xFF;
    int b = (comp >> 8) & 0xFF;
    int g = (comp >> 16) & 0xFF;
    int r = (comp >> 24) & 0xFF;

    glm::vec4 color { r, g, b, a};
    color = color / 255.0f;
    color = color * light_amt;

    a = color.a * 255.0f + 0.5f;
    b = color.b * 255.0f + 0.5f;
    g = color.g * 255.0f + 0.5f;
    r = color.r * 255.0f + 0.5f;

    m_pixel_components[dst_index] = ((r & 0xFF) << 24) + ((g & 0xFF) << 16) + ((b & 0xFF) << 8) + (a & 0xFF);
}
