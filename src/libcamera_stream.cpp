#include <libcamera_stream.hpp>

int libcamera_request(
    std::shared_ptr<libcamera::Camera> camera,
    std::vector<std::unique_ptr<libcamera::Request>> *requests
) {
    static libcamera::ControlList ctrls = camera->controls();

    for (auto &request : *requests) {
        ctrls.set(libcamera::controls::AE_ENABLE, false);
        ctrls.set(libcamera::controls::AE_EXPOSURE_MODE, libcamera::controls::AeExposureModeEnum::ExposureShort);

        request->controls() = ctrls;

        if (camera->queueRequest(request.get()) < 0) {
            return 0;
        }
        return 1;
    }

    return 0;
}