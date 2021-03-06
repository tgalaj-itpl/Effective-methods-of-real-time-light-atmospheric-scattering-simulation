/**
 Copyright (c) 2015 Eric Bruneton
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 3. Neither the name of the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software without
 specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ATMOSPHERE_ATMOSPHERE_H_
#define ATMOSPHERE_ATMOSPHERE_H_

#include "math/angle.h"
#include "physics/units.h"
#include <glm/glm.hpp>

constexpr Length EarthRadius = 6360.0 * km;
constexpr Length AtmosphereRadius = 6420.0 * km;

// The height of the atmosphere if its density was uniform.
constexpr Length RayleighScaleHeight = 8000.0 * m;

// The height of the Mie particle layer if its density was uniform.
constexpr Length MieScaleHeight = 1200.0 * m;

// The Angstrom alpha coefficient for the Mie optical depth.
constexpr double MieAngstromAlpha = 0.8;

// The Angstrom beta coefficient for the Mie optical depth.
constexpr double MieAngstromBeta = 0.04;

// The g parameter of the Cornette-Shanks phase function used for Mie particles.
constexpr double MiePhaseFunctionG = 0.7;

// The Linke turbidity value, for the Preetham and Hosek models.
constexpr double Turbidity = 2.53;

// Returns the spectral irradiance of the Sun at the top of the atmosphere.
const IrradianceSpectrum &SolarSpectrum();

// Returns the Rayleigh scattering coefficient at sea level.
const ScatteringSpectrum &RayleighScattering();

// Returns the Mie extinction coefficient at sea level.
const ScatteringSpectrum &MieExtinction();

// Returns the Mie scattering coefficient at sea level.
const ScatteringSpectrum &MieScattering();

// Returns the Mie extinction and scattering coefficients at sea level for
// the given Angstrom parameters.
ScatteringSpectrum MieExtinction(double angstrom_alpha, double angstrom_beta);
ScatteringSpectrum MieScattering(double angstrom_alpha, double angstrom_beta);

// Computes the Rayleigh phase function for the given scattering angle.
// The integral of this function over all solid angles is 1.
InverseSolidAngle RayleighPhaseFunction(Angle scattering_angle);
InverseSolidAngle RayleighPhaseFunction(Number scattering_angle_cosine);

// Computes the Mie phase function for the given scattering angle (using
// the Cornette-Shanks approximation).
// The integral of this function over all solid angles is 1.
InverseSolidAngle MiePhaseFunction(Angle scattering_angle);
InverseSolidAngle MiePhaseFunction(Number scattering_angle_cosine);
InverseSolidAngle MiePhaseFunction(double g, Number scattering_angle_cosine);

const DimensionlessSpectrum &GroundAlbedo();

class Ray
{
public:
    Ray() : Ray(glm::highp_dvec3(0.0f), glm::highp_dvec3(0.0, 0.0, -1.0)) {}

    Ray(const glm::highp_dvec3 & origin, const glm::highp_dvec3 & direction) 
        : m_origin(origin), 
          m_direction(direction),
          t_min(0.1f),
          t_max(1000.0f)
    {
        m_direction = glm::normalize(m_direction);
        is_valid = false;
    }

    glm::highp_dvec3 m_origin;
    glm::highp_dvec3 m_direction; // normalized
    
    const double t_min, t_max;
    bool is_valid;
};

// An abstract atmosphere model.
class Atmosphere
{
public:
    virtual ~Atmosphere() {}

    // Returns the number of wavelengths originally used by this atmosphere model.
    // The default implementation returns spectrum::NUM_WAVELENGTH.
    virtual int GetOriginalNumberOfWavelengths() const;

    // Returns the irradiance on a plane perpendicular to the Sun direction.
    virtual IrradianceSpectrum GetSunIrradiance(Length altitude,
                                                Angle sun_zenith) const = 0;

    virtual RadianceSpectrum GetSkyRadiance(Length altitude, Angle sun_zenith,
                                            Angle view_zenith, Angle view_sun_azimuth) const = 0;

    virtual RadianceSpectrum GetSkyRadiance(Length altitude, Angle sun_zenith,
                                            Angle sun_azimuth, Angle view_zenith, Angle view_azimuth) const;

    // Returns the irradiance on a horizontal plane.
    virtual IrradianceSpectrum GetSkyIrradiance(Length altitude,
                                                Angle sun_zenith) const;

protected:
    static Angle GetViewSunAngle(Angle sun_zenith, Angle view_zenith,
                                 Angle view_sun_azimuth);
};

#endif // ATMOSPHERE_ATMOSPHERE_H_
