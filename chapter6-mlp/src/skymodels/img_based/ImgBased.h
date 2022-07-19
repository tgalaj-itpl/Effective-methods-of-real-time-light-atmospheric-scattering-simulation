#pragma once
#include "skymodels/Atmosphere.h"
#include "raytracer/Options.h"
#include <fstream>
#include "src/model.h"

#include "RootDir.h"
#define IMG_BASED_TF 1
#define USE_IMG_BASED_SYNTH 0

class ImgBased : public Atmosphere
{
public:
    explicit ImgBased(const Options & options);
    ~ImgBased() = default;

    glm::highp_dvec3 computeIncidentLight(const Ray & ray, double t_min, double t_max) override;

private:
    std::vector<IntegrationData> integrator(Ray ray, double a, double b, unsigned n, bool precomptute) override;

#if !IMG_BASED_TF
    #if USE_IMG_BASED_SYNTH
    keras2cpp::Model m_neural_network = keras2cpp::Model::load(ROOT_DIR "/res/nn_img_based_synth_256_256_10l-84.model");
    #else
    keras2cpp::Model m_neural_network = keras2cpp::Model::load(ROOT_DIR "/res/nn_img_based_real_256_256_10l-88.model");
    #endif
#else
    keras2cpp::Model m_neural_network = keras2cpp::Model::load(ROOT_DIR "/res/nn_img_based_tf_real_256_256-81.model");
#endif // !IMG_BASED_TF

    Options m_opt;
};

