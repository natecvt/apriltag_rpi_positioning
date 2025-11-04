# README

Small library for use finding exact position using apriltags in a vertical setup. For use on raspberry pi, with the picam.

## Dependencies

`sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev gstreamer1.0-libcamera gstreamer1.0-tools gstreamer1.0-libav libjson-c-dev`

Apriltag debian package does not install the `.so` with all function definitions, so run
`git clone https://github.com/AprilRobotics/apriltag/ && cd apriltag && sudo cmake -B build -DCMAKE_BUILD_TYPE=Release && sudo cmake --build build --target install`
, then delete the apriltag folder.

Sometimes libglib2.0-dev won't be installed properly, reinstall it and the glibconfig.h file should appear, or run
`sudo cp /usr/lib/<linux-name>/glib-2.0/include/glibconfig.h /usr/include/glib-2.0/glibconfig.h`

(-): not started, (i): in progress, (o): tentatively complete, Nothing: complete

There will be 6 parts to this program:

- (o) Interpreting a config file to set image size, framerate, tag family, other constants needed by the program 
- (o) Setting up the gstreamer video capabilities and providing functionality to the creation of images from it. This will also need to convert the data from images to the necessary format used by the apriltags library
- (i) Running apriltag detection on all image data, managing timing to help maintain efficiency, refining pose estimate if multiple tags are detected
- (i) Connecting to the beaglebone via I2C or SPI through the GPIO pins and sending pose data, including: x, y, z, qx, qy, qz, qw, time, status, ndetected, uncertainty?
- (i) A main program that manages all of the above
- (i) Calibration script that writes intrinsic matrix values to a small file
- (i) Logging functionality for testing

## Calibration

1. Change the n_cal_imgs setting, then run ./bin/calibrate. Make sure the resolution is the same as desired when running the main program.
2. Cycle through the dialog and press enter when the calibration checkerboard is in frame. Take as many images as needed, change orientation of the board or camera to get more versatile data.
3. Download the .pnm images and run the cameraCalibrator script on matlab, giving the square size in mm. Other programs to accomplish this also exist.
4. Press calibrate and save the results to a variable, the 'K' attribute will contain the full intrinsic matrix.
5. Copy the nonzero/nonunitary values in column-first order.

Functions will return error codes starting from zero for debugging

current compile command:
`mkdir build && cd build && cmake .. && make`