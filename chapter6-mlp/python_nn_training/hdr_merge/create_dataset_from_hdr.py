import math
import os

import imageio
import numpy as np

import glm

# This script generates dataset (using Kider's images) 
# that is used for training img based neural network in model_training.py

# NOTE: be sure to first generate HDR images from the provided dataset images
#       using hdr_merge_tool.py script

# Kider's images dataset is availble under this link: 
# http://web.archive.org/web/20160617214024/http://www.graphics.cornell.edu/resources/clearsky/data/2013-05-27/HDR/

class Time:
    second = 0
    minute = 0
    hour   = 0
    day    = 27
    month  = 5
    year   = 2013

def get_primary_ray(x, y, width, height, origin, y_axis_angle):
    yaw   = 90 + y_axis_angle
    pitch = 0
    
    yaw   = glm.radians(-90.0 - yaw)
    pitch = glm.radians(-pitch)

    cam_dir   = glm.vec3(0)
    cam_dir.x = glm.cos(yaw) * glm.cos(pitch)
    cam_dir.y = glm.sin(pitch)
    cam_dir.z = glm.sin(yaw) * glm.cos(pitch)
    cam_dir   = glm.normalize(cam_dir)

    cam_transform = glm.lookAt(origin, origin + cam_dir, glm.vec3(0.0, 1.0, 0.0))

    ray_direction = glm.vec3(0.0)

    fisheye_cam_x = 2.0 * x / float(width - 1) - 1.0
    fisheye_cam_y = 2.0 * y / float(height - 1) - 1.0
    z2 = fisheye_cam_x * fisheye_cam_x + fisheye_cam_y * fisheye_cam_y

    is_good = False

    if z2 <= 1.0:
        phi   = math.atan2(fisheye_cam_y, fisheye_cam_x)
        theta = math.acos(1.0 - z2)

        ray_direction = glm.vec3(glm.sin(theta) * glm.cos(phi), glm.cos(theta), glm.sin(theta) * glm.sin(phi))
        ray_direction = glm.vec3(cam_transform * glm.vec4(ray_direction, 0.0))
        ray_direction = glm.normalize(ray_direction)
        is_good = True

    return ray_direction, is_good

## Main App ##

time = Time()

hours = ["09-30", "09-45", "10-00", "10-15", "10-30", 
         "10-45", "11-00", "11-15", "11-30", "11-45", 
         "12-00", "12-15", "12-30", "12-45", "13-00"]

sun_zeniths  = [ 48.9379, 46.2088, 43.5081, 40.846, 38.2352, 35.6912, 33.2334, 30.8862, 28.6804, 26.6542, 24.8547, 23.3369, 22.1608, 21.3844, 21.053 ]
sun_azimuths = [ 98.1425, 101.094, 104.233, 107.600, 111.244, 115.223, 119.609, 124.483, 129.936, 136.066, 142.959, 150.668, 159.179, 168.365, 177.978 ]

current_dir = os.getcwd() + "/hdr_merge/"
file = open(current_dir + 'dataset_py_hdr_missing_11_15_test.txt', 'w')

earth_radius = 6360e3
scaling_factor = 1.0 / earth_radius

for i in range(0, len(hours), 1):
    if i != 7: continue; # 11_15
    
    hdr_data = imageio.imread(current_dir + "dataset/2013_05-27__" + hours[i] + ".hdr", format='HDR-FI')

    IMG_WIDTH  = hdr_data.shape[0]
    IMG_HEIGHT = hdr_data.shape[1]

    origin = glm.vec3(0.0, (earth_radius + 1000.0) * scaling_factor, 0.0)

    minutes     = 9 * 60 + 30 + i * 15
    time.minute = minutes % 60
    time.hour   = int(minutes / 60.0)

    zenith        = glm.radians(sun_zeniths[i])
    azimuth       = glm.radians(sun_azimuths[i])

    #print("time = " + str(time.hour) + ":" + str(time.minute) + ", zenith = " + str(glm.degrees(zenith)) + ", azimuth = " + str(glm.degrees(azimuth)))
    print('Collecting data for ' + hours[i] + ' || (zenith, azimuth): ' + str(glm.degrees(zenith)) + ", " + str(glm.degrees(azimuth)))
    for y in range(IMG_HEIGHT):
        for x in range(IMG_WIDTH):
            direction, is_valid = get_primary_ray((IMG_WIDTH - 1) - x + 0.5, y + 0.5, IMG_WIDTH, IMG_HEIGHT, origin, 0.0)

            if is_valid:
                color     = glm.vec3(hdr_data[y][x][0], hdr_data[y][x][1], hdr_data[y][x][2])
                color     = color / (color + 10.0) # simple tone mapping

                file.write(str(zenith/np.pi) + " " + str(azimuth/np.pi) + " " + str(direction.x) + " " + str(direction.y) + " " + str(direction.z) + " " + str(color[0]) + " " + str(color[1]) + " " + str(color[2]) + "\n")

print('End')
file.close()
