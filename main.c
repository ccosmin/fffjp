#include "sls_json.h"
#include "sls_buffer.h"

int main(int argc, char* argv[])
{
    const char* fn = argv[1];
    sls_buffer* buffer = sls_buffer_create_from_file(fn, 1024);
    sls_json_callback cb = { 0 };
    sls_parse(buffer, &cb, NULL);
    sls_buffer_destroy(buffer);
    return 0;
}
