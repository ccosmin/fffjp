#include "sls_buffer.h"

int main(int argc, char* argv[])
{
    sls_buffer* b = sls_buffer_create_from_str("abcdefg", 2);
    char c;
    while ( (c = sls_buffer_getc(b)) > 0 )
        printf("%c", c);
    sls_buffer_destroy(b);
    return 0;
}
