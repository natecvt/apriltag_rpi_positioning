#include <settings.h>
#include <detect_apriltags.h>
#include <transmit_pose.h>
#include <logger.h>

extern "C" {
    #include <memory.h>
    #include <stdlib.h>
    #include <signal.h>
    #include <sys/mman.h>
}

#include <libcamera_stream.hpp>

using namespace libcamera;

volatile sig_atomic_t stop = 0;
uint8_t new_data = 0; // flag for new image data available
uint8_t *data; // image data
static std::shared_ptr<Camera> camera;

void handle_sigint(int sig) {
    stop = 1;
}

static void request_complete(Request *request) {
    if (request->status() == Request::RequestCancelled) return;

    const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();

    if (buffers.empty()) return;

    // get the buffer for the Y plane
    FrameBuffer *fb = buffers.begin()->second;
    const FrameBuffer::Plane &yplane = fb->planes()[0]; // the Y-plane for YUV420

    // checking buffer
    if (yplane.fd.get() < 0 || yplane.length == 0) {
        printf("Image buffer invalid (fd=%d, length=%zu)\n", yplane.fd.get(), (size_t)yplane.length);
        return;
    }

    // mmap requires the offset to be aligned to the system page size. Compute
    // a page-aligned mapping and adjust the pointer to the plane start.
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) page_size = 4096;

    off_t map_offset = yplane.offset & ~(page_size - 1);
    size_t map_diff = yplane.offset - map_offset;
    size_t map_length = yplane.length + map_diff;

    // creating a uspace pointer to access image data
    void *ptr = mmap(NULL, map_length, PROT_READ | PROT_WRITE, MAP_SHARED, yplane.fd.get(), map_offset);

    if (ptr == MAP_FAILED) {
        perror("Image buffer memmap failed\n");
        return;
    }

    //printf("Mapped plane: length=%zu offset=%zu map_offset=%zu map_length=%zu\n",
    //       (size_t)yplane.length, (size_t)yplane.offset, (size_t)map_offset, map_length);

    // copy the data to the global data array
    uint8_t *plane_data = static_cast<uint8_t*>(ptr) + map_diff;

    memcpy(data, plane_data, yplane.length);

    if (munmap(ptr, map_length) == -1) {
        perror("Image buffer munmap failed");
    }

    new_data = 1;

    // reuse and requeue request
    request->reuse(libcamera::Request::ReuseBuffers);
    camera->queueRequest(request);
}

int main(int argc, char *argv[]) {
    // setenv("GST_DEBUG", "3", 1);
    // gst_init(&argc, &argv);

    signal(SIGINT, handle_sigint);
    int ec;

    Settings settings; // global settings structure

    // state input items
    uint8_t state_in = STANDBY;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    struct termios oldt, newt;

    // Disable canonical mode so characters arrive immediately
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    fd_set read_fd;
    

    // libcamera objects
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    FrameBufferAllocator *allocator;
    Stream *stream;

    // apriltag items
    apriltag_detector_t *td;
    apriltag_family_t *tf;
    apriltag_detection_info_t info;
    apriltag_pose_t pose;
    int ids[MAX_DETECTIONS]; // array to hold detected ids
    uint8_t nids = 0;

    // pose array and transmission
    CoordDefs cd;
    matd_t *p = matd_create(3, 1); // position vector
    matd_t *q = matd_create(4, 1); // quaternion vector

    UARTInfo uart_info;

    // logging setup
    Logger logger;
    uint8_t log_options = LO_EN | LO_EN_IDS | LO_EN_POSES | LO_EN_QUATS | LO_EN_TIME;
    struct timeval tstart, tstop;

    // initializing logging
    char *log_filename = (char *)malloc(256 * sizeof(char));
    if (log_filename == NULL) {
        perror("Log filename allocation failed\n");
        exit(1);
    }
    ec = name_logfile(log_filename);
    if (ec) {
        printf("Failed to name logfile: %d\n", ec);
        exit(2);
    }

    ec = init_logger(&logger, log_filename, log_options);
    if (ec) {
        printf("Logger initialization failed with error code: %d\n", ec);
        exit(3);
    }

    // read in settings from json file, #TODO: make the path an arg (using stropts?)
    ec = load_settings_from_path(argv[1], &settings);
    if(ec) {
        printf("Settings failed to load with error code: %d\n", ec);
        exit(4);
    }
    settings.np = settings.width * settings.height;

    // allocating data
    data = (uint8_t *)malloc(settings.np * settings.stride);
    if (data == NULL) {
        perror("Image data allocation failed\n");
        exit(5);
    }

    // start the camera manager
    
    cm->start();

    // detect cameras attached to the system
    auto cameras = cm->cameras();
    if (cameras.empty()) {
        printf("Cam Setup: No cameras were identified on the system\n");
        cm->stop();
        exit(6);
    }

    // get the first registered camera
    std::string camera_id = cameras[0]->id();

    // try to acquire the camera for exclusive use
    camera = cm->get(camera_id);
    if (camera->acquire() != 0) {
        printf("Cam Setup: Camera disappeared since detection\n");
        cm->stop();
        exit(6);
    }

    printf("Current camera: %s\n", camera->id().c_str());

    // get and change the camera's configuration
    std::unique_ptr<libcamera::CameraConfiguration> config = camera->generateConfiguration({libcamera::StreamRole::Viewfinder});

    printf("Default configuration: %s\n", config->at(0).toString().c_str());

    libcamera::StreamConfiguration stream_conf = config->at(0); // prefils necessary values that aren't changed
    
    // change camera parameters here

    stream_conf.size.width = settings.width;
    stream_conf.size.height = settings.height;
    stream_conf.pixelFormat = libcamera::formats::YUV420;

    config->at(0) = stream_conf;

    if (config->validate() != libcamera::CameraConfiguration::Status::Valid) {
        printf("Cam setup: error trying to change configuration\n");
        exit(6);
    }

    // note that setting the configuration this way ignores the device's preferences
    // pisp (picam program) will select a different one most of the time
    printf("Configuration changed to: %s\n", config->at(0).toString().c_str());

    if (camera->configure(config.get())) {
        printf("Cam setup: error trying to change to configured state\n");
        exit(6);
    }

    allocator = new libcamera::FrameBufferAllocator(camera);

    stream = config->at(0).stream();

    if (allocator->allocate(stream) < 0) {
        printf("Cam setup: error trying to allocate for the stream\n");
        exit(ENOMEM);
    }

    size_t allocated = allocator->buffers(stream).size();
    printf("Cam setup: allocated %d buffers for the stream\n", allocated);

    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
    std::vector<std::unique_ptr<Request>> requests;

    for (unsigned int i = 0; i < buffers.size(); ++i) {
        std::unique_ptr<Request> request = camera->createRequest();
        if (!request)
        {
            printf("Cam setup: can't create request %d", i);
            continue;
        }

        const std::unique_ptr<FrameBuffer> &buffer = buffers[i];

        int ret = request->addBuffer(stream, buffer.get());
        if (ret < 0)
        {
            printf("Cam setup: can't assign buffer to request %d", i);
            exit(6);
        }

        requests.push_back(std::move(request));
    }

    camera->requestCompleted.connect(request_complete);

    if (camera->start()) {
        printf("Cam setup: error trying to start camera\n");
        exit(6);
    }

    libcamera_request(camera, &requests);

    // bus = gst_element_get_bus(streams.pipeline);

    // perform apriltag setup
    ec = apriltag_setup(&td, &tf, &info, &settings);
    if (ec) {
        printf("Setup returned error code: %d\n", ec);
        exit(7);
    }

    // perform UART setup
    ec = init_transmit_pose(&uart_info, &settings, &cd);
    if (ec) {
        printf("UART initialization failed with error code: %d\n", ec);
        exit(8);
    }

    // the main loop
    while(!stop) {
        FD_ZERO(&read_fd);
        FD_SET(STDIN_FILENO, &read_fd);

        if (select(STDIN_FILENO + 1, &read_fd, NULL, NULL, &timeout) > 0) {
            char s;
            read(STDIN_FILENO, &s, 1);
            s -= 48;

            if (s >= STANDBY && s < RETURN + 1) {
                state_in = s;
                printf("State changed to: %d\n", s);
                continue;
            }

            printf("State input not recognized\n");
        }

        if (new_data) {
            gettimeofday(&tstart, NULL);
            // pulling sample from camera and print bus error message, the only gstream functions used in a loop
            // ec = gstream_pull_sample(&streams, data, &settings);
            // if (ec) {
            //     continue;
            //     // do not exit, run something to fix the break in timing
            // }

            // detect apriltags and update the pose and ids array
            ec = apriltag_detect(td, data, &info, &pose, &settings, ids, &nids);
            if (ec) {
                printf("Apriltag detection returned error code: %d\n", ec);
                // do not exit, perform error handling based on what happened
                if (ec == 3) {
                    // no detections, continue
                    new_data = 0;
                    continue;
                }
            }

            // transform the pose to desired coordinates
            ec = pose_transform(p, q, &pose);
            if (ec) {
                printf("Pose transformation returned error code: %d\n", ec);
            }

            // transmit the pose through the UART bus
            ec = transmit_pose(&uart_info, &tstop, p, q, state_in);
            if (ec) {
                printf("Pose transmission returned error code: %d\n", ec);
            }

            // log 
            ec = log_message(&logger, p, q, ids, nids, &tstart, &tstop);
            if (ec) {
                printf("Logging returned error code: %d\n", ec);
            }

            // reset new data flag
            new_data = 0;
        }

    }

    printf("Exiting main loop...\n");

    // apriltag cleanup
    apriltag_cleanup(&td, &tf, &info);

    matd_destroy(p);
    matd_destroy(q);
    
    // free dynamically allocated image data array
    free(log_filename);
    free(data);

    // free the libcamera items
    camera->stop();
    allocator->free(stream);
    delete allocator;
    camera->release();
    camera.reset();
    cm->stop();
    delete cm.get();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    exit(0);
}
