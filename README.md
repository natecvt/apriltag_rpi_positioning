# aprlitag_rpi_positioning
Small library for use finding exact position using apriltags in a vertical setup. For use on raspberry pi, with the picam.

#### Dependencies
`libgstreamer1.0-dev` `libgstreamer-plugins-base1.0-dev` `libapriltag-dev` `gstreamer1.0-libcamerasrc` `gstreamer1.0-tools` `gstreamer1.0-libav`

Sometimes libglib2.0-dev won't install properly, reinstall it and the glibconfig.h file should appear

(-): not started, (i): in progress, (o): tentatively complete, Nothing: complete

There will be 5 parts to this program:
- (-) Interpreting a config file to set image size, framerate, tag family, other constants needed by the program 
- (i) Setting up the gstreamer video capabilities and providing functionality to the creation of images from it. This will also need to convert the data from images to the necessary format used by the apriltags library
- (i) Running apriltag detection on all image data, managing timing to help maintain efficiency, refining pose estimate if multiple tags are detected
- (-) Connecting to the beaglebone via I2C or SPI through the GPIO pins and sending pose data, including: x, y, z, r, p, y, time, status, ndetected
- (-) A main program that manages all of the above

Functions will return error codes starting from zero for debugging

current compile command:
gcc -o bin/detect `pkg-config --cflags --libs apriltag gstreamer-1.0 gstreamer-app-1.0` -Iinclude -I/usr/include/apriltag/common -c src/detect_apriltags.c