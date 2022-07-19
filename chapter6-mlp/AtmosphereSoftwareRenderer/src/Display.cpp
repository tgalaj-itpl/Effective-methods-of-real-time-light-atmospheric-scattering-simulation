#include "Display.h"
#include <iostream>

Display::Display(size_t width, size_t height, std::string title)
    : m_framebuffer(nullptr),
      m_should_window_close(false)
{
    SDL_Init(SDL_INIT_VIDEO);

    m_window = SDL_CreateWindow(title.c_str(), 
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, 
                                width, 
                                height, 
                                0);

    m_renderer      = SDL_CreateRenderer(m_window, -1, 0);
    m_display_image = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, width, height);

    m_framebuffer = new RenderContext(width, height);
    m_framebuffer->clear(0x88888888);
    m_framebuffer->drawPixel(100, 100, 0xFF, 0x00, 0x00, 0x00);
}

Display::~Display()
{
    if (m_framebuffer != nullptr)
    {
        delete m_framebuffer;
    }

    SDL_DestroyTexture(m_display_image);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);

    SDL_Quit();
}

void Display::swapBuffers()
{
    SDL_UpdateTexture(m_display_image, 
                      nullptr,
                      m_framebuffer->getPixels(), 
                      m_framebuffer->getWidth() * sizeof(Uint32));

    SDL_PollEvent    (&m_event);

    switch (m_event.type)
    {
        case SDL_QUIT:
            m_should_window_close = true;
            break;
        case SDL_KEYDOWN:
            switch(m_event.key.keysym.sym)
            {
                case SDLK_ESCAPE:
                    m_should_window_close = true;
                    break;
            }
        break;
    }

    SDL_RenderClear  (m_renderer);
    SDL_RenderCopy   (m_renderer, m_display_image, nullptr, nullptr);
    SDL_RenderPresent(m_renderer);
}
