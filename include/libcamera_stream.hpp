#ifndef LIBCAMERA_STREAM_H
#define LIBCAMERA_STREAM_H

#include <libcamera/libcamera/camera.h>
#include <libcamera/libcamera/libcamera.h>
#include <libcamera/libcamera/controls.h>

#include <settings.h>

#include <string>

int libcamera_request(
    std::shared_ptr<libcamera::Camera> camera,
    std::vector<std::unique_ptr<libcamera::Request>> *requests
);

#endif