# aprlitag_rpi_positioning
Small library for use finding exact position using apriltags in a vertical setup. For use on raspberry pi, with the picam.

#### Dependencies
`sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev gstreamer1.0-libcamerasrc gstreamer1.0-tools gstreamer1.0-libav`

Apriltag debian package does not install the `.so` with all function definitions, so run
`git clone https://github.com/AprilRobotics/apriltag/ && cd apriltag && cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --target install`

Sometimes libglib2.0-dev won't be installed properly, reinstall it and the glibconfig.h file should appear, or run
`sudo cp /usr/lib/aarch64-linux-gnu/glib-2.0/include/glibconfig.h /usr/include/glib-2.0/glibconfig.h`

(-): not started, (i): in progress, (o): tentatively complete, Nothing: complete

There will be 5 parts to this program:
- (i) Interpreting a config file to set image size, framerate, tag family, other constants needed by the program 
- (i) Setting up the gstreamer video capabilities and providing functionality to the creation of images from it. This will also need to convert the data from images to the necessary format used by the apriltags library
- (i) Running apriltag detection on all image data, managing timing to help maintain efficiency, refining pose estimate if multiple tags are detected
- (-) Connecting to the beaglebone via I2C or SPI through the GPIO pins and sending pose data, including: x, y, z, r, p, y, time, status, ndetected
- (i) A main program that manages all of the above

Functions will return error codes starting from zero for debugging

current compile command:
`mkdir build && cd build && cmake .. && make`