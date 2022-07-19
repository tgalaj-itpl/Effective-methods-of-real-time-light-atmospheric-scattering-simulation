import glm
import utils

class Atmosphere:
    '''
    This class is responsible for rendering the atmosphere (colors of the sky)
    '''
    def __init__(self, planet_radius=6360e3, atmopshere_thickness=100e3, H_r=8e3, H_m=1.2e3, BETA_R=glm.vec3(3.8e-6, 13.5e-6, 33.1e-6), BETA_M=glm.vec3(21e-6), light_dir=glm.vec3(0, 1, 0)):
        self.light_dir = glm.normalize(light_dir)
        self.scaling_factor = 1.0 / planet_radius

        self.planet_radius = planet_radius * self.scaling_factor
        self.atmopshere_radius = (planet_radius + atmopshere_thickness) * self.scaling_factor
        self.H_r = H_r * self.scaling_factor
        self.H_m = H_m * self.scaling_factor
        self.BETA_R = BETA_R * (1.0 / self.scaling_factor)
        self.BETA_M = BETA_M * (1.0 / self.scaling_factor)

    def compute_radiance(self, origin: glm.vec3, view_dir: glm.vec3, t_min: float, t_max: float):
        t0 = 0.0
        t1 = 0.0

        is_atmosphere_intersection, t0, t1 = utils.ray_sphere_intersection(origin, view_dir, self.atmopshere_radius)
        if (not is_atmosphere_intersection) or (t1 < 0.0):
            return glm.vec3(0.0)

        if t0 > t_min and t0 > 0.0:
            t_min = 0.0

        if t1 < t_max:
            t_max = t1

        view_samples  = 16
        light_samples = 8

        segment_length = (t_max - t_min) / float(view_samples)
        tcurrent = t_min

        sum_r = glm.vec3(0.0)
        sum_m = glm.vec3(0.0)
        optical_depth_r = 0.0
        optical_depth_m = 0.0

        mu = glm.dot(view_dir, self.light_dir)
        g = 0.76

        phase_r = utils.rayleigh_phase_func(mu)
        phase_m = utils.mie_phase_func(g, mu)

        for i in range(view_samples):
            sample_position = origin + (tcurrent + segment_length * 0.5) * view_dir
            height = glm.length(sample_position) - self.planet_radius

            # compute optical depth on view ray (out-scattering)
            hr = glm.exp(-height / self.H_r) * segment_length
            hm = glm.exp(-height / self.H_m) * segment_length

            optical_depth_r += hr
            optical_depth_m += hm

            # compute optical depth on light ray (in-scattering)
            t1_light = 0.0

            _, _, t1_light = utils.ray_sphere_intersection(sample_position, self.light_dir, self.atmopshere_radius)
            
            segment_length_light = t1_light / float(light_samples)
            tcurrent_light = 0.0

            optical_depth_light_r = 0.0
            optical_depth_light_m = 0.0

            j = 0
            while j < light_samples:
                sample_position_light = sample_position + (tcurrent_light + segment_length_light * 0.5) * self.light_dir
                height_light = glm.length(sample_position_light) - self.planet_radius

                if height_light < 0.0: break

                optical_depth_light_r += glm.exp(-height / self.H_r) * segment_length_light
                optical_depth_light_m += glm.exp(-height / self.H_m) * segment_length_light

                tcurrent_light += segment_length_light
                j += 1

            if j == light_samples:
                transmittance: glm.vec3 = self.BETA_R * (optical_depth_r + optical_depth_light_r) + self.BETA_M * (optical_depth_m + optical_depth_light_m)
                attenuation: glm.vec3   = glm.exp(-transmittance)

                sum_r += attenuation * hr
                sum_m += attenuation * hm

            tcurrent += segment_length

        return (sum_r * phase_r * self.BETA_R + sum_m * phase_m * self.BETA_M) * 20.0