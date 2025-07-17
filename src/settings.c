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


int load_settings_from_path(const char* path, Settings *settings) {
    struct json_object* tmp = NULL;

    int success = 0;

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

    json_object_put(jobj);
    success = 1;
    return 0;
}

