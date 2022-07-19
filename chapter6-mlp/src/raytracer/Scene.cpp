#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Scene.h"
#include "RootDir.h"

void Scene::loadScene(const std::string & scene_file_name, Options & options)
{
    std::ifstream fs;
    std::string str, cmd;

    fs.open(ROOT_DIR "/res/scenes/" + scene_file_name);
    options.OUTPUT_FILE_NAME = scene_file_name.substr(scene_file_name.find_first_of('/') + 1, scene_file_name.find('.'));

    double values[10];

    if (fs.is_open())
    {
        std::getline(fs, str);

        while (fs)
        {
            if ((str.find_first_not_of(" \t\r\n") != std::string::npos) && (str[0] != '#'))
            {
                std::stringstream s(str);
                s >> cmd;

                bool isValidInput;

                if (cmd == "filip_samples")
                {
                    if (readvalues(s, 1, values))
                    {
                        options.FILIP_SAMPLES = uint32_t(values[0]);
                    }
                }
                else if (cmd == "filip_light_samples")
                {
                    if (readvalues(s, 1, values))
                    {
                        options.FILIP_SAMPLES_LIGHT = uint32_t(values[0]);
                    }
                }
                else if (cmd == "midpoint_samples")
                {
                    if (readvalues(s, 1, values))
                    {
                        options.MIDPOINT_SAMPLES = uint32_t(values[0]);
                    }
                }
                else if (cmd == "midpoint_light_samples")
                {
                    if (readvalues(s, 1, values))
                    {
                        options.MIDPOINT_SAMPLES_LIGHT = uint32_t(values[0]);
                    }
                }
                else if (cmd == "trapezoidal_samples")
                {
                    if (readvalues(s, 1, values))
                    {
                        options.TRAPEZOIDAL_SAMPLES = uint32_t(values[0]);
                    }
                }
                else if (cmd == "trapezoidal_light_samples")
                {
                    if (readvalues(s, 1, values))
                    {
                        options.TRAPEZOIDAL_SAMPLES_LIGHT = uint32_t(values[0]);
                    }
                }
                else if (cmd == "gauss_samples")
                {
                    if (readvalues(s, 1, values))
                    {
                        options.GAUSS_SAMPLES = uint32_t(values[0]);
                    }
                }
                else if (cmd == "gauss_light_samples")
                {
                    if (readvalues(s, 1, values))
                    {
                        options.GAUSS_SAMPLES_LIGHT = uint32_t(values[0]);
                    }
                }
                else if (cmd == "romberg_samples")
                {
                    if (readvalues(s, 1, values))
                    {
                        options.ROMBERG_SAMPLES = uint32_t(values[0]);
                    }
                }
                else if (cmd == "romberg_light_samples")
                {
                    if (readvalues(s, 1, values))
                    {
                        options.ROMBERG_SAMPLES_LIGHT = uint32_t(values[0]);
                    }
                }
                else if (cmd == "size")
                {
                    isValidInput = readvalues(s, 2, values);

                    if (isValidInput)
                    {
                        options.WIDTH = static_cast<int>(values[0]);
                        options.HEIGHT = static_cast<int>(values[1]);
                    }
                }
                else if (cmd == "output")
                {
                    std::string output;
                    isValidInput = readvalues(s, 1, output);

                    if (isValidInput)
                    {
                        options.OUTPUT_FILE_NAME = output;
                    }
                }
                else if (cmd == "camera_origin")
                {
                    isValidInput = readvalues(s, 3, values);

                    if (isValidInput)
                    {
                        options.CAM_ORIGIN = glm::highp_dvec3(values[0], values[1], values[2]);
                    }
                }
                else if (cmd == "camera_orientation")
                {
                    isValidInput = readvalues(s, 2, values);

                    if (isValidInput)
                    {
                        options.CAM_PITCH = values[0];
                        options.CAM_YAW   = values[1];
                    }
                }
                else if (cmd == "camera_up")
                {
                    isValidInput = readvalues(s, 3, values);

                    if (isValidInput)
                    {
                        options.CAM_UP = glm::highp_dvec3(values[0], values[1], values[2]);
                    }
                }
                else if (cmd == "camera_fov")
                {
                    isValidInput = readvalues(s, 1, values);

                    if (isValidInput)
                    {
                        options.CAM_FOV = values[0];
                    }
                }
                else if (cmd == "camera_fisheye")
                {
                    isValidInput = readvalues(s, 1, values);

                    if (isValidInput)
                    {
                        options.CAM_FISHEYE = bool(values[0]);
                    }
                }
                else if (cmd == "sun_angle")
                {
                    isValidInput = readvalues(s, 1, values);

                    if (isValidInput)
                    {
                        double angle = glm::radians(values[0]);
                        glm::highp_dvec3 dir = glm::highp_dvec3(0.0, -glm::cos(angle), glm::sin(angle));

                        options.SUN_DIRECTION = glm::normalize(dir);
                    }
                }
                else if (cmd == "sun_intensity")
                {
                    isValidInput = readvalues(s, 1, values);

                    if (isValidInput)
                    {
                        options.SUN_INTENSITY = values[0];
                    }
                }
            }

            std::getline(fs, str);
        }
    }
    else
    {
        printf("Unable to open file %s\n", scene_file_name.c_str());
    }

    fs.close();
}

bool Scene::readvalues(std::stringstream &s, const int numvals, double* values)
{
    for (int i = 0; i < numvals; i++) {
        s >> values[i];
        if (s.fail()) {
            std::cout << "Failed reading value " << i << " will skip\n";
            return false;
        }
    }
    return true;
}

bool Scene::readvalues(std::stringstream &s, const int numvals, std::string & values)
{
    std::string ts;

    for (int i = 0; i < numvals; i++) {
        s >> ts;
        values += ts;
        if (s.fail()) {
            std::cout << "Failed reading value " << i << " will skip\n";
            return false;
        }
    }
    return true;
}
