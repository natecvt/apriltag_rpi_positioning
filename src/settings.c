#include <settings.h>

static json_object *jobj;

////////////////////////////////////////////////////////////////////////////////
/// MACROS FOR PARSING JSON TYPES
////////////////////////////////////////////////////////////////////////////////

// macro for reading a boolean
#define PARSE_BOOL(name)                                                                 \
    if (json_object_object_get_ex(jobj, #name, &tmp) == 0)                               \
    {                                                                                    \
        fprintf(stderr, "ERROR parsing settings file, can't find " #name "\n");          \
        return -1;                                                                       \
    }                                                                                    \
    if (json_object_is_type(tmp, json_type_boolean) == 0)                                \
    {                                                                                    \
        fprintf(stderr, "ERROR parsing settings file, " #name " should be a boolean\n"); \
        return -1;                                                                       \
    }                                                                                    \
    settings->name = json_object_get_boolean(tmp);

// macro for reading an integer
#define PARSE_INT(name)                                                               \
    if (json_object_object_get_ex(jobj, #name, &tmp) == 0)                            \
    {                                                                                 \
        fprintf(stderr, "ERROR parsing settings file, can't find " #name "\n");       \
        return -1;                                                                    \
    }                                                                                 \
    if (json_object_is_type(tmp, json_type_int) == 0)                                 \
    {                                                                                 \
        fprintf(stderr, "ERROR parsing settings file, " #name " should be an int\n"); \
        return -1;                                                                    \
    }                                                                                 \
    settings->name = json_object_get_int(tmp);

// macro for reading a floating point number
#define PARSE_DOUBLE_MIN_MAX(name, min, max)                                \
    if (json_object_object_get_ex(jobj, #name, &tmp) == 0)                  \
    {                                                                       \
        fprintf(stderr, "ERROR can't find " #name " in settings file\n");   \
        return -1;                                                          \
    }                                                                       \
    if (json_object_is_type(tmp, json_type_double) == 0)                    \
    {                                                                       \
        fprintf(stderr, "ERROR " #name " should be a double\n");            \
        return -1;                                                          \
    }                                                                       \
    settings->name = json_object_get_double(tmp);                           \
    if (settings->name < min || settings->name > max)                       \
    {                                                                       \
        fprintf(stderr, "ERROR " #name " should be between min and max\n"); \
        return -1;                                                          \
    }

// macro for reading a string
#define PARSE_STRING(name)                                                              \
    if (json_object_object_get_ex(jobj, #name, &tmp) == 0)                              \
    {                                                                                   \
        fprintf(stderr, "ERROR parsing settings file, can't find " #name "\n");         \
        return -1;                                                                      \
    }                                                                                   \
    if (json_object_is_type(tmp, json_type_string) == 0)                                \
    {                                                                                   \
        fprintf(stderr, "ERROR parsing settings file, " #name " should be a string\n"); \
        return -1;                                                                      \
    }                                                                                   \
    strcpy(settings->name, json_object_get_string(tmp));

float get_float_at(char *stream, int at) {
    int j = 0, n = 0;
    while (n < at) {
        if (j > FLEN) return 0.0f;
        n += stream[j] == 32; // ASCII code for space, delimiter in .cal file
        j++;
    }

    return strtod(stream + j, NULL);
}

int load_settings_from_path(const char* path, Settings *settings) {
    struct json_object* tmp = NULL;

    if (access(path, F_OK) != 0) {
        printf("Incorrect Settings Path\n");
        return 1;
    }

    jobj = json_object_from_file(path);
    if (jobj == NULL) {
        printf("Failed to Read Settings File\n");
        return 2;
    }

    PARSE_INT(width);
    PARSE_INT(height);
    PARSE_BOOL(is_height_from_ar);
    // #TODO: create macro to parse aspect ratio
    PARSE_INT(framerate);
    PARSE_INT(stride);

    PARSE_BOOL(debug);
    PARSE_BOOL(quiet);
    PARSE_INT(iterations);
    PARSE_INT(hamming);
    PARSE_INT(threads);

    // #TODO: find actual minimum and maximum values or redefine macro for no limits
    PARSE_DOUBLE_MIN_MAX(dec, 0.0f, 4.0f);
    PARSE_DOUBLE_MIN_MAX(blur, -1.0f, 1.0f);
    PARSE_BOOL(refine);
    PARSE_INT(tag_family);
    PARSE_DOUBLE_MIN_MAX(tag_size, 0.01f, 1.0f);

    (*settings).output_directory = (char*)malloc(PLEN);
    PARSE_STRING(output_directory);

    (*settings).cal_file_path = (char*)malloc(PLEN);
    PARSE_STRING(cal_file_path);

    PARSE_BOOL(use_preset_camera_calibration);
    if (settings->use_preset_camera_calibration) {
        PARSE_DOUBLE_MIN_MAX(fx,          0.0, __FLT_MAX__);
        PARSE_DOUBLE_MIN_MAX(fy,          0.0, __FLT_MAX__);
        PARSE_DOUBLE_MIN_MAX(cx, -__FLT_MAX__, __FLT_MAX__);
        PARSE_DOUBLE_MIN_MAX(cy, -__FLT_MAX__, __FLT_MAX__);
    }
    else {
        FILE* f = fopen(settings->cal_file_path, "r");

        if (f == NULL) {
            printf("Failed to open calibration file or invalid path");
        }

        char *stream = (char*)malloc(FLEN);

        if (fgets(stream, FLEN, f) != NULL) {
            settings->fx = get_float_at(stream, 0);
            settings->fy = get_float_at(stream, 1);
            settings->cx = get_float_at(stream, 2);
            settings->cy = get_float_at(stream, 3);
        }
        else {
            printf("Failed to read calibration file or invalid format, using default values");
            PARSE_DOUBLE_MIN_MAX(fx,         0.0f, __FLT_MAX__);
            PARSE_DOUBLE_MIN_MAX(fy,         0.0f, __FLT_MAX__);
            PARSE_DOUBLE_MIN_MAX(cx, -__FLT_MAX__, __FLT_MAX__);
            PARSE_DOUBLE_MIN_MAX(cy, -__FLT_MAX__, __FLT_MAX__);
        }

        free(stream);
        fclose(f);
    }

    PARSE_DOUBLE_MIN_MAX(grid_unit_length, 0.0f, 10.0f);
    PARSE_DOUBLE_MIN_MAX(grid_unit_width, 0.0f, 10.0f);
    PARSE_DOUBLE_MIN_MAX(grid_elevation, -__FLT_MAX__, __FLT_MAX__); // positive down, negative up
    PARSE_INT(grid_units_x);
    PARSE_INT(grid_units_y);
    
    PARSE_BOOL(use_computed_center);
    if (settings->use_computed_center) {
        settings->center_id = (settings->grid_units_x / 2) + settings->grid_units_x * (settings->grid_units_y / 2);
    }
    else {
        PARSE_INT(center_id);
    }

    json_object_put(jobj);
    return 0;
}

