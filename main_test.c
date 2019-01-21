#include <stdarg.h>
#include <stdio.h>
#include "sls_json.h"
#include "sls_tinytest.h"
#include "sls_buffer.h"

static int error(void* context, const char* fmt, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
}

static int value(sls_json_value_type vt, const sls_json_value* v, void* context)
{
    fprintf(stderr, "%s\n", v->str);
}

static int open_parse_file(const char* fn)
{
    sls_json_callback cb = { 0 }; 
    cb.error = &error;
    //cb.event_value = &value;
    sls_buffer* b = sls_buffer_create_from_file(fn, 4096);
    int res = sls_parse(b, &cb, NULL);
    sls_buffer_destroy(b);
    return res;
}

static int empty_array(const char* pName)
{
    TINYTEST_ASSERT(open_parse_file("data/empty_array.json"));
}

static int empty_object(const char* pName)
{
    TINYTEST_ASSERT(open_parse_file("data/empty_object.json"));
}

static int object_with_keys(const char* pName)
{
    TINYTEST_ASSERT(open_parse_file("data/object_with_keys.json"));
}

static int array_with_number(const char* pName)
{
    TINYTEST_ASSERT(open_parse_file("data/array_with_number.json"));
}

static int object_with_null(const char* pName)
{
    TINYTEST_ASSERT(open_parse_file("data/object_with_null.json"));
}

static int huge_number(const char* pName)
{
    TINYTEST_ASSERT(open_parse_file("data/object_with_pi.json"));
}

TINYTEST_START_SUITE(sls_json);
    TINYTEST_ADD_TEST(empty_array, NULL, NULL);
    TINYTEST_ADD_TEST(empty_object, NULL, NULL);
    TINYTEST_ADD_TEST(object_with_keys, NULL, NULL);
    TINYTEST_ADD_TEST(array_with_number, NULL, NULL);
    TINYTEST_ADD_TEST(object_with_null, NULL, NULL);
    TINYTEST_ADD_TEST(huge_number, NULL, NULL);
TINYTEST_END_SUITE();

TINYTEST_MAIN_SINGLE_SUITE(sls_json);
