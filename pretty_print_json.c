#include <stdarg.h>
#include "sls_json.h"
#include "sls_buffer.h"

static int error(void* context, const char* fmt, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
}

typedef struct Ctxt 
{
    int indent_level;
    int value_in_key;
} Ctxt;

static void indent(int level)
{
    for ( int i = 0; i < level; ++i )
        fprintf(stdout, "\t");
}

static int event_key(const char* key_name, void* context)
{
    Ctxt* ctxt = (Ctxt*)context;
    indent(ctxt->indent_level);
    fprintf(stdout, "\"%s\": ", key_name);
    ctxt->value_in_key = 1;
}

static int event_value(sls_json_value_type type, 
                       const sls_json_value* value, 
                       void* context)
{
    Ctxt* ctxt = (Ctxt*)context;
    if ( ! ctxt->value_in_key )
        indent(ctxt->indent_level);
    switch ( type ) 
    {
        case sls_json_value_type_string:
            fprintf(stdout, "\"%s\",\n", value->str);
            break;
        case sls_json_value_type_number:
            fprintf(stdout, "%f,\n", value->number);
            break;
        case sls_json_value_type_number_long:
            fprintf(stdout, "%d,\n", value->number_long);
            break;
        case sls_json_value_type_boolean:
            if ( value->boolean )
                fprintf(stdout, "true,\n");
            else
                fprintf(stdout, "false,\n");
            break;
        case sls_json_value_type_null:
            fprintf(stdout, "null,\n", value->str);
            break;
    }
    ctxt->value_in_key = 0;
}

static int event_start_object(void* context)
{
    Ctxt* ctxt = (Ctxt*)context;
    indent(ctxt->indent_level);
    fprintf(stdout, "{\n");
    ctxt->indent_level++;
}

static int event_end_object(void* context)
{
    Ctxt* ctxt = (Ctxt*)context;
    fprintf(stdout, "},\n");
    ctxt->indent_level--;
    indent(ctxt->indent_level);
    ctxt->value_in_key = 0;
}

static int event_start_array(void* context)
{
    Ctxt* ctxt = (Ctxt*)context;
    indent(ctxt->indent_level);
    fprintf(stdout, "[\n");
    ctxt->indent_level++;
    ctxt->value_in_key = 0;
}

static int event_end_array(void* context)
{
    Ctxt* ctxt = (Ctxt*)context;
    fprintf(stdout, "],\n");
    ctxt->indent_level--;
    indent(ctxt->indent_level);
    ctxt->value_in_key = 0;
}

int main(int argc, char* argv[])
{
    if ( argc != 2 )
    {
        fprintf(stderr, "pretty_print_json <file_to_pretty_print>\n");
        return 1;
    }
    const char* fn = argv[1];
    sls_buffer* buffer = sls_buffer_create_from_file(fn, 1024);
    sls_json_callback cb = { 0 };
    cb.error = &error;
    cb.event_key = &event_key;
    cb.event_value = &event_value;
    cb.event_start_object = &event_start_object; 
    cb.event_end_object = &event_end_object;
    cb.event_start_array = &event_start_array;
    cb.event_end_array = &event_end_array;

    Ctxt ctxt = { 0 };
    sls_parse(buffer, &cb, &ctxt);
    sls_buffer_destroy(buffer);
    return 0;
}
