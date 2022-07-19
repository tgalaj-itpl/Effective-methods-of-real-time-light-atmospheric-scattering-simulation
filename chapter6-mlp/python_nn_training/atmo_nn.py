import numpy as np
from keras.models import Sequential

import glm
import utils
from atmo import Atmosphere

class AtmoNN(Atmosphere):
    def __init__(self, nn_model: Sequential, max_planet_radius=6360e3, planet_radius=6360e3, atmopshere_thickness=100e3, BETA_R=glm.vec3(3.8e-6, 13.5e-6, 33.1e-6), BETA_M=glm.vec3(21e-6), light_dir=glm.vec3(0, 1, 0), is_multi_planets=False):
        super().__init__(planet_radius=planet_radius, atmopshere_thickness=atmopshere_thickness, BETA_R=BETA_R, BETA_M=BETA_M, light_dir=light_dir)

        self.max_planet_radius = max_planet_radius * self.scaling_factor
        self.nn_model = nn_model
        self.is_multi_planets = is_multi_planets

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

        P_v = origin
        R_v = view_dir

        if (t0 > 0.0):
            P_v = origin + view_dir * t0

        # Calculate light intensity for P_v
        view_angle = glm.acos(glm.dot(glm.normalize(P_v), R_v))
        sun_angle  = glm.acos(glm.dot(glm.normalize(P_v), self.light_dir))
        height     = glm.length(P_v) - self.planet_radius

        # Fetch scattering values from NN
        H_TOP = float(self.atmopshere_radius - self.planet_radius)

        # For single planet NN
        datax = np.zeros((1, 3), dtype=float)
        datax[0] = [height / H_TOP, sun_angle / np.pi, view_angle / np.pi]

        # For multi planets NN
        if self.is_multi_planets:
            planet_r = self.planet_radius / self.max_planet_radius
            atmo_r   = self.atmopshere_radius / self.max_planet_radius

            datax = np.zeros((1, 5), dtype=float)
            datax[0] = [height / H_TOP, sun_angle / np.pi, view_angle / np.pi, planet_r, atmo_r]

        I_v = self.nn_model.predict(datax)

        beta_r = self.BETA_R
        beta_m = self.BETA_M

        i_rayleigh = glm.vec4(I_v[0][0], I_v[0][1], I_v[0][2], I_v[0][3])
        i_mie = glm.vec3(i_rayleigh) * i_rayleigh.a * beta_r.r * beta_m / (i_rayleigh.r * beta_m.r * beta_r + 0.00001)

        mu = glm.dot(view_dir, self.light_dir)
        g = 0.76

        phase_r = utils.rayleigh_phase_func(mu)
        phase_m = utils.mie_phase_func(g, mu)

        res = (glm.vec3(i_rayleigh) * phase_r + i_mie * phase_m) * 13.661

        return res