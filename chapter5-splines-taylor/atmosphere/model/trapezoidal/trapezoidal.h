#ifndef ATMOSPHERE_MODEL_TRAPEZOIDAL_H_
#define ATMOSPHERE_MODEL_TRAPEZOIDAL_H_

#include "atmosphere/atmosphere.h"
#include "math/angle.h"
#include "physics/units.h"
#include <cstdint>

/* Trapezoidal method */
class Trapezoidal : public Atmosphere {
 public:
  Trapezoidal();

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

    // Helper functions here
    static Length DistanceToSphere(Length r, Length rmu, Length sphere_radius);
};

#endif  // ATMOSPHERE_MODEL_TRAPEZOIDAL_H_