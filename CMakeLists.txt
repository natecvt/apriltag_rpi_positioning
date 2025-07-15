cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(apriltag_tracking)

find_package(json-c REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)

set(APRILTAG_INCLUDE_DIR /usr/include/apriltag)
set(APRILTAG_COMMON_INCLUDE_DIR /usr/include/apriltag/common)
find_library(APRILTAG_LIBRARY apriltag PATHS /usr/local/lib /usr/lib REQUIRED)

include_directories(
    include
    ${GST_INCLUDE_DIRS}
    ${APRILTAG_INCLUDE_DIR}
    ${APRILTAG_COMMON_INCLUDE_DIR}
)

link_directories(${GST_LIBRARY_DIRS})

add_executable(tracker
    src/gstream_from_cam.c
    src/detect_apriltags.c
    src/settings.c
    src/transmit_pose.c
    src/main.c
)

target_link_libraries(tracker
    ${GST_LIBRARIES}
    ${APRILTAG_LIBRARY}
)