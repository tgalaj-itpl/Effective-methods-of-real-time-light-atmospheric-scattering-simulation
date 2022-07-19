#pragma once
#include "skymodels/Atmosphere.h"
#include "raytracer/Options.h"
#include <fstream>
#include "src/model.h"

#include "RootDir.h"
#define USE_10LAYERS 1

class DeepAS : public Atmosphere
{
public:
    explicit DeepAS(const Options & options);
    ~DeepAS() = default;

    glm::highp_dvec3 computeIncidentLight(const Ray & ray, double t_min, double t_max) override;

private:
    std::vector<IntegrationData> integrator(Ray ray, double a, double b, unsigned n, bool precomptute) override;

#if !USE_10LAYERS
    keras2cpp::Model m_neural_network = keras2cpp::Model::load(ROOT_DIR "/res/nn_lut_512_128_128_128_128_rm_earth_3layers-186.model");
#else
    keras2cpp::Model m_neural_network = keras2cpp::Model::load(ROOT_DIR "/res/nn_lut_512_128_128_128_128_rm_earth_10layers-587.model");
#endif

    Options m_opt;
};

