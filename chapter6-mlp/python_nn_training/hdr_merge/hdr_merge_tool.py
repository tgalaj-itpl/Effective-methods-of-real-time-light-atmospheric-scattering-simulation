import os

import cv2
import numpy as np

# This script generates generates HDR images from the provided Kider's dataset images

# Kider's images dataset is availble under this link: 
# http://web.archive.org/web/20160617214024/http://www.graphics.cornell.edu/resources/clearsky/data/2013-05-27/HDR/
# unpack all archives to the separate folders and place them in the directory /hdr_merge/dataset/
# or adjust the paths in the script below

def readImagesAndTimes(hour):
    # List of exposure times
    times = np.array([1/8000.0, 1/1000.0, 1/250.0, 1/15.0, 1/32.0, 1/4.0, 1.0, 2.0], dtype=np.float32)

    # List of image filenames
    dir = os.getcwd() + "/hdr_merge/dataset/2013_05-27__" + hour + "/"
    filenames = [f for f in os.listdir(dir) if os.path.isfile(os.path.join(dir, f))]

    images = []
    for filename in filenames:
        im = cv2.imread(dir + filename)
        images.append(im)

    return images, times

## Main App ##

hours = ["09-15", "09-30", "09-45", "10-00", "10-15", 
         "10-30", "10-45", "11-00", "11-15", "11-30", 
         "11-45", "12-00", "12-15", "12-30", "12-45",
         "13-00"]

for hour in hours:
    print("Creating HDR for hour " + hour)
    current_dir = os.getcwd() + "/hdr_merge/dataset/"

    images, times = readImagesAndTimes(hour)

    # Obtain Camera Response Function (CRF)
    calibrateDebevec = cv2.createCalibrateDebevec()
    responseDebevec = calibrateDebevec.process(images, times)

    # Merge images into an HDR linear image
    mergeDebevec = cv2.createMergeDebevec()
    hdrDebevec = mergeDebevec.process(images, times, responseDebevec)

    # Save HDR image.
    hdr_filename = current_dir + "2013_05-27__" + hour + ".hdr"
    hdrDebevec = cv2.resize(hdrDebevec, (256, 256), interpolation=cv2.INTER_AREA)
    cv2.imwrite(hdr_filename, hdrDebevec)

    # Tonemap using Reinhard's method to obtain 24-bit color image
    tonemapReinhard = cv2.createTonemapReinhard(1.0, 1.25,0,0)
    ldrReinhard = tonemapReinhard.process(hdrDebevec)

    ldr_filename = current_dir + "2013_05-27__" + hour + "_kiders.jpg"
    cv2.imwrite(ldr_filename, ldrReinhard * 255)
