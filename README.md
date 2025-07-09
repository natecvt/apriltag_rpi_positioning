# aprlitag_rpi_positioning
Small library for use finding exact position using apriltags in a vertical setup. For use on raspberry pi, with the picam.

#### Dependencies
`libgstreamer1.0-dev` `libgstreamer-plugins-base1.0-dev`
/
`gstreamer1.0-libcamerasrc` `gstreamer1.0-tools` `gstreamer1.0-libav`
`libopencv-dev`

Sometimes libglib2.0-dev won't install properly, reinstall it and the glibconfig.h file should appear

There will be 5 parts to this program:
- Interpreting a config file to set image size, framerate, tag family, other constants needed by the program
- Setting up the gstreamer video capabilities and providing functionality to the creation of images from it. This will also need to convert the data from images to the necessary format used by the apriltags library
- Running apriltag detection on all image data, managing timing to help maintain efficiency, refining pose estimate if multiple tags are detected
- Connecting to the beaglebone via I2C or SPI through the GPIO pins and sending pose data, including: x, y, z, r, p, y, time, status, ndetected
- A main program that manages all of the above

Functions will return error codes starting from zero for debugging