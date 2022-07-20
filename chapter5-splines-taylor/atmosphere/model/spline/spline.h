#ifndef ATMOSPHERE_MODEL_SPLINE_H_
#define ATMOSPHERE_MODEL_SPLINE_H_

#include "atmosphere/atmosphere.h"
#include "math/angle.h"
#include "physics/units.h"
#include "math/vector.h"
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstddef>
#include "atmosphere/model/spline/Spline2dSolution.h"
#include <memory>

typedef dimensional::Vector3<Length> Position;
typedef dimensional::Vector3<Length> Vector;
typedef dimensional::Vector3<Number> Direction;

#define SPLINE_OUTPUT_DIR "output/cache/spline/"
#define USE_SPLINE_PRECOMPUTATION 1

/* Spline method */
class Spline : public Atmosphere
{
public:
    Spline();

    int GetOriginalNumberOfWavelengths() const override { return 3; }

    IrradianceSpectrum GetSunIrradiance(Length altitude,
                                        Angle sun_zenith) const override;

    RadianceSpectrum GetSkyRadiance(Length altitude,
                                    Angle sun_zenith,
                                    Angle view_zenith,
                                    Angle view_sun_azimuth) const override;

protected:
    std::unique_ptr<solution::Spline2dPrecomputation> spline2d_rayleigh_below;
    std::unique_ptr<solution::Spline2dPrecomputation> spline2d_rayleigh_above;
    std::unique_ptr<solution::Spline2dPrecomputation> spline2d_mie_below;
    std::unique_ptr<solution::Spline2dPrecomputation> spline2d_mie_above;
    const uint32_t num_samples;
    const uint32_t num_samples_light;
    double planet_radius, atmosphere_radius;
    double h_rayleigh, h_mie;
    Length inner_radius;

    static Length DistanceToSphere(Length r, Length rmu, Length sphere_radius);
    glm::highp_dvec2 getIntegralValue(const glm::highp_dvec3 &pa, const glm::highp_dvec3 &pe, const Ray &ray);
};

#endif // ATMOSPHERE_MODEL_SPLINE_H_