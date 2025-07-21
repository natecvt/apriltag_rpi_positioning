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

extern "C" {
    #include <gstream_from_cam.h>
}

using namespace cv;
using namespace std;

class SettingsCV {
    private:
    public:
        enum Pattern { NOT_EXISTING, CHESSBOARD };
        enum InputType { INVALID, CAMERA, IMAGE_LIST };
        
        void read(const FileNode& node)                          //Read serialization for this class
        {
            node["BoardSize_Width"] >> boardSize.width;
            node["BoardSize_Height"] >> boardSize.height;
            node["Calibrate_Pattern"] >> patternToUse;
            node["Square_Size"] >> squareSize;
            node["Marker_Size"] >> markerSize;
            node["Calibrate_NrOfFrameToUse"] >> nrFrames;
            node["Calibrate_FixAspectRatio"] >> aspectRatio;
            node["Write_outputFileName"] >> outputFileName;
            node["Calibrate_AssumeZeroTangentialDistortion"] >> calibZeroTangentDist;
            node["Calibrate_FixPrincipalPointAtTheCenter"] >> calibFixPrincipalPoint;
            node["Calibrate_UseFisheyeModel"] >> useFisheye;
            node["Input_FlipAroundHorizontalAxis"] >> flipVertical;
            node["Show_UndistortedImage"] >> showUndistorted;
            node["Input"] >> input;
            node["Fix_K1"] >> fixK1;
            node["Fix_K2"] >> fixK2;
            node["Fix_K3"] >> fixK3;
            node["Fix_K4"] >> fixK4;
            node["Fix_K5"] >> fixK5;

            validate();
        }

        void validate()
        {
            goodInput = true;
            if (boardSize.width <= 0 || boardSize.height <= 0)
            {
                cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
                goodInput = false;
            }
            if (squareSize <= 10e-6)
            {
                cerr << "Invalid square size " << squareSize << endl;
                goodInput = false;
            }
            if (nrFrames <= 0)
            {
                cerr << "Invalid number of frames " << nrFrames << endl;
                goodInput = false;
            }

            if (input.empty())      // Check for valid input
                    inputType = INVALID;
            else
            {
                if (input.find("settings") != string::npos)
                {
                    inputType = CAMERA;
                }
                else
                {
                    if (isListOfImages(input) && readStringList(input, imageList))
                    {
                        inputType = IMAGE_LIST;
                        nrFrames = (nrFrames < (int)imageList.size()) ? nrFrames : (int)imageList.size();
                    }
                    else {
                        inputType = INVALID;
                    }
                }

                if (inputType = CAMERA) {
                    int ec = load_settings_from_path(input.c_str(), settings);
                    if(ec) {
                        printf("Settings failed to load with error code: %d\n", ec);
                        exit(1);
                    }
                    (*settings).np = settings->width * settings->height;

                    ec = gstream_setup(streams, settings, FALSE, FALSE);
                    if (ec) {
                        g_printerr("Gstream setup returned error code: %d\n", ec);
                        exit(1);
                    }

                    bus = gst_element_get_bus(streams->pipeline);
                }
            }
            if (inputType == INVALID)
            {
                cerr << " Input does not exist: " << input;
                goodInput = false;
            }

            flag = 0;
            if(calibFixPrincipalPoint) flag |= CALIB_FIX_PRINCIPAL_POINT;
            if(calibZeroTangentDist)   flag |= CALIB_ZERO_TANGENT_DIST;
            if(aspectRatio)            flag |= CALIB_FIX_ASPECT_RATIO;
            if(fixK1)                  flag |= CALIB_FIX_K1;
            if(fixK2)                  flag |= CALIB_FIX_K2;
            if(fixK3)                  flag |= CALIB_FIX_K3;
            if(fixK4)                  flag |= CALIB_FIX_K4;
            if(fixK5)                  flag |= CALIB_FIX_K5;

            if (useFisheye) {
                // the fisheye model has its own enum, so overwrite the flags
                flag = fisheye::CALIB_FIX_SKEW | fisheye::CALIB_RECOMPUTE_EXTRINSIC;
                if(fixK1)                   flag |= fisheye::CALIB_FIX_K1;
                if(fixK2)                   flag |= fisheye::CALIB_FIX_K2;
                if(fixK3)                   flag |= fisheye::CALIB_FIX_K3;
                if(fixK4)                   flag |= fisheye::CALIB_FIX_K4;
                if (calibFixPrincipalPoint) flag |= fisheye::CALIB_FIX_PRINCIPAL_POINT;
            }

            calibrationPattern = NOT_EXISTING;
            if (!patternToUse.compare("CHESSBOARD")) calibrationPattern = CHESSBOARD;
            if (calibrationPattern == NOT_EXISTING)
            {
                cerr << " Camera calibration mode does not exist: " << patternToUse << endl;
                goodInput = false;
            }
            atImageList = 0;

        }

        static bool readStringList( const string& filename, vector<string>& l )
        {
            l.clear();
            FileStorage fs(filename, FileStorage::READ);
            if( !fs.isOpened() )
                return false;
            FileNode n = fs.getFirstTopLevelNode();
            if( n.type() != FileNode::SEQ )
                return false;
            FileNodeIterator it = n.begin(), it_end = n.end();
            for( ; it != it_end; ++it )
                l.push_back((string)*it);
            return true;
        }

        static bool isListOfImages( const string& filename)
        {
            string s(filename);
            // Look for file extension
            if( s.find(".xml") == string::npos && s.find(".yaml") == string::npos && s.find(".yml") == string::npos )
                return false;
            else
                return true;
        }

        Size boardSize;              // The size of the board -> Number of items by width and height
        Pattern calibrationPattern;  // One of the Chessboard, ChArUco board, circles, or asymmetric circle pattern
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
        InputType inputType;
        string patternToUse;

        Settings *settings;
        StreamSet *streams;
        GstBus *bus;
};

static inline void read(const FileNode& node, SettingsCV& x, const SettingsCV& default_value = SettingsCV())
{
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };

//! [compute_errors]
static double computeReprojectionErrors( const vector<vector<Point3f> >& objectPoints,
                                         const vector<vector<Point2f> >& imagePoints,
                                         const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                         const Mat& cameraMatrix , const Mat& distCoeffs,
                                         vector<float>& perViewErrors, bool fisheye)
{
    vector<Point2f> imagePoints2;
    size_t totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for(size_t i = 0; i < objectPoints.size(); ++i )
    {
        if (fisheye)
        {
            fisheye::projectPoints(objectPoints[i], imagePoints2, rvecs[i], tvecs[i], cameraMatrix,
                                   distCoeffs);
        }
        else
        {
            projectPoints(objectPoints[i], rvecs[i], tvecs[i], cameraMatrix, distCoeffs, imagePoints2);
        }
        err = norm(imagePoints[i], imagePoints2, NORM_L2);

        size_t n = objectPoints[i].size();
        perViewErrors[i] = (float) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }

    return std::sqrt(totalErr/totalPoints);
}

//! [board_corners]
static bool runCalibration( SettingsCV& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                            vector<vector<Point2f> > imagePoints, vector<Mat>& rvecs, vector<Mat>& tvecs,
                            vector<float>& reprojErrs,  double& totalAvgErr, vector<Point3f>& newObjPoints,
                            float grid_width, bool release_object)
{
    //! [fixed_aspect]
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if( !s.useFisheye && s.flag & CALIB_FIX_ASPECT_RATIO )
        cameraMatrix.at<double>(0,0) = s.aspectRatio;
    //! [fixed_aspect]
    if (s.useFisheye) {
        distCoeffs = Mat::zeros(4, 1, CV_64F);
    } else {
        distCoeffs = Mat::zeros(8, 1, CV_64F);
    }

    vector<vector<Point3f> > objectPoints(1);
    objectPoints[0].clear();
    objectPoints[0][s.boardSize.width - 1].x = objectPoints[0][0].x + grid_width;

    newObjPoints = objectPoints[0];

    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    //Find intrinsic and extrinsic camera parameters
    double rms;

    if (s.useFisheye) {
        Mat _rvecs, _tvecs;
        rms = fisheye::calibrate(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, _rvecs,
                                 _tvecs, s.flag);

        rvecs.reserve(_rvecs.rows);
        tvecs.reserve(_tvecs.rows);
        for(int i = 0; i < int(objectPoints.size()); i++){
            rvecs.push_back(_rvecs.row(i));
            tvecs.push_back(_tvecs.row(i));
        }
    } else {
        int iFixedPoint = -1;
        if (release_object)
            iFixedPoint = s.boardSize.width - 1;
        rms = calibrateCameraRO(objectPoints, imagePoints, imageSize, iFixedPoint,
                                cameraMatrix, distCoeffs, rvecs, tvecs, newObjPoints,
                                s.flag | CALIB_USE_LU);
    }

    if (release_object) {
        cout << "New board corners: " << endl;
        cout << newObjPoints[0] << endl;
        cout << newObjPoints[s.boardSize.width - 1] << endl;
        cout << newObjPoints[s.boardSize.width * (s.boardSize.height - 1)] << endl;
        cout << newObjPoints.back() << endl;
    }

    cout << "Re-projection error reported by calibrateCamera: "<< rms << endl;

    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    objectPoints.clear();
    objectPoints.resize(imagePoints.size(), newObjPoints);
    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints, rvecs, tvecs, cameraMatrix,
                                            distCoeffs, reprojErrs, s.useFisheye);

    return ok;
}

void writeToCal(Mat cameraMatrix, string outputFileName) {
    // #TODO: write to file parameters
    string path = "$HOME/apriltag_rpi_positioning/calibration/";
    path.append(outputFileName);
    FILE* f = fopen(path.c_str(), "w");
    if (f == NULL) {
        printf("Calibration output file failed to open");
    }

    fprintf(f, "%5.5f ", cameraMatrix.data[0]);
    fprintf(f, "%5.5f ", cameraMatrix.data[4]);
    fprintf(f, "%5.5f ", cameraMatrix.data[2]);
    fprintf(f, "%5.5f ", cameraMatrix.data[5]);
}

//! [run_and_save]
bool runCalibrationAndSave(SettingsCV& s, Size imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                           vector<vector<Point2f> > imagePoints, float grid_width, bool release_object)
{
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;
    double totalAvgErr = 0;
    vector<Point3f> newObjPoints;

    bool ok = runCalibration(s, imageSize, cameraMatrix, distCoeffs, imagePoints, rvecs, tvecs, reprojErrs,
                             totalAvgErr, newObjPoints, grid_width, release_object);
    cout << (ok ? "Calibration succeeded" : "Calibration failed")
         << ". avg re projection error = " << totalAvgErr << endl;

    if (ok)
        writeToCal(cameraMatrix, s.outputFileName);
    return ok;
}

cv::Mat gst_sample_to_mat(GstSample* sample) {
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstCaps* caps = gst_sample_get_caps(sample);
    GstStructure* s = gst_caps_get_structure(caps, 0);

    int width, height;
    gst_structure_get_int(s, "width", &width);
    gst_structure_get_int(s, "height", &height);

    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        return cv::Mat();  // Return empty on failure
    }

    // Assuming format=GRAY8, one byte per pixel
    cv::Mat mat(height, width, CV_8UC1, (void*)map.data);

    // Copy to safely use after buffer is unmapped
    cv::Mat output = mat.clone();

    gst_buffer_unmap(buffer, &map);
    return output;
}

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

    SettingsCV s;
    const string inputSettingsCVFile = parser.get<string>(0);

    FileStorage fs(inputSettingsCVFile, FileStorage::READ); // Read the SettingsCV
    if (!fs.isOpened())
    {
        cout << "Could not open the configuration file: \"" << inputSettingsCVFile << "\"" << endl;
        parser.printMessage();
        return -1;
    }

    fs["SettingsCV"] >> s;
    fs.release();                                           // close SettingsCV file

    if (!s.goodInput)
    {
        cout << "Invalid input detected. Application stopping. " << endl;
        return -1;
    }

    int winSize = parser.get<int>("winSize");

    float grid_width = s.squareSize * (s.boardSize.width - 1);

    bool release_object = false;
    if (parser.has("d")) {
        grid_width = parser.get<float>("d");
        release_object = true;
    }

    // allocating data 
    uint8_t *data = (uint8_t *)malloc(s.settings->np * s.settings->stride);
    if (data == NULL) {
        perror("Image data allocation failed\n");
        exit(2);
    }

    printf("Press <Enter> once:");
    int key_ent = getchar();

    vector<vector<Point2f> > imagePoints;
    Mat cameraMatrix, distCoeffs;
    Size imageSize;
    int mode = s.inputType == SettingsCV::IMAGE_LIST ? CAPTURING : DETECTION;
    const Scalar RED(0,0,255), GREEN(0,255,0);

    while (1) {
        GstSample *sample;
        bool blinkOutput = false;

        printf("Press <Enter> to capture image:");
        if (getchar() == key_ent) {
            // pull the sample
            sample = gst_app_sink_try_pull_sample(GST_APP_SINK(s.streams->sink), GST_SECOND / s.settings->framerate);
            if (!sample) continue;
        }

        Mat view = gst_sample_to_mat(sample);

        //-----  If no more image, or got enough, then stop calibration and show result -------------
        if( mode == CAPTURING && imagePoints.size() >= (size_t)s.nrFrames )
        {
          if(runCalibrationAndSave(s, imageSize,  cameraMatrix, distCoeffs, imagePoints, grid_width,
                                   release_object))
              mode = CALIBRATED;
          else
              mode = DETECTION;
        }
        if(view.empty())          // If there are no more images stop the loop
        {
            // if calibration threshold was not reached yet, calibrate now
            if( mode != CALIBRATED && !imagePoints.empty() )
                runCalibrationAndSave(s, imageSize,  cameraMatrix, distCoeffs, imagePoints, grid_width,
                                      release_object);
            break;
        }
        //! [get_input]

        imageSize = view.size();  // Format input image.
        if( s.flipVertical )    flip( view, view, 0 );

        //! [find_pattern]
        vector<Point2f> pointBuf;

        bool found;

        int chessBoardFlags = CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE;

        if(!s.useFisheye) {
            // fast check erroneously fails with high distortions like fisheye
            chessBoardFlags |= CALIB_CB_FAST_CHECK;
        }

        switch( s.calibrationPattern ) // Find feature points on the input format
        {
        case SettingsCV::CHESSBOARD:
            found = findChessboardCorners( view, s.boardSize, pointBuf, chessBoardFlags);
            break;
        default:
            found = false;
            break;
        }
        //! [find_pattern]

        //! [pattern_found]
        if (found)                // If done with success,
        {
            // improve the found corners' coordinate accuracy for chessboard
            if( s.calibrationPattern == SettingsCV::CHESSBOARD)
            {
                Mat viewGray;
                cvtColor(view, viewGray, COLOR_BGR2GRAY);
                cornerSubPix( viewGray, pointBuf, Size(winSize,winSize),
                    Size(-1,-1), TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 30, 0.0001 ));
            }

            drawChessboardCorners( view, s.boardSize, Mat(pointBuf), found );
        }
        //! [pattern_found]
        //----------------------------- Output Text ------------------------------------------------
        //! [output_text]
        string msg = (mode == CAPTURING) ? "100/100" :
                      mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
        int baseLine = 0;
        Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
        Point textOrigin(view.cols - 2*textSize.width - 10, view.rows - 2*baseLine - 10);

        if( mode == CAPTURING )
        {
            if(s.showUndistorted)
                msg = cv::format( "%d/%d Undist", (int)imagePoints.size(), s.nrFrames );
            else
                msg = cv::format( "%d/%d", (int)imagePoints.size(), s.nrFrames );
        }

        putText( view, msg, textOrigin, 1, 1, mode == CALIBRATED ?  GREEN : RED);

        if( blinkOutput )
            bitwise_not(view, view);
        //! [output_text]
        //------------------------- Video capture  output  undistorted ------------------------------
        //! [output_undistorted]
        if( mode == CALIBRATED && s.showUndistorted )
        {
            Mat temp = view.clone();
            if (s.useFisheye)
            {
                Mat newCamMat;
                fisheye::estimateNewCameraMatrixForUndistortRectify(cameraMatrix, distCoeffs, imageSize,
                                                                    Matx33d::eye(), newCamMat, 1);
                cv::fisheye::undistortImage(temp, view, cameraMatrix, distCoeffs, newCamMat);
            }
            else
              undistort(temp, view, cameraMatrix, distCoeffs);
        }
        //! [output_undistorted]
    }

    gstream_cleanup(s.bus, s.streams);
}