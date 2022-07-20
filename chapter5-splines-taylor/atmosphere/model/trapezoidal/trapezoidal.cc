#include "atmosphere/model/trapezoidal/trapezoidal.h"
#include "math/vector.h"
#include <iostream>

typedef dimensional::Vector3<Length> Position;
typedef dimensional::Vector3<Length> Vector;
typedef dimensional::Vector3<Number> Direction;

Trapezoidal::Trapezoidal() 
    : num_samples(8),
      num_samples_light(128)
{
    std::cout << "TRAPEZOIDAL RULE INFO" << std::endl;
    std::cout << "VIEW SAMPLES = " << num_samples << std::endl << std::endl;
}

IrradianceSpectrum Trapezoidal::GetSunIrradiance(Length altitude, 
                                         Angle sun_zenith) const
{
#ifdef A_USE_UNIFORM_IRRADIANCE_METHOD
    return IrradianceSpectrum(0.0 * watt_per_square_meter_per_nm);
#else
    // TODO: Make this code uniform across all implementations (maybe default Irradiance Spectrum)
    // I hope this code is ok...
    Length len(1.0 * m);
    Number light_angle            = cos(sun_zenith);
    Number rayleigh_optical_depth = exp(-altitude / RayleighScaleHeight) * light_angle * Number(1.0 / num_samples_light);
    Number mie_optical_depth      = exp(-altitude / MieScaleHeight)      * light_angle * Number(1.0 / num_samples_light);

    DimensionlessSpectrum transmittance = exp(-(MieExtinction()      * len * mie_optical_depth * 1.1 + 
                                                RayleighScattering() * len * rayleigh_optical_depth));

    return transmittance * SolarSpectrum();
#endif
}

RadianceSpectrum Trapezoidal::GetSkyRadiance(Length altitude, 
                                     Angle sun_zenith, 
                                     Angle view_zenith, 
                                     Angle view_sun_azimuth) const
{
    Length inner_radius  = EarthRadius;
    Length camera_height = inner_radius + altitude;

    Position  camera_position(0.0 * m, 0.0 * m, camera_height);
    Direction light_direction(cos(view_sun_azimuth) * sin(sun_zenith),
                              sin(view_sun_azimuth) * sin(sun_zenith), 
                              cos(sun_zenith));

    // Get the ray from the camera, and its length.
    Direction ray_direction(sin(view_zenith), 0.0, cos(view_zenith));
    Length far = DistanceToSphere(camera_height, 
                                  camera_height * cos(view_zenith), 
                                  AtmosphereRadius);

    // Calculate the ray's starting position
    Position ray_origin = camera_position;
    
    // Initialize the scattering loop variables.
    Length segment_length = far / float(num_samples);
    Length t_current      = Length(0.0 * m);

    /* Rayleigh and Mie contributions */
    WavelengthFunction<1, 0, 0, 0, 0> sum_r(0.0 * m);
    WavelengthFunction<1, 0, 0, 0, 0> sum_m(0.0 * m);

    Length optical_depth_r(0.0 * m);
    Length optical_depth_m(0.0 * m);

    float coeff = 1.0f;
    for(uint32_t i = 0; i < num_samples + 1; ++i)
    {
        coeff = (i == 0 || i == num_samples) ? 2.0f : 1.0f;

        Position sample_position = ray_origin + (ray_direction * t_current);
        Length height = length(sample_position);

        /* compute optical depth for view ray */
        Length hr = exp((inner_radius - height) / RayleighScaleHeight) * segment_length / coeff;
        Length hm = exp((inner_radius - height) / MieScaleHeight)      * segment_length / coeff;

        optical_depth_r += hr;
        optical_depth_m += hm;
        
        /* Compute in-scattering */
        Length far_light = DistanceToSphere(height, 
                                            height * cos(sun_zenith), 
                                            AtmosphereRadius);

        // Calculate the light ray's starting position
        Position light_ray_origin = sample_position;
        
        // Initialize the scattering loop variables.
        Length segment_length_light  = far_light / float(num_samples_light);
        Length t_current_light = Length(0.0 * m);

        Length optical_depth_light_r(0.0 * m);
        Length optical_depth_light_m(0.0 * m);

        uint32_t j;
        float coeff_light = 1.0f;
        for(j = 0; j < num_samples_light + 1; ++j)
        {
            coeff_light = (j == 0 || j == num_samples_light) ? 2.0f : 1.0f;

            Position sample_position_light = light_ray_origin + (light_direction * t_current_light);
            Length height_light = length(sample_position_light);
            
            if(height_light < EarthRadius)
            {
                break;
            }

            optical_depth_light_r += exp((inner_radius - height_light) / RayleighScaleHeight) * segment_length_light / coeff_light;
            optical_depth_light_m += exp((inner_radius - height_light) / MieScaleHeight)      * segment_length_light / coeff_light;

            t_current_light += segment_length_light;
        }

        if(j == num_samples_light + 1)
        {
            DimensionlessSpectrum tau = RayleighScattering() * (optical_depth_r + optical_depth_light_r) + 
                                        MieExtinction()      * (optical_depth_m + optical_depth_light_m);
            DimensionlessSpectrum attenuation = exp(-tau);

            sum_r += attenuation * hr;
            sum_m += attenuation * hm;
         }

         t_current += segment_length;
    }

    Number nu = dot(ray_direction, light_direction);
    InverseSolidAngle rayleigh_phase = RayleighPhaseFunction(nu);
    InverseSolidAngle mie_phase      = MiePhaseFunction(nu);

    return (sum_r * RayleighScattering() * rayleigh_phase +
            sum_m * MieExtinction()      * mie_phase) * SolarSpectrum();

}

// Returns the distance from a point at radius r to the sphere of radius
// sphere_radius in a direction whose angle with the local vertical is
// acos(rmu / r), or 0 if there is no intersection.
Length Trapezoidal::DistanceToSphere(Length r, Length rmu, Length sphere_radius) {
  Area delta_sq = sphere_radius * sphere_radius - r * r + rmu * rmu;
  return delta_sq < 0.0 * m2 ? 0.0 * m :
      (r < sphere_radius ? -rmu + sqrt(delta_sq) : -rmu - sqrt(delta_sq));
}