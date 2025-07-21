#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdio>

#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/aruco/dictionary.hpp>

#include <gstream_from_cam.h>

using namespace cv;
using namespace std;

class SettingsCV {
    private:
    public:
        Size boardSize;              // The size of the board -> Number of items by width and height
        float squareSize;            // The size of a square in your defined unit (point, millimeter,etc).
        float markerSize;            // The size of a marker in your defined unit (point, millimeter,etc).
        int nrFrames;                // The number of frames to use from the input for calibration
        float aspectRatio;           // The aspect ratio
        bool calibZeroTangentDist;   // Assume zero tangential distortion
        bool calibFixPrincipalPoint; // Fix the principal point at the center
        bool flipVertical;           // Flip the captured images around the horizontal axis
        string outputFileName;       // The name of the file where to write
        bool showUndistorted;        // Show undistorted images after calibration
        string input;                // The input ->
        bool useFisheye;             // use fisheye camera model for calibration
        bool fixK1;                  // fix K1 distortion coefficient
        bool fixK2;                  // fix K2 distortion coefficient
        bool fixK3;                  // fix K3 distortion coefficient
        bool fixK4;                  // fix K4 distortion coefficient
        bool fixK5;                  // fix K5 distortion coefficient

        vector<string> imageList;
        size_t atImageList;
        bool goodInput;
        int flag;
};

int main(int argc, char* argv[]) {
    const String keys
        = "{help h usage ? |           | print this message            }"
          "{@SettingsCV      |default.xml| input setting file            }"
          "{d              |           | actual distance between top-left and top-right corners of "
          "the calibration grid }"
          "{winSize        | 11        | Half of search window for cornerSubPix }";
    CommandLineParser parser(argc, argv, keys);
    parser.about("This is a camera calibration sample.\n"
                 "Usage: camera_calibration [configuration_file -- default ./default.xml]\n"
                 "Near the sample file you'll find the configuration file, which has detailed help of "
                 "how to edit it. It may be any OpenCV supported file format XML/YAML.");
    if (!parser.check()) {
        parser.printErrors();
        return 0;
    }

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    //! [file_read]
    const string inputSettingsCVFile = parser.get<string>(0);

    FileStorage fs(inputSettingsCVFile, FileStorage::READ); // Read the SettingsCV
    if (!fs.isOpened())
    {
        cout << "Could not open the configuration file: \"" << inputSettingsCVFile << "\"" << endl;
        parser.printMessage();
        return -1;
    }

    Settings settings;
    StreamSet streams;
    int ec;
    GstBus *bus;

    ec = load_settings_from_path("/home/natec/apriltag_rpi_positioning/settings/settings.json", &settings);
    if(ec) {
        printf("Settings failed to load with error code: %d\n", ec);
        exit(1);
    }
    settings.np = settings.width * settings.height;

    ec = gstream_setup(&streams, &settings, FALSE, FALSE);
    if (ec) {
        g_printerr("Gstream setup returned error code: %d\n", ec);
        exit(1);
    }

    bus = gst_element_get_bus(streams.pipeline);

    while (true) {
        
    }
}