#pragma once
#include "atmosphere/skymodels/Atmosphere.h"
#include "atmosphere/raytracer/Options.h"
#include <fstream>
#include "src/model.h"

class DeepAS : public Atmosphere
{
public:
    explicit DeepAS(const Options & options);
    ~DeepAS() = default;

    glm::highp_dvec3 computeIncidentLight(const Ray & ray, double t_min, double t_max) override;

private:
    std::vector<IntegrationData> integrator(Ray ray, double a, double b, unsigned n, bool precomptute) override;
    keras2cpp::Model m_neural_network = keras2cpp::Model::load("res/nn_lut_512_128_128_128_128_rm_earth_10layers-587.model");
    Options m_opt;
};

