#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "Display.h"
#include "Timing.h"
#include "apps/AtmoElek.h"
#include "apps/AtmoNN.h"

#define HD_RES 1

#if !HD_RES
    #define IMG_WIDTH  800
    #define IMG_HEIGHT 450
#else
    #define IMG_WIDTH  1920
    #define IMG_HEIGHT 1080
#endif

struct SceneSettings
{
    glm::highp_dvec3 cam_position;
    glm::highp_dvec2 cam_angles; // yaw, pitch
    glm::highp_dvec2 sun_angles; // azimuth, elevation
};

int main(int argc, char *argv[])
{
    Display display(IMG_WIDTH, IMG_HEIGHT, "C++ Software Renderer", SDL_WINDOW_BORDERLESS);
    RenderContext* target = display.getFramebuffer();

    Options opt;

    AtmoElek atmo(opt);
    //AtmoNN atmo(opt);

    atmo.init(target);
 
    double current_time;
    double previous_time = Time::getTime();
    float  delta         = 0.0f;

    SceneSettings scene_settings[] = { { { 0.0,       -1000.0,     0.0},        { 116.0, -18.0}, {149.0, 307.0} }, // morning
                                       { { 0.0,       -1000.0,     0.0},        { 116.0, -18.0}, {90.0,  273.0} }, // sunset
                                       { { 0.0,       -5061000.00, 9319801.97}, {-90.0,   50.0}, {318.0, 290.0} }, // space view planet 
                                       { {-977641.32, -65760.64,   3928892.86}, {-52.0,   28.0}, {21.0,  254.0} }  // space view close look
                                    };

    int    current_view        = 0;
    auto   cam_pos             = scene_settings[current_view].cam_position;
    double cam_yaw             = scene_settings[current_view].cam_angles.x;
    double cam_pitch           = scene_settings[current_view].cam_angles.y;
    double sun_azimuth_angle   = scene_settings[current_view].sun_angles.x;
    double sun_elevation_angle = scene_settings[current_view].sun_angles.y;

    atmo.setCameraPosition(cam_pos);
    atmo.setCameraDir(cam_yaw, cam_pitch);
    atmo.setSunDirection(sun_elevation_angle, sun_azimuth_angle);

    while (!display.shouldClose())
    {
        std::cout << "time = " << delta << " | current view = " << current_view + 1 << "               \r";
        current_time  = Time::getTime();
        delta         = static_cast<float>(current_time - previous_time);
        previous_time = current_time;

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
                        if (sun_elevation_angle < 359.0)
                            sun_elevation_angle += 1.0;
                        else
                            sun_elevation_angle = 0.0;
                        break;
                    case SDLK_z:
                        // Decrement sun angle
                        if (sun_elevation_angle > 0.0)
                            sun_elevation_angle -= 1.0;
                        else
                            sun_elevation_angle = 359.0;
                        break;
                    case SDLK_c:
                        // Increment sun angle
                        if (sun_azimuth_angle < 359.0)
                            sun_azimuth_angle += 1.0;
                        else
                            sun_azimuth_angle = 0.0;
                        break;
                    case SDLK_v:
                        // Decrement sun angle
                        if (sun_azimuth_angle > 0.0)
                            sun_azimuth_angle -= 1.0;
                        else
                            sun_azimuth_angle = 359.0;
                        break;
                    case SDLK_a:
                        // Look left
                        cam_yaw -= 2.0;
                        break;
                    case SDLK_d:
                        // Look right
                        cam_yaw += 2.0;
                        break;
                    case SDLK_w:
                        // Look up
                        cam_pitch -= 2.0;
                        break;
                    case SDLK_s:
                        // Look down
                        cam_pitch += 2.0;
                        break;
                    case SDLK_r:
                        // Change viewpoint
                        current_view        = (current_view + 1) % std::size(scene_settings);
                        cam_pos             = scene_settings[current_view].cam_position;
                        cam_yaw             = scene_settings[current_view].cam_angles.x;
                        cam_pitch           = scene_settings[current_view].cam_angles.y;
                        sun_azimuth_angle   = scene_settings[current_view].sun_angles.x;
                        sun_elevation_angle = scene_settings[current_view].sun_angles.y;
                        break;
                    case SDLK_q:
                        // move camera down
                        cam_pos.y += delta * 10000.0f;
                        if (cam_pos.y > 0.0f)
                            cam_pos.y = 0.0f;
                        break;
                    case SDLK_e:
                        // move camera up
                        cam_pos.y -= delta * 10000.0f;
                        break;
                    case SDLK_UP:
                        cam_pos.z -= delta * 1000000.0f;
                        break;
                    case SDLK_DOWN:
                        cam_pos.z += delta * 1000000.0f;
                        break;
                    case SDLK_LEFT:
                        cam_pos.x += delta * 1000000.0f;
                        break;
                    case SDLK_RIGHT:
                        cam_pos.x -= delta * 1000000.0f;
                        break;
                    case SDLK_p:
                        printf("\n");
                        printf("Camera Position           = <%.2f, %.2f, %.2f>\n", cam_pos.x, cam_pos.y, cam_pos.z);
                        printf("Camera Yaw and Pitch      = <%.2f, %.2f>\n",       cam_yaw, cam_pitch);
                        printf("Sun Azimuth and Elevation = <%.2f, %.2f>\n",       sun_azimuth_angle, sun_elevation_angle);
                        break;
                }
                atmo.setSunDirection(sun_elevation_angle, sun_azimuth_angle);
                atmo.setCameraPosition(cam_pos);
                atmo.setCameraDir(cam_yaw, cam_pitch);

                break;
        }

        display.swapBuffers();
    }

    return 0;
}
