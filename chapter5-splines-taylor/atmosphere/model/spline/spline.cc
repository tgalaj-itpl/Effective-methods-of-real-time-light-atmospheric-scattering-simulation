#include "atmosphere/model/spline/spline.h"
#include "atmosphere/model/spline/Spline2dSolution.h"
#include <iomanip>
#include "timing.h"

Spline::Spline() : num_samples(1024),
                   num_samples_light(1),
                   planet_radius(EarthRadius.to(m)),
                   atmosphere_radius(AtmosphereRadius.to(m)),
                   h_rayleigh(RayleighScaleHeight.to(m)),
                   h_mie(MieScaleHeight.to(m)),
                   inner_radius(EarthRadius)
{

    int height_points     = 50;
    int distance_points   = 20;

    double scaling_factor = 1.0;

    std::cout << "SPLINE RULE INFO" << std::endl;
    std::cout << "HEIGHT POINTS   = " << height_points << std::endl;
    std::cout << "DISTANCE POINTS = " << distance_points << std::endl;
    std::cout << "VIEW SAMPLES    = " << num_samples << std::endl << std::endl;

    spline2d_rayleigh_below = std::unique_ptr<solution::Spline2dPrecomputation>(new solution::Spline2dPrecomputation(height_points, distance_points, h_rayleigh, solution::Spline2dPrecomputation::Below, planet_radius, atmosphere_radius, scaling_factor));
    spline2d_rayleigh_above = std::unique_ptr<solution::Spline2dPrecomputation>(new solution::Spline2dPrecomputation(height_points, distance_points, h_rayleigh, solution::Spline2dPrecomputation::Above, planet_radius, atmosphere_radius, scaling_factor));

    spline2d_mie_below = std::unique_ptr<solution::Spline2dPrecomputation>(new solution::Spline2dPrecomputation(height_points, distance_points, h_mie, solution::Spline2dPrecomputation::Below, planet_radius, atmosphere_radius, scaling_factor));
    spline2d_mie_above = std::unique_ptr<solution::Spline2dPrecomputation>(new solution::Spline2dPrecomputation(height_points, distance_points, h_mie, solution::Spline2dPrecomputation::Above, planet_radius, atmosphere_radius, scaling_factor));
}

IrradianceSpectrum Spline::GetSunIrradiance(Length altitude,
                                            Angle sun_zenith) const
{
#ifdef A_USE_UNIFORM_IRRADIANCE_METHOD
    return IrradianceSpectrum(0.0 * watt_per_square_meter_per_nm);
#else
    // TODO: Make this code uniform across all implementations (maybe default Irradiance Spectrum)
    // I hope this code is ok...
    Length len(1.0 * m);
    Number light_angle = cos(sun_zenith);
    Number rayleigh_optical_depth = exp(-altitude / RayleighScaleHeight) * light_angle * Number(1.0 / num_samples_light);
    Number mie_optical_depth = exp(-altitude / MieScaleHeight) * light_angle * Number(1.0 / num_samples_light);

    DimensionlessSpectrum transmittance = exp(-(MieExtinction() * len * mie_optical_depth * 1.1 +
                                                RayleighScattering() * len * rayleigh_optical_depth));

    return transmittance * SolarSpectrum();
#endif
}

RadianceSpectrum Spline::GetSkyRadiance(Length altitude,
                                        Angle sun_zenith,
                                        Angle view_zenith,
                                        Angle view_sun_azimuth) const
{
    Length camera_height = inner_radius + altitude;

    Position camera_position(0.0 * m, 0.0 * m, camera_height);
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
    Length segment_length = far / num_samples;
    Length t_current = Length(0.0 * m);

    /* Rayleigh and Mie contributions */
    WavelengthFunction<1, 0, 0, 0, 0> sum_r(0.0 * m);
    WavelengthFunction<1, 0, 0, 0, 0> sum_m(0.0 * m);

    Length optical_depth_r(0.0 * m);
    Length optical_depth_m(0.0 * m);

    auto r_o = glm::highp_dvec3(ray_origin.x.to(m), ray_origin.y.to(m), ray_origin.z.to(m));
    auto r_d = glm::highp_dvec3(ray_direction.x.to(Number::Unit()), ray_direction.y.to(Number::Unit()), ray_direction.z.to(Number::Unit()));

    Ray ray(r_o, r_d);

    for (uint32_t i = 0; i < num_samples; ++i)
    {
        Position sample_position_a = ray_origin + (ray_direction * t_current);
        Position sample_position_b = ray_origin + (ray_direction * (t_current + segment_length));

        Position sample_position = (sample_position_a + sample_position_b) / Number(2.0);
        Length height = length(sample_position);

        /* compute optical depth for view ray */
        Length hr = exp((inner_radius - height) / RayleighScaleHeight) * segment_length;
        Length hm = exp((inner_radius - height) / MieScaleHeight)      * segment_length;

        glm::highp_dvec3 pa = ray.m_origin + ray.m_direction * (t_current).to(m);
        glm::highp_dvec3 pe = ray.m_origin + ray.m_direction * (t_current + segment_length).to(m);
        auto mid_sample_point = (pa + pe) / 2.0;

        optical_depth_r += hr;
        optical_depth_m += hm;

        // Initialize the scattering loop variables.
        Length optical_depth_light_r(0.0 * m);
        Length optical_depth_light_m(0.0 * m);

        Length t_min_light(0.0 * m), t_max_light(0.0 * m);
        Length t0_light_e(0.0 * m), t1_light_e(0.0 * m);
        Length t0_light(0.0 * m), t1_light(0.0 * m);

        auto light_dir = glm::highp_dvec3(light_direction.x.to(Number::Unit()), light_direction.y.to(Number::Unit()), light_direction.z.to(Number::Unit()));
        Ray light_ray(mid_sample_point, light_dir);

        bool is_light = true;

        t1_light_e = DistanceToSphere(height, height * cos(sun_zenith), EarthRadius);
        t1_light   = DistanceToSphere(height, height * cos(sun_zenith), AtmosphereRadius);

        if (t1_light_e > Length(0.0 * m) || t1_light < Length(0.0 * m))
            is_light = false;

        if (is_light)
        {
            if (t0_light > Length(0.0 * m))
            {
                t_min_light = t0_light;
            }

            t_max_light = t1_light;

            glm::highp_dvec3 pa_light = light_ray.m_origin + t_min_light.to(m) * light_ray.m_direction;
            glm::highp_dvec3 pe_light = light_ray.m_origin + t_max_light.to(m) * light_ray.m_direction;

            auto optical_depth_light = const_cast<Spline*>(this)->getIntegralValue(pa_light, pe_light, light_ray);
            optical_depth_light_r = optical_depth_light.x * m;
            optical_depth_light_m = optical_depth_light.y * m;

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
Length Spline::DistanceToSphere(Length r, Length rmu, Length sphere_radius)
{
    Area delta_sq = sphere_radius * sphere_radius - r * r + rmu * rmu;
    return delta_sq < 0.0 * m2 ? 0.0 * m : (r < sphere_radius ? -rmu + sqrt(delta_sq) : -rmu - sqrt(delta_sq));
}

glm::highp_dvec2 Spline::getIntegralValue(const glm::highp_dvec3 &pa, const glm::highp_dvec3 &pe, const Ray &ray)
{
    /*
     * Rayleigh value is stored in the x field
     * Mie      value is stored in the y field.
     */
    glm::highp_dvec2 integral_value;

    double paDistance = glm::length(pa);
    double paHeight = paDistance - planet_radius;
    double peDistance = glm::length(pe);
    double peHeight = peDistance - planet_radius;
    double rayDistance = glm::length(glm::cross(pa, ray.m_direction));

    paHeight = glm::clamp(paHeight, 0.0, atmosphere_radius - planet_radius);
    peHeight = glm::clamp(peHeight, 0.0, atmosphere_radius - planet_radius);

    /* Below case */
    if (rayDistance < planet_radius - spline2d_rayleigh_above->offset)
    {
#if USE_SPLINE_PRECOMPUTATION
        integral_value.x = spline2d_rayleigh_below->integralValue(paHeight, peHeight, rayDistance);
        integral_value.y = spline2d_mie_below->integralValue(paHeight, peHeight, rayDistance);
#else
        const auto splineRayleigh(spline2d_rayleigh_below->spline2dToSpline1d(rayDistance));
        const auto splineMie(spline2d_mie_below->spline2dToSpline1d(rayDistance));

        integral_value.x = spline2d_rayleigh_below->integralValue(splineRayleigh, paHeight, peHeight, rayDistance);
        integral_value.y = spline2d_mie_below->integralValue(splineMie, paHeight, peHeight, rayDistance);
#endif
    }
    /* Above case */
    else
    {
#if !USE_SPLINE_PRECOMPUTATION
        const auto splineRayleigh(spline2d_rayleigh_above->spline2dToSpline1d(rayDistance));
        const auto splineMie(spline2d_mie_above->spline2dToSpline1d(rayDistance));
#endif

        double rayLength = glm::length(pe - pa);
        bool isOnTheSameSide = (peDistance * peDistance - rayLength * rayLength - paDistance * paDistance > 0);

        if (isOnTheSameSide)
        {
#if USE_SPLINE_PRECOMPUTATION
            integral_value.x = spline2d_rayleigh_above->integralValue(paHeight, peHeight, rayDistance);
            integral_value.y = spline2d_mie_above->integralValue(paHeight, peHeight, rayDistance);
#else
            integral_value.x = spline2d_rayleigh_above->integralValue(splineRayleigh, paHeight, peHeight, rayDistance);
            integral_value.y = spline2d_mie_above->integralValue(splineMie, paHeight, peHeight, rayDistance);
#endif
        }
        else
        {
            double rayHeight = rayDistance - planet_radius;
            double pLowerHeight = ((paHeight <= peHeight) ? paHeight : peHeight);
            double pHigherHeight = ((paHeight > peHeight) ? paHeight : peHeight);

#if USE_SPLINE_PRECOMPUTATION
            integral_value.x = 2.0 * spline2d_rayleigh_above->integralValue(rayHeight, pLowerHeight, rayDistance) +
                               spline2d_rayleigh_above->integralValue(pLowerHeight, pHigherHeight, rayDistance);

            integral_value.y = 2.0 * spline2d_mie_above->integralValue(rayHeight, pLowerHeight, rayDistance) +
                               spline2d_mie_above->integralValue(pLowerHeight, pHigherHeight, rayDistance);
#else
            integral_value.x = 2.0 * spline2d_rayleigh_above->integralValue(splineRayleigh, rayHeight, pLowerHeight, rayDistance) +
                               spline2d_rayleigh_above->integralValue(splineRayleigh, pLowerHeight, pHigherHeight, rayDistance);

            integral_value.y = 2.0 * spline2d_mie_above->integralValue(splineMie, rayHeight, pLowerHeight, rayDistance) +
                               spline2d_mie_above->integralValue(splineMie, pLowerHeight, pHigherHeight, rayDistance);
#endif
        }
    }

    return integral_value;
}
