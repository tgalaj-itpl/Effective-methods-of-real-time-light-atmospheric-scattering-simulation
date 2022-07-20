#include "atmosphere/model/taylor/taylor.h"
#include "math/vector.h"
#include <glm/glm.hpp>

typedef dimensional::Vector3<Length> Position;
typedef dimensional::Vector3<Length> Vector;
typedef dimensional::Vector3<Number> Direction;

Taylor::Taylor() : num_samples(1024),
                   num_samples_light(1)
{
    
}

IrradianceSpectrum Taylor::GetSunIrradiance(Length altitude, 
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

    DimensionlessSpectrum transmittance = exp(-(MieExtinction()      * len * mie_optical_depth + 
                                                RayleighScattering() * len * rayleigh_optical_depth));

    return transmittance * SolarSpectrum();
#endif
}

RadianceSpectrum Taylor::GetSkyRadiance(Length altitude, 
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
    Length segment_length = far / double(num_samples);
    Length t_current      = Length(0.0 * m);

    /* Rayleigh and Mie contributions */
    WavelengthFunction<1, 0, 0, 0, 0> sum_r(0.0 * m);
    WavelengthFunction<1, 0, 0, 0, 0> sum_m(0.0 * m);

    Length optical_depth_r(0.0 * m);
    Length optical_depth_m(0.0 * m);
    
    double planet_radius = inner_radius.to(m);

    for(uint32_t i = 0; i < num_samples; ++i)
    {
        Position sample_position = ray_origin + ray_direction * (t_current + segment_length * Number(0.5));
        Length height = length(sample_position);

        /* compute optical depth for view ray */
        Length hr = exp((inner_radius - height) / RayleighScaleHeight) * segment_length;
        Length hm = exp((inner_radius - height) / MieScaleHeight)      * segment_length;

        optical_depth_r += hr;
        optical_depth_m += hm;
        
        /* Compute in-scattering */
        Length far_light = DistanceToSphere(height, 
                                            height * cos(sun_zenith), 
                                            AtmosphereRadius);

        // Calculate the light ray's starting position
        Position light_ray_origin = sample_position;
        Length optical_depth_light_r(0.0 * m);
        Length optical_depth_light_m(0.0 * m);

        auto light_pos = glm::highp_dvec3(light_ray_origin.x.to(m), light_ray_origin.y.to(m), light_ray_origin.z.to(m));
        auto sun_light = glm::highp_dvec3(light_direction.x.to(Number::Unit()), light_direction.y.to(Number::Unit()), light_direction.z.to(Number::Unit()));

        Ray light_ray(light_pos, sun_light);

        auto od_light_r = approx_air_column_density_ratio_along_3d_ray_for_curved_world(light_ray.m_origin, light_ray.m_direction, far_light.to(m), planet_radius, RayleighScaleHeight.to(m));
        auto od_light_m = approx_air_column_density_ratio_along_3d_ray_for_curved_world(light_ray.m_origin, light_ray.m_direction, far_light.to(m), planet_radius, MieScaleHeight.to(m));

        optical_depth_light_r = od_light_r * m;
        optical_depth_light_m = od_light_m * m;

        DimensionlessSpectrum tau = RayleighScattering() * (optical_depth_r + optical_depth_light_r) + 
                                    MieExtinction()      * (optical_depth_m + optical_depth_light_m);
        DimensionlessSpectrum attenuation = exp(-tau);

        sum_r += attenuation * hr;
        sum_m += attenuation * hm;

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
Length Taylor::DistanceToSphere(Length r, Length rmu, Length sphere_radius) {
  Area delta_sq = sphere_radius * sphere_radius - r * r + rmu * rmu;
  return delta_sq < 0.0 * m2 ? 0.0 * m :
      (r < sphere_radius ? -rmu + sqrt(delta_sq) : -rmu - sqrt(delta_sq));
}

double Taylor::approx_air_column_density_ratio_along_3d_ray_for_curved_world(glm::highp_dvec3 P, glm::highp_dvec3 V, double x, double r, double H) const
{
    double xz = glm::dot(-P,V);           // distance ("radius") from the ray to the center of the world at closest approach, squared
    double z2 = glm::dot( P,P) - xz * xz; // distance from the origin at which closest approach occurs

    return approx_air_column_density_ratio_along_2d_ray_for_curved_world( 0.0-xz, x-xz, z2, r, H );
}

double Taylor::approx_air_column_density_ratio_along_2d_ray_for_curved_world(double x_start, double x_stop, double z2, double r, double H) const
{
    // GUIDE TO VARIABLE NAMES:
    //  "x*" distance along the ray from closest approach
    //  "z*" distance from the center of the world at closest approach
    //  "r*" distance ("radius") from the center of the world
    //  "h*" distance ("height") from the surface of the world
    //  "*b" variable at which the slope and intercept of the height approximation is sampled
    //  "*0" variable at which the surface of the world occurs
    //  "*1" variable at which the top of the atmosphere occurs
    //  "*2" the square of a variable
    //  "d*dx" a derivative, a rate of change over distance along the ray

    double a = 0.45;
    double b = 0.45;
    double inf = std::numeric_limits<double>::max();

    double x0 = std::sqrt(glm::max(r * r - z2, 0.0));

    // if ray is obstructed
    if (x_start < x0 && -x0 < x_stop && z2 < r * r)
    {
        // return ludicrously big number to represent obstruction
        return inf;
    }
    
    double r1       = r + 6.0 * H;
    double x1       = std::sqrt(glm::max(glm::abs(r1 * r1 - z2), 0.0));
    double xb       = x0 + (x1 - x0) * b;
    double rb2      = xb * xb + z2;
    double rb       = std::sqrt(rb2);
    double d2hdx2   = z2 / std::sqrt(rb2 * rb2 * rb2);
    double dhdx     = xb / rb; 
    double hb       = rb - r;
    double dx0      = x0 - xb;
    double dx_stop  = std::abs(x_stop ) - xb;
    double dx_start = std::abs(x_start) - xb;
    double h0       = (0.5 * a * d2hdx2 * dx0      + dhdx) * dx0      + hb;
    double h_stop   = (0.5 * a * d2hdx2 * dx_stop  + dhdx) * dx_stop  + hb;
    double h_start  = (0.5 * a * d2hdx2 * dx_start + dhdx) * dx_start + hb;

    double rho0  =   std::exp(-h0 / H);
    double sigma =   glm::sign(x_stop ) * glm::max(H / dhdx * (rho0 - std::exp(-h_stop  / H)), 0.0) 
                   - glm::sign(x_start) * glm::max(H / dhdx * (rho0 - std::exp(-h_start / H)), 0.0);

    // NOTE: we clamp the result to prevent the generation of inifinities and nans, 
    // which can cause graphical artifacts.
    return glm::min<double>(std::abs(sigma), inf);
}