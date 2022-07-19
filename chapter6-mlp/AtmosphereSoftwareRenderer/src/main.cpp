#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "Display.h"
#include "Timing.h"
#include "apps/AtmoElek.h"
#include "apps/AtmoNN.h"

#define IMG_WIDTH  800
#define IMG_HEIGHT 600

// #define IMG_WIDTH  1280
// #define IMG_HEIGHT 720

int main(int argc, char *argv[])
{
    Display display(IMG_WIDTH, IMG_HEIGHT, "C++ Software Renderer");
    RenderContext* target = display.getFramebuffer();

    Options opt;

    //AtmoElek atmo(opt);
    AtmoNN atmo(opt);

    atmo.init(target);
 
    double current_time;
    double previous_time = Time::getTime();
    float delta          = 0.0f;

    glm::mat4 view_proj = glm::perspective(glm::radians(70.0f), target->getAspectRatio(), 0.001f, 1000.0f);
    float rot_counter = 0.0f;

    double sun_angle = 271.0;
    double cam_yaw = -90.0;
    double cam_pitch = 0.0;

    atmo.setSunDirection(sun_angle);

    glm::highp_dvec3 cam_positions[3] = { glm::highp_dvec3{0.0, 0.0f, 0.0f}, /* surface view */
                                          glm::highp_dvec3{ 0.0, 5061e3, -12650e3 } /* outer space view */ };

    atmo.setCameraPosition(cam_positions[0]);

    while (!display.shouldClose())
    {
        std::cout << "time = " << delta << " || sun angle = " << sun_angle << " || cam yaw = " << cam_yaw << "|| cam pitch = " << cam_pitch << "               \r";
        current_time  = Time::getTime();
        delta         = static_cast<float>(current_time - previous_time);
        previous_time = current_time;

        rot_counter += delta;
        glm::mat4 trans = glm::translate(glm::mat4(), glm::vec3(0, 0, -4 + glm::sin(1.5f*rot_counter))) *
                          glm::rotate(glm::mat4(), rot_counter, glm::vec3(1, 0, 0)) *
                          glm::rotate(glm::mat4(), rot_counter, glm::vec3(0, -1, 0)) *
                          glm::rotate(glm::mat4(), rot_counter, glm::vec3(0, 0, 1));

        target->clear(0x00);
        target->clearDepthBuffer();

        atmo.updateAndRender(target, delta);

        /* Process Input */
        SDL_PollEvent(display.getSDLEvent());

        switch (display.getSDLEvent()->type)
        {
            case SDL_KEYDOWN:
                switch (display.getSDLEvent()->key.keysym.sym)
                {
                    case SDLK_x:
                        // Increment sun angle
                        if (sun_angle < 359.0)
                            sun_angle += 1.0;
                        else
                            sun_angle = 0.0;
                        break;
                    case SDLK_z:
                        // Decrement sun angle
                        if (sun_angle > 0.0)
                            sun_angle -= 1.0;
                        else
                            sun_angle = 359.0;
                        break;
                    case SDLK_a:
                        // Look left
                        cam_yaw += 2.0;
                        break;
                    case SDLK_d:
                        // Look right
                        cam_yaw -= 2.0;
                        break;
                    case SDLK_w:
                        // Look up
                        cam_pitch += 2.0;
                        break;
                    case SDLK_s:
                        // Look down
                        cam_pitch -= 2.0;
                        break;
                    case SDLK_r:
                        // Change viewpoint
                        static int view = 0;
                        atmo.setCameraPosition(cam_positions[++view % 3]);
                        break;
                }
                atmo.setSunDirection(sun_angle);
                atmo.setCameraDir(cam_yaw, cam_pitch);

                break;
        }

        display.swapBuffers();
    }

    return 0;
}
