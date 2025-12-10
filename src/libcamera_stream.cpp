#include <libcamera_stream.hpp>

int libcamera_request(
    std::shared_ptr<libcamera::Camera> camera,
    std::vector<std::unique_ptr<libcamera::Request>> *requests
) {
    static libcamera::ControlList ctrls = camera->controls();

    for (auto &request : *requests) {
        ctrls.set(libcamera::controls::AeEnable, false);
        ctrls.set(libcamera::controls::ExposureTime, 50000);
        ctrls.set(libcamera::controls::AnalogueGain, 2.0);

        request->controls() = ctrls;

        if (camera->queueRequest(request.get()) < 0) {
            return 0;
        }
        return 1;
    }

    return 0;
}