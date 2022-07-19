import numpy as np
from keras.models import Sequential

import glm
import utils
from atmo import Atmosphere

class AtmoImgBased(Atmosphere):
    def __init__(self, nn_model: Sequential, planet_radius=6360e3, atmopshere_thickness=100e3, light_dir=glm.vec3(0, 1, 0)):
        super().__init__(planet_radius=planet_radius, atmopshere_thickness=atmopshere_thickness, light_dir=light_dir)
        self.nn_model = nn_model
        self.m_zenith_angle = 0.0
        self.m_azimuth_angle = 0.0

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

        if (t_min >= 0.0):
            P_v = origin + view_dir * t_min

        # Fetch predicted pixel value from NN
        datax = np.zeros((1, 5), dtype=float)
        datax[0] = [self.m_zenith_angle / np.pi, self.m_azimuth_angle / np.pi, R_v.x, R_v.y, R_v.z]

        predicted_pixel = self.nn_model.predict(datax)
        color = glm.vec3(predicted_pixel[0][0], predicted_pixel[0][1], predicted_pixel[0][2])
        color = 10.0 * color / (1.0 - color)

        return color