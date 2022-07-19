import math
import glm

class Camera:
    def __init__(self, width: float, height: float, fov: float, position: glm.vec3, pitch: float = 0.0, yaw: float = 0.0):
        self.width = width
        self.height = height
        self.fov = fov
        self.position = position
        self.aspect_ratio = float(width) / float(height)
        
        self.update()
        
        yaw   = glm.radians(-90.0 - yaw)
        pitch = glm.radians(-pitch)

        self.cam_dir.x = glm.cos(yaw) * glm.cos(pitch)
        self.cam_dir.y = glm.sin(pitch)
        self.cam_dir.z = glm.sin(yaw) * glm.cos(pitch)
        self.cam_dir   = glm.normalize(self.cam_dir)

        self.cam_transform = glm.lookAt(self.position, self.position + self.cam_dir, glm.vec3(0.0, 1.0, 0.0))

    def __get_primary_ray_fisheye_camera(self, x: float, y: float):
        ray_direction = glm.vec3(0.0)

        fisheye_cam_x = 2.0 * x / float(self.width - 1) - 1.0
        fisheye_cam_y = 2.0 * y / float(self.height - 1) - 1.0
        z2 = fisheye_cam_x * fisheye_cam_x + fisheye_cam_y * fisheye_cam_y

        if z2 <= 1.0:
            phi = math.atan2(fisheye_cam_y, fisheye_cam_x)
            theta = math.acos(1.0 - z2)

            ray_direction = glm.vec3(glm.sin(theta) * glm.cos(phi), glm.cos(theta), glm.sin(theta) * glm.sin(phi))
            ray_direction = glm.vec3(self.cam_transform * glm.vec4(ray_direction, 0.0))
            ray_direction = glm.normalize(ray_direction)

        return self.position, ray_direction

    def __get_primary_ray_normal_camera(self, x: float, y: float):
        pixel_cam_x = (2.0 * (x / self.width) - 1.0) * self.aspect_ratio * self.angle
        pixel_cam_y = (1.0 - 2.0 * (y / self.height)) * self.angle

        ray_direction = glm.vec3(pixel_cam_x, pixel_cam_y, -1.0)
        ray_direction = glm.vec3(self.cam_transform * glm.vec4(ray_direction, 0.0))
        ray_direction = glm.normalize(ray_direction)

        return self.position, ray_direction

    def get_primary_ray(self, x: float, y: float):
        if self.fisheye:
            return self.__get_primary_ray_fisheye_camera(x, y)
        
        return self.__get_primary_ray_normal_camera(x, y)

    def update(self):
        self.angle = glm.tan(glm.radians(self.fov * 0.5))

    # public members
    width: float = 0.0
    height: float = 0.0
    aspect_ratio: float = 0.0
    position = glm.vec3(0.0)
    fov: float = 60.0
    fisheye = False

    cam_dir   = glm.vec3(0.0)

    cam_transform = glm.mat4(1.0)
    angle = 0.0