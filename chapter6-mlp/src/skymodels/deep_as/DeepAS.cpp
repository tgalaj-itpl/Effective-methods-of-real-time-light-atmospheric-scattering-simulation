#include "DeepAS.h"
#include "raytracer/Utils.h"
#include "MemoryMapped.h"
#include "raytracer/Timing.h"

#include <sstream>
#include <iomanip>
#include <sstream>
#include <glm/gtc/epsilon.hpp>
#include <3rdparty\keras2cpp\src\layers\dense.h>

DeepAS::DeepAS(const Options & options)
    : Atmosphere(options)
{
    std::cout << "DEEP ATMOSPHERIC SCATTERING INFO" << std::endl;
        
    auto tmp_beta_r = BETA_RAYLEIGH / (1.0 / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR);
    auto tmp_beta_m = BETA_MIE      / (1.0 / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR);

    std::cout << "PLANET R             = " << planet_radius / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "ATMOSPHERE THICKNESS = " << (atmosphere_radius - planet_radius) / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "H_r                  = " << h_rayleigh / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "H_m                  = " << h_mie / options.ATMOSPHERE_PROPERTIES_SCALING_FACTOR << std::endl;
    std::cout << "BETA_r               = " << "(" << tmp_beta_r.x << ", " << tmp_beta_r.y << "," << tmp_beta_r.z << ")" << std::endl;
    std::cout << "BETA_m               = " << "(" << tmp_beta_m.x << ", " << tmp_beta_m.y << "," << tmp_beta_m.z << ")" << std::endl;

    std::cout << "Model size in bytes  = " << sizeof(m_neural_network) << std::endl << std::endl;
    m_opt = options;
}

glm::highp_dvec3 DeepAS::computeIncidentLight(const Ray & ray, double t_min, double t_max)
{
    double t0, t1;
    if (!intersect(ray, t0, t1) || t1 < 0.0)
    {
        return glm::highp_dvec3(0.0);
    }

    if (t0 > t_min && t0 > 0.0)
    {
        t_min = t0;
    }

    if (t1 < t_max)
    {
        t_max = t1;
    }

    /* mu in the paper which is the cosine of the angle between the sun direction and the ray direction */
    double mu = glm::dot(ray.m_direction, sun_light);
    double phase_r = rayleigh_phase_func(mu);
    double phase_m = mie_phase_func(g, mu);

    glm::highp_dvec3 P_v = ray.m_origin;
    glm::highp_dvec3 P_g = ray.m_origin + ray.m_direction * t_max;
    glm::highp_dvec3 R_v = ray.m_direction;

    if (t0 > 0.0)
    {
        P_v = ray.m_origin + ray.m_direction * t0;
    }

    /* Calculate light intensity for the P_v */
    double view_angle = glm::acos(glm::dot(glm::normalize(P_v), R_v));
    double sun_angle  = glm::acos(glm::dot(glm::normalize(P_v), sun_light));
    double height     = sampleHeight(P_v);
    
    auto in = keras2cpp::Tensor{ 3 };
    in.data_ = { float(height / (atmosphere_radius - planet_radius)), float(sun_angle / glm::pi<double>()), float(view_angle / glm::pi<double>()) };
    auto out = m_neural_network(in);

#if 0
    /* My nn implementation */
    std::vector<double> outputs(dynamic_cast<keras2cpp::layers::Dense*>(m_neural_network.layers_[0].get())->biases_.size());
    std::vector<double> inputs(3);
    std::cout << "outputs size = " << outputs.size() << std::endl;
    std::cout << "inputs size = " << inputs.size() << std::endl;

    inputs[0] = float(height / (atmosphere_radius - planet_radius));
    inputs[1] = float(sun_angle / glm::pi<double>());
    inputs[2] = float(view_angle / glm::pi<double>());

    std::vector<double> weights;
    std::vector<double> biases;

    for (int i = 0; i < m_neural_network.layers_.size(); i += 2)
    {
        auto tensor_weights = dynamic_cast<keras2cpp::layers::Dense*>(m_neural_network.layers_[i].get())->weights_;
        auto tensor_biases  = dynamic_cast<keras2cpp::layers::Dense*>(m_neural_network.layers_[i].get())->biases_;

        weights.insert(std::end(weights), std::begin(tensor_weights), std::end(tensor_weights));
        biases.insert (std::end(biases),  std::begin(tensor_biases),  std::end(tensor_biases));
    }

    std::cout << "weights size = " << weights.size() << std::endl;
    std::cout << "biases size = " << biases.size() << std::endl;

    int offset_n = 0;
    int offset_b = 0;

    for (int i = 0; i < m_neural_network.layers_.size(); i += 2)
    {
        auto tensor_weights = dynamic_cast<keras2cpp::layers::Dense*>(m_neural_network.layers_[i].get())->weights_;
        auto tensor_biases  = dynamic_cast<keras2cpp::layers::Dense*>(m_neural_network.layers_[i].get())->biases_;

        unsigned num_prev_neurons    = tensor_weights.dims_[1];
        unsigned num_current_neurons = tensor_weights.dims_[0];

        for (int n = 0; n < num_current_neurons; ++n) //iterate over neurons from the current layer
        {
            double sum = 0.0;
            for (int pn = 0; pn < num_prev_neurons; ++pn)
            {
                sum += weights[offset_n + (n * num_prev_neurons) + pn] * inputs[pn];
            }
            outputs[n] = glm::max(sum + biases[offset_b + n], 0.0);

            if (i == m_neural_network.layers_.size() - 1)
                outputs[n] = sum + biases[offset_b + n];
        }
        offset_n += num_current_neurons * num_prev_neurons;
        //offset_n += tensor_weights.size();
        offset_b += tensor_biases.size();
        inputs = outputs;

        printf("Layer %d, num_w=%d, num_b=%d, num_prev_neurons=%d, offset_n=%d, offset_b=%d\n", i / 2, tensor_weights.size(), tensor_biases.size(), num_prev_neurons, offset_n, offset_b);
    }

    printf("Output mine = %.6f, %.6f, %.6f, %.6f\n", outputs[0], outputs[1], outputs[2], outputs[3]);
    printf("Output ref  = %.6f, %.6f, %.6f, %.6f\n\n", out.data_[0], out.data_[1], out.data_[2], out.data_[3]);
    exit(0);
    /* End My nn implementation */
#endif

    auto intensity_rayleigh = glm::highp_dvec4(out.data_[0], out.data_[1], out.data_[2], out.data_[3]);
    auto intensity_mie      = glm::highp_dvec3(intensity_rayleigh) * intensity_rayleigh.a * BETA_RAYLEIGH.r * BETA_MIE / (intensity_rayleigh.r * BETA_MIE.r * BETA_RAYLEIGH + 0.00001);
    
    auto atmo_color = (glm::highp_dvec3(intensity_rayleigh) * phase_r + intensity_mie * phase_m) * sun_intensity;

    return atmo_color;
}

std::vector<IntegrationData> DeepAS::integrator(Ray ray, double a, double b, unsigned n, bool precomptute)
{
    std::vector<IntegrationData> data(1);
    return data;
}