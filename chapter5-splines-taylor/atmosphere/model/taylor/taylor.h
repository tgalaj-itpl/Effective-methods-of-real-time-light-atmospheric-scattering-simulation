#ifndef ATMOSPHERE_MODEL_TAYLOR_H_
#define ATMOSPHERE_MODEL_TAYLOR_H_

#include "atmosphere/atmosphere.h"
#include "math/angle.h"
#include "physics/units.h"
#include <cstdint>

/* Taylor based method */
class Taylor : public Atmosphere {
 public:
  Taylor();

  int GetOriginalNumberOfWavelengths() const override { return 3; }

  IrradianceSpectrum GetSunIrradiance(Length altitude, 
                                      Angle sun_zenith) const override;

  RadianceSpectrum GetSkyRadiance(Length altitude, 
                                  Angle sun_zenith, 
                                  Angle view_zenith, 
                                  Angle view_sun_azimuth) const override;

 protected:
    const uint32_t num_samples;
    const uint32_t num_samples_light;
    
    double approx_air_column_density_ratio_along_3d_ray_for_curved_world (
        glm::highp_dvec3  P, // position of viewer
        glm::highp_dvec3  V, // direction of viewer (unit vector)
        double x, // distance from the viewer at which we stop the "raymarch"
        double r, // radius of the planet
        double H  // scale height of the planet's atmosphere
    ) const;

    double approx_air_column_density_ratio_along_2d_ray_for_curved_world(
        double x_start, // distance along path from closest approach at which we start the raymarch
        double x_stop,  // distance along path from closest approach at which we stop the raymarch
        double z2,      // distance at closest approach, squared
        double r,       // radius of the planet
        double H        // scale height of the planet's atmosphere
    ) const;

    // Helper functions here
    static Length DistanceToSphere(Length r, Length rmu, Length sphere_radius);
};

#endif  // ATMOSPHERE_MODEL_TAYLOR_H_