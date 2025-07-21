cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(rpicam_calibration)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin")

find_package(OpenCV REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONC REQUIRED json-c)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)

set(APRILTAG_INCLUDE_DIR /usr/include/apriltag)
set(APRILTAG_COMMON_INCLUDE_DIR /usr/include/apriltag/common)
find_library(APRILTAG_LIBRARY apriltag PATHS /usr/local/lib /usr/lib REQUIRED)

include_directories(
    ../include
    ${OpenCV_INCLUDE_DIRS}
    ${JSONC_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
    ${APRILTAG_INCLUDE_DIR}
    ${APRILTAG_COMMON_INCLUDE_DIR}
)

link_directories(${GST_LIBRARY_DIRS})

add_executable(calibrate
    ../src/settings.c
    ../src/gstream_from_cam.c
    calibrate.cpp
)

target_link_libraries(calibrate
    ${OpenCV_LIBS}
    ${JSONC_LIBRARIES}
    ${GST_LIBRARIES}
    ${APRILTAG_LIBRARY}
)