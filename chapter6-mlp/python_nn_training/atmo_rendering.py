import os
from timeit import default_timer as timer

import matplotlib.pyplot as plt
import copy
import numpy as np
from keras import backend
from keras.models import load_model

import glm
import utils
from atmo import Atmosphere
from atmo_img_based import AtmoImgBased
from atmo_nn import AtmoNN
from camera import Camera

# Methods in this file are responsible for rendering the sky 
# using previously trained models from model_training.py script

class Scene:
    '''
    Class Scene
    '''
    def __init__(self, img_width=640, img_height=480, output_filename="test.png", is_fisheye=False, cam_fov=60.0, cam_origin=glm.vec3(0.0), cam_pitch=0.0, cam_yaw=0.0, sun_angle=0.0):
        self.img_width = img_width
        self.img_height = img_height
        self.output_filename = output_filename
        self.is_fisheye = is_fisheye
        self.cam_fov = cam_fov
        self.cam_origin = cam_origin
        self.cam_pitch = cam_pitch
        self.cam_yaw = cam_yaw
        self.sun_angle = sun_angle

class Planet:
    '''
    Class Planet
    '''
    def __init__(self, name, planet_radius, atmosphere_thickness, beta_r, beta_m):
        self.name = name
        self.planet_radius = planet_radius
        self.atmosphere_radius = planet_radius + atmosphere_thickness
        self.beta_r = beta_r
        self.beta_m = beta_m
    
def rmse(y_true, y_pred):
    return backend.sqrt(backend.mean(backend.square(y_pred - y_true), axis=-1))

def render_skydome(filename: str, img_width: int, img_height: int, cam: Camera, atmo, tonemapping=True, exposure=1.0, vertical_flip=False):
    img     = np.zeros((img_height, img_width, 3))
    start_t = timer()

    # Loop through all pixels.
    utils.printProgressBar(0, img_width, prefix='[%s] Progress:' % filename, suffix='Complete', length=50)
    for y in range(img_height):
        for x in range(img_width):
            origin, direction = cam.get_primary_ray(x + 0.5, y + 0.5)

            is_planet_intersection, t0, t1 = utils.ray_sphere_intersection(origin, direction, atmo.planet_radius)
            t_max = np.inf

            if(is_planet_intersection and  t1 > 0.0): 
                t_max = glm.max(0.0, t0)

            pixel = atmo.compute_radiance(origin, direction, 0.0, t_max)

            if tonemapping:
                pixel = 1.0 - glm.exp(-pixel * exposure)    # exposure tone mapping (1 is exposure)
                pixel = glm.pow(pixel, glm.vec3(1.0 / 2.2)) # gamma correction

            if vertical_flip:
                img[img_height - y - 1, x, :] = np.clip(pixel, 0, 1)
            else:
                img[y, x, :] = np.clip(pixel, 0, 1)

        elapsed_time = 'Complete | Time elapsed: %.2f [s]' % (timer() - start_t)
        utils.printProgressBar(y + 1, img_height, prefix='[%s] Progress:' % filename, suffix=elapsed_time, length=50)

    plt.imsave(filename, img)
    plt.close()

def set_up_camera(scene: Scene, planet_radius: float, scaling_factor: float):
    '''
    Set up Camera settings
    '''

    cam_origin = copy.deepcopy(scene.cam_origin)
    cam_origin.y = -cam_origin.y

    cam_position = glm.vec3(0.0, planet_radius + (1000.0 * scaling_factor), 0.0) + (cam_origin * scaling_factor)

    camera = Camera(scene.img_width, scene.img_height, scene.cam_fov, cam_position, scene.cam_pitch, scene.cam_yaw)
    camera.fisheye = copy.deepcopy(scene.is_fisheye)

    return camera

def render_results_lut_based_single(scenes):
    #### Atmo NN ####
    print("Rendering using Atmo NN - Single")
    model = load_model("saved_nn_models/single_planet_10layers/nn_lut_512_128_128_128_128_rm_earth_10layers-587.h5", custom_objects={'rmse': rmse})
    model.summary()
    planet_name = "earth"

    for scene in scenes:
        scene.img_width = int(scene.img_width / 4)
        scene.img_height = int(scene.img_height / 4)

        sun_angle = glm.radians(scene.sun_angle)
        sun_dir   = glm.normalize(glm.vec3(0, glm.cos(sun_angle), -glm.sin(sun_angle)))

        atmosphere = AtmoNN(model, light_dir=sun_dir, is_multi_planets=False)

        camera = set_up_camera(scene, atmosphere.planet_radius, atmosphere.scaling_factor)
        render_skydome("res/nn_lut_" + planet_name + "_" + scene.output_filename + '.png', scene.img_width, scene.img_height, camera, atmosphere)

def render_results_lut_based_multi(scenes):
    #### Atmo NN ####
    print("Rendering using Atmo NN - Multi")
    model = load_model("saved_nn_models/multi_planets/nn_lut_512_128_128_128_128_rm_multi_planets-100.h5", custom_objects={'rmse': rmse})
    
    planets = [#Planet(name="earth", planet_radius=6360e3,   atmosphere_thickness=100e3,     beta_r=glm.vec3(3.8e-6, 13.5e-6, 33.1e-6),           beta_m=glm.vec3(21e-6)), 
               #Planet(name="venus", planet_radius=6052e3,   atmosphere_thickness=200.059e3, beta_r=glm.vec3(11.37e-6, 11.37e-6, 1.8e-6),         beta_m=glm.vec3(2.12153e-6)), 
               #Planet(name="mars",  planet_radius=3389.5e3, atmosphere_thickness=30.588e3,  beta_r=glm.vec3(23.918e-6, 13.57e-6, 5.78e-6),       beta_m=glm.vec3(5.12153e-6)),
               #Planet(name="im1",   planet_radius=4500e3,     atmosphere_thickness=150e3, beta_r=glm.vec3(7.72e-5, 15.24e-6, 8.72-5), beta_m=glm.vec3(3.53e-6)), # mars-like
               #Planet(name="im2",   planet_radius=5900e3,    atmosphere_thickness=400e3, beta_r=glm.vec3(3e-8, 56.24e-6, 64e-6),               beta_m=glm.vec3(8.53e-6)),   # venus-like
               Planet(name="im3",   planet_radius=6300e3,    atmosphere_thickness=200e3,  beta_r=glm.vec3(67.5e-6, 82.8e-6, 12.5e-8),            beta_m=glm.vec3(15e-6))    # earth-like
    ]
    max_planet_radius = 6360e3

    #planets = [#Planet(name="im1", planet_radius=2.0e6, atmosphere_thickness=1.0e4, beta_r=glm.vec3(1.14e-7, 5.0e-6, 3.0e-6), beta_m=glm.vec3(2.0e-6)),
               #Planet(name="im2", planet_radius=4.78e6, atmosphere_thickness=5.12e4, beta_r=glm.vec3(4.772e-5, 2.214e-5, 8.56e-6), beta_m=glm.vec3(8.24e-6)),
               #Planet(name="im3", planet_radius=4.0e6, atmosphere_thickness=1.5e5, beta_r=glm.vec3(1.0e-5, 5.0e-6, 1.2e-6), beta_m=glm.vec3(1.3e-6)),
               #Planet(name="im4",  planet_radius=8.1e6, atmosphere_thickness=2.5e5, beta_r=glm.vec3(1.27e-5, 1.77e-5, 2.4e-6), beta_m=glm.vec3(2.94e-6)),
               #Planet(name="im44", planet_radius=2.1e6, atmosphere_thickness=1.2e5, beta_r=glm.vec3(2.55e-5, 5.22e-5, 22.2e-6), beta_m=glm.vec3(2.5e-6)),
               #Planet(name="im5", planet_radius=5.0e6, atmosphere_thickness=5.0e4, beta_r=glm.vec3(2.0e-6, 5.0e-6, 1.5e-5), beta_m=glm.vec3(1.1e-5)),
               #Planet(name="im6", planet_radius=7.72e6, atmosphere_thickness=1.50e5, beta_r=glm.vec3(5.6e-6, 2.2e-5, 5.12e-5), beta_m=glm.vec3(3.1e-5)),
               #Planet(name="earth", planet_radius=6360e3, atmosphere_thickness=100e3, beta_r=glm.vec3(3.8e-6, 13.5e-6, 33.1e-6), beta_m=glm.vec3(21e-6)), 
               #Planet(name="venus", planet_radius=6052e3, atmosphere_thickness=200.059e3, beta_r=glm.vec3(11.37e-6, 11.37e-6, 1.8e-6), beta_m=glm.vec3(2.12153e-6)), 
               #Planet(name="mars", planet_radius=3389.5e3, atmosphere_thickness=30.588e3, beta_r=glm.vec3(23.918e-6, 13.57e-6, 5.78e-6), beta_m=glm.vec3(5.12153e-6))
               #]
    # max_planet_radius = 2.1e6 #8.1e6

    for scene in scenes:
        scene.img_width = int(scene.img_width / 5)
        scene.img_height = int(scene.img_height / 5)

    for planet in planets:
        atmo_thickness = planet.atmosphere_radius - planet.planet_radius
        atmosphere = AtmoNN(model, is_multi_planets=True, max_planet_radius=max_planet_radius, planet_radius=planet.planet_radius, atmopshere_thickness=atmo_thickness, BETA_R=planet.beta_r, BETA_M=planet.beta_m)

        for scene in scenes:
            sun_angle = glm.radians(scene.sun_angle)
            sun_dir   = glm.normalize(glm.vec3(0, glm.cos(sun_angle), -glm.sin(sun_angle)))
            atmosphere.light_dir = glm.normalize(sun_dir)

            if(scene.output_filename == "space_view"):
                scene.cam_origin = glm.vec3(0.0, planet.atmosphere_radius * 0.783436533, planet.atmosphere_radius * 1.958204334)

            camera = set_up_camera(scene, atmosphere.planet_radius, atmosphere.scaling_factor)
            render_skydome("res/img_planets/nn_lut_multi_" + planet.name + "_" + scene.output_filename + '.png', scene.img_width, scene.img_height, camera, atmosphere)

def render_results_img_based(scenes):
    print("Rendering using Atmo Img Based")
    
    # model_name = "saved_nn_models/image_based/5params/nn_img_based_synth_256_256_10l-84.h5"
    # model_name = "nn_img_based_real_256_256_10l_missing_no_02-277.h5"
    model_name = "nn_img_based_real_even-92.h5"
    # model_name = "nn_img_based_real_missing_11_15-52.h5"
    # model_name = "nn_img_based_tf_real_256_256_even.h5"
    # model_name = "nn_img_based_tf_real_256_256_missing_11_15.h5"

    # for scene in scenes:
    #     scene.img_width = int(scene.img_width / 4)
    #     scene.img_height = int(scene.img_height / 4)

    # 11_15
    # sun_zeniths  = [ 30.8862 ]
    # sun_azimuths = [ 124.483 ]

    # 6_00
    sun_zeniths  = [ 86.6242 ]
    # sun_azimuths = [ 63.8769 ]

    # sun_zeniths  = [ 46.2088, 40.846,  35.6912, 30.8862, 26.6542, 23.3369, 21.3844 ]
    # sun_azimuths = [ 101.094, 107.600, 115.223, 124.483, 136.066, 150.668, 168.365 ]

    for scene in scenes:
        scene.cam_yaw *= -1.0
        sun_angle = glm.radians(scene.sun_angle)
        sun_dir   = glm.normalize(glm.vec3(0, glm.cos(sun_angle), -glm.sin(sun_angle)))

        model = load_model(model_name, custom_objects={'rmse': rmse})
        atmosphere = AtmoImgBased(model, light_dir=sun_dir)

        for i in range(len(sun_zeniths)):
            atmosphere.m_zenith_angle  = sun_angle#glm.radians(sun_zeniths[i])# sun_angle
            atmosphere.m_azimuth_angle = glm.radians(np.pi / 2.0)#sun_azimuths[i])# np.pi / 2.0

            camera = set_up_camera(scene, atmosphere.planet_radius, atmosphere.scaling_factor)
            #render_skydome("res/img_based_" + scene.output_filename + '.png', scene.img_width, scene.img_height, camera, atmosphere, exposure=1.0) #use for synth img based
            render_skydome("res/new_photo_based/img_based_real_even_" + scene.output_filename + '_' + str(i) + '.png', scene.img_width, scene.img_height, camera, atmosphere, exposure=0.2, vertical_flip=True) #use for real img based

def render_results_trapezoidal_rule(scenes):
    print("Rendering using Atmo Classic")

    for scene in scenes:
        sun_angle = glm.radians(scene.sun_angle)
        sun_dir   = glm.normalize(glm.vec3(0, glm.cos(sun_angle), -glm.sin(sun_angle)))

        atmosphere = Atmosphere(light_dir=sun_dir)

        camera = set_up_camera(scene, atmosphere.planet_radius, atmosphere.scaling_factor)
        render_skydome("trapezoidal_" + scene.output_filename + '.png', scene.img_width, scene.img_height, camera, atmosphere)

#####################################################
##                   MAIN APP                      ##
#####################################################

os.environ['CUDA_VISIBLE_DEVICES'] = '-1'

scenes = [Scene(img_width=512, img_height=512, output_filename="fisheye_dawn", is_fisheye=True, cam_origin=glm.vec3(0.0), cam_pitch=0.0, cam_yaw=90.0, sun_angle=45.0),
          Scene(img_width=512, img_height=512, output_filename="fisheye_dusk", is_fisheye=True, cam_origin=glm.vec3(0.0), cam_pitch=0.0, cam_yaw=90.0, sun_angle=85.0),
          Scene(img_width=640, img_height=480, output_filename="space_view",   is_fisheye=False, cam_origin=glm.vec3(0.0, 5061e3, 12650e3), cam_pitch=0.0, cam_yaw=0.0, sun_angle=0.0),
          Scene(img_width=640, img_height=480, output_filename="surface_view_dawn", is_fisheye=False, cam_origin=glm.vec3(0.0), cam_pitch=25.0, cam_yaw=0.0, sun_angle=45.0),
          Scene(img_width=640, img_height=480, output_filename="surface_view_dusk", is_fisheye=False, cam_origin=glm.vec3(0.0), cam_pitch=25.0, cam_yaw=0.0, sun_angle=85.0)]

# scenes = [Scene(img_width=256, img_height=256, output_filename="", is_fisheye=True, cam_origin=glm.vec3(0.0), cam_pitch=0.0, cam_yaw=90.0, sun_angle=45.0)]

# scenes = [Scene(img_width=512, img_height=512, output_filename="fisheye_dawn", is_fisheye=True, cam_origin=glm.vec3(0.0), cam_pitch=0.0, cam_yaw=90.0, sun_angle=45.0)]

# render_results_lut_based_single(scenes)
render_results_lut_based_multi(scenes)
# render_results_img_based(scenes)
# render_results_trapezoidal_rule(scenes)
