#pragma once

#include <string>
#include <SDL2\SDL.h>

#include "RenderContext.h"

class Display
{
public:
    Display(size_t width, size_t height, std::string title);
    ~Display();

    void swapBuffers();
    bool shouldClose() const { return m_should_window_close; }
    RenderContext* const getFramebuffer() const { return m_framebuffer; }
    SDL_Event * getSDLEvent() { return &m_event; }

private:
    SDL_Event m_event;

    RenderContext* m_framebuffer;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_display_image;

    bool m_should_window_close;
};
