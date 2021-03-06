#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <string>

struct Options
{
    std::string OUTPUT_FILE_NAME  = "output";
    uint32_t WIDTH                = 640;
    uint32_t HEIGHT               = 480;

	uint32_t FILIP_SAMPLES       = 16, FILIP_SAMPLES_LIGHT       = 1;
    uint32_t MIDPOINT_SAMPLES    = 16, MIDPOINT_SAMPLES_LIGHT    = 8;
    uint32_t TRAPEZOIDAL_SAMPLES = 16, TRAPEZOIDAL_SAMPLES_LIGHT = 8;

    uint32_t GAUSS_SAMPLES   = 16, GAUSS_SAMPLES_LIGHT   = 8;
    uint32_t ROMBERG_SAMPLES = 16, ROMBERG_SAMPLES_LIGHT = 8;

    uint32_t CHAPMAN_SAMPLES = 16;

    glm::highp_dvec3 CAM_ORIGIN       = glm::highp_dvec3(0.0f);
    glm::highp_dvec3 CAM_UP           = glm::highp_dvec3(0.0f, -1.0f, 0.0f);
    double CAM_PITCH                  = 0.0f;
    double CAM_YAW                    = -90.0f;
    double CAM_FOV                    = 60.0f;
    bool   CAM_FISHEYE                = false;
    
    glm::highp_dvec3 SUN_DIRECTION = glm::highp_dvec3(0.0, -1.0, 0.0);
    double           SUN_INTENSITY = 13.661;

    /** 
      * Toggle the defines just for one of the below planets.
      */

#if 1 // Planets from the solar system

    /* Venus */
    #if 0
    double PLANET_RADIUS           = 6052e3;
    double ATMOSPHERE_RADIUS       = PLANET_RADIUS + 200.059e3;
    double H_RAYLEIGH              = 15.9e3;
    double H_MIE                   = 2.244e3;
    glm::highp_dvec3 BETA_RAYLEIGH = glm::highp_dvec3(11.37e-6, 11.37e-6, 1.8e-6);
    glm::highp_dvec3 BETA_MIE      = glm::highp_dvec3(2.12153e-6);
    #endif

    /* Earth */
    #if 1
    double PLANET_RADIUS           = 6360e3;
    double ATMOSPHERE_RADIUS       = PLANET_RADIUS + 100e3;
    double H_RAYLEIGH              = 8000.0;
    double H_MIE                   = 1200.0;
    glm::highp_dvec3 BETA_RAYLEIGH = glm::highp_dvec3(3.8e-6f, 13.5e-6f, 33.1e-6f);
    glm::highp_dvec3 BETA_MIE      = glm::highp_dvec3(21e-6f);
    #endif

    /* Mars */
    #if 0
    double PLANET_RADIUS           = 3389.5e3;
    double ATMOSPHERE_RADIUS       = PLANET_RADIUS + 30.588e3;
    double H_RAYLEIGH              = 11.1e3;
    double H_MIE                   = 1.5671e3;
    glm::highp_dvec3 BETA_RAYLEIGH = glm::highp_dvec3(23.918e-6, 13.57e-6, 5.78e-6);
    glm::highp_dvec3 BETA_MIE      = glm::highp_dvec3(5.12153e-6);
    #endif

    /* Imaginary Im7 [Venus - Mars] */
    #if 0
    double PLANET_RADIUS           = 4.8e6;
    double ATMOSPHERE_RADIUS       = PLANET_RADIUS + 1.20368e5;
    double H_RAYLEIGH              = 1.36429e4;
    double H_MIE                   = 1.92570e3;
    glm::highp_dvec3 BETA_RAYLEIGH = glm::highp_dvec3(1.72705e-5, 1.24045e-5, 3.67153e-6);
    glm::highp_dvec3 BETA_MIE      = glm::highp_dvec3(3.53223e-6);
    #endif

    double MAX_PLANET_R = 6360e3 * ATMOSPHERE_PROPERTIES_SCALING_FACTOR;

#else // Imaginary planets, with imaginary properties

    /* Imaginary Im1 [Mars0] */
    #if 0
    double PLANET_RADIUS           = 2.0e6;
    double ATMOSPHERE_RADIUS       = PLANET_RADIUS + 1.0e4;
    double H_RAYLEIGH              = 5.0e3;
    double H_MIE                   = 1.0e3;
    glm::highp_dvec3 BETA_RAYLEIGH = glm::highp_dvec3(1.14e-7, 5.0e-6, 3.0e-6);
    glm::highp_dvec3 BETA_MIE      = glm::highp_dvec3(2.0e-6);
    #endif

    /* Imaginary Im2 [Mars1] */
    #if 0
    double PLANET_RADIUS           = 4.78e6;
    double ATMOSPHERE_RADIUS       = PLANET_RADIUS + 5.12e4;
    double H_RAYLEIGH              = 1.72e4;
    double H_MIE                   = 2.13e3;
    glm::highp_dvec3 BETA_RAYLEIGH = glm::highp_dvec3(4.772e-5, 2.214e-5, 8.56e-6);
    glm::highp_dvec3 BETA_MIE      = glm::highp_dvec3(8.24e-6);
    #endif

    /* Imaginary Im3 [Venus0] */
    #if 0
    double PLANET_RADIUS           = 4.0e6;
    double ATMOSPHERE_RADIUS       = PLANET_RADIUS + 1.5e5;
    double H_RAYLEIGH              = 1.2e4;
    double H_MIE                   = 1.2e3;
    glm::highp_dvec3 BETA_RAYLEIGH = glm::highp_dvec3(1.0e-5, 5.0e-6, 1.2e-6);
    glm::highp_dvec3 BETA_MIE      = glm::highp_dvec3(1.3e-6);
    #endif

    /* Imaginary Im4 [Venus1] */
    #if 0
    double PLANET_RADIUS           = 8.1e6;
    double ATMOSPHERE_RADIUS       = PLANET_RADIUS + 2.5e5;
    double H_RAYLEIGH              = 1.98e4;
    double H_MIE                   = 3.29e3;
    glm::highp_dvec3 BETA_RAYLEIGH = glm::highp_dvec3(1.27e-5, 1.77e-5, 2.4e-6);
    glm::highp_dvec3 BETA_MIE      = glm::highp_dvec3(2.94e-6);
    #endif

    /* Imaginary Im5 [Earth0] */
    #if 0
    double PLANET_RADIUS           = 5.0e6;
    double ATMOSPHERE_RADIUS       = PLANET_RADIUS + 5.0e4;
    double H_RAYLEIGH              = 3.0e3;
    double H_MIE                   = 8.0e2;
    glm::highp_dvec3 BETA_RAYLEIGH = glm::highp_dvec3(2.0e-6, 5.0e-6, 1.5e-5);
    glm::highp_dvec3 BETA_MIE      = glm::highp_dvec3(1.1e-5);
    #endif

    /* Imaginary Im6 [Earth1] */
    #if 0
    double PLANET_RADIUS           = 7.72e6;
    double ATMOSPHERE_RADIUS       = PLANET_RADIUS + 1.50e5;
    double H_RAYLEIGH              = 1.30e4;
    double H_MIE                   = 1.60e3;
    glm::highp_dvec3 BETA_RAYLEIGH = glm::highp_dvec3(5.6e-6, 2.2e-5, 5.12e-5);
    glm::highp_dvec3 BETA_MIE      = glm::highp_dvec3(3.1e-5);
    #endif

    double MAX_PLANET_R = 8.10e6 * ATMOSPHERE_PROPERTIES_SCALING_FACTOR;
#endif

    double ATMOSPHERE_PROPERTIES_SCALING_FACTOR = 1.0 / PLANET_RADIUS;
    glm::highp_dvec3 BACKGROUND_COLOR = glm::highp_dvec3(0.2);

    void printConfiguration() const
    {
        std::cout << std::endl << "==== Configuration ====\n" << std::endl;
         
        std::cout << "File                   = " << OUTPUT_FILE_NAME         << std::endl;
        std::cout << "Resolution             = " << WIDTH << " x " << HEIGHT << std::endl;

        std::cout << std::endl;

        std::cout << "Camera origin        = [" << CAM_ORIGIN.x << ", " << CAM_ORIGIN.y << ", " << CAM_ORIGIN.z << "]" << std::endl;
        std::cout << "Camera up            = [" << CAM_UP.x << ", " << CAM_UP.y << ", " << CAM_UP.z << "]"             << std::endl;
        std::cout << "Camera field of view = "  << CAM_FOV                                                             << std::endl;
        std::cout << "Camera pitch         = "  << CAM_PITCH                                                           << std::endl;
        std::cout << "Camera yaw           = "  << CAM_YAW                                                             << std::endl;

        std::cout << std::endl;
    }
};
