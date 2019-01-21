#include <stdarg.h>
#include <stdio.h>
#include "sls_buffer.h"
#include "sls_tinytest.h"

static int read_known_string(sls_buffer* b)
{
    TINYTEST_ASSERT(sls_buffer_getc(b) == 'a');
    TINYTEST_ASSERT(sls_buffer_getc(b) == 'b');
    TINYTEST_ASSERT(sls_buffer_getc(b) == 'c');
    TINYTEST_ASSERT(sls_buffer_getc(b) == 'd');
    TINYTEST_ASSERT(sls_buffer_getc(b) == 'e');
    TINYTEST_ASSERT(sls_buffer_getc(b) == 'f');
    TINYTEST_ASSERT(sls_buffer_getc(b) == 'g');
    TINYTEST_ASSERT(sls_buffer_getc(b) == EOF);
    return 1;
}

static int read_from_string(const char* pName)
{
    sls_buffer* b = sls_buffer_create_from_str("abcdefg", 2);
    read_known_string(b);
    sls_buffer_destroy(b);
    return 1;
}

static int peek_does_not_advance_stream(const char* pname)
{
    sls_buffer* b = sls_buffer_create_from_str("abcdefg", 2);
    int i = 0;
    while ( i++ < 1000 )
        sls_buffer_peek(b);
    read_known_string(b);
    sls_buffer_destroy(b);
    return 1;
}

static int read_from_file(const char* pName)
{
    sls_buffer* b =  sls_buffer_create_from_file("data/citylots.json", 4096);
    char c = 0;
    while((c = sls_buffer_getc(b)) != EOF);
    sls_buffer_destroy(b);
    return 1;
}

TINYTEST_START_SUITE(buffer);
    TINYTEST_ADD_TEST(read_from_string, NULL, NULL);
    TINYTEST_ADD_TEST(peek_does_not_advance_stream, NULL, NULL);
    TINYTEST_ADD_TEST(read_from_file, NULL, NULL);
TINYTEST_END_SUITE();

TINYTEST_MAIN_SINGLE_SUITE(buffer);
