#include <opencv2/opencv.hpp>
#include <iostream>

// for simple testing:
// int main() {
//     cv::Mat test = cv::Mat::zeros(480, 640, CV_8UC3);
//     cv::putText(test, "Hello OpenCV", {100, 240}, cv::FONT_HERSHEY_SIMPLEX, 1.5, {255, 255, 255}, 2);

//     cv::imshow("Test", test);
//     cv::waitKey(0);  // Wait indefinitely
//     return 0;
// }

int main() {
    // for debugging, comment out
    setenv("GST_DEBUG", "3", 1);

    // the gstreamer pipeline, controls camera settings through libcamerasrc
    std::string pipeline = 
    "libcamerasrc ! "
    "queue ! "
    "video/x-raw,width=640,height=480,format=NV12,framerate=10/1 ! "
    "videoconvert ! "
    "video/x-raw ! "
    "appsink";

    // video capture structure, checks for failure to open
    cv::VideoCapture cap;
    cap.open(pipeline, cv::CAP_GSTREAMER);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera stream" << std::endl;
        return -1;
    }

    // reading frame data and processing to image
    cv::Mat frame;
    int frame_count = 0;
    while (frame_count < 20) {
        cap >> frame;

        if (frame.empty()) {
            std::cerr << "Empty frame" << std::endl;
            break;
        }

        std::string filename = "output/frame_" + std::to_string(frame_count) + ".jpg";
        cv::imwrite(filename, frame);
        std::cout << "Saved " << filename << std::endl;
        frame_count++;

        if (cv::waitKey(1) == 27) break;  // ESC to exit
    }

    return 0;
}