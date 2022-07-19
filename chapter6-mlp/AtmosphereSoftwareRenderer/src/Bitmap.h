#pragma once
#include <SDL2/SDL_hints.h>
#include <string>

class Bitmap
{
public:
    Bitmap(size_t width, size_t height);
    explicit Bitmap(const std::string& file_name);

    virtual ~Bitmap();

    void clear(Uint32 shade) const;
    void drawPixel(int x, 
                   int y,
                   unsigned char r,
                   unsigned char g, 
                   unsigned char b, 
                   unsigned char a) const;

    void copyPixel(int dst_x,
                   int dst_y,
                   int src_x,
                   int src_y,
                   const Bitmap* src,
                   float light_amt) const;

    Uint32 getComponent(int index) const
    {
        return m_pixel_components[index]; 
    }

    size_t getWidth()      const { return m_width; }
    size_t getHeight()     const { return m_height; }
    float getAspectRatio() const { return static_cast<float>(m_width)/static_cast<float>(m_height); }

    Uint32* getPixels() const { return m_pixel_components; }

protected:
    int m_width;
    int m_height;
    Uint32* m_pixel_components;
};
