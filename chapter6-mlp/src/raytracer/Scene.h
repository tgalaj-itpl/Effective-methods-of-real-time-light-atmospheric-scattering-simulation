#pragma once
#include <vector>

#include "Options.h"

class Scene
{
public:
    Scene()  = default;
    ~Scene() = default;

    static void loadScene(const std::string & scene_file_name, Options & options);

private:
    static bool readvalues(std::stringstream &s, const int numvals, double* values);
    static bool readvalues(std::stringstream &s, const int numvals, std::string & values);
};
