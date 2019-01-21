#ifndef __SLS_JSON_H__
#define __SLS_JSON_H__

#include <stdio.h>

typedef enum sls_json_value_type
{
    sls_json_value_type_string,
    sls_json_value_type_number,
    sls_json_value_type_number_long,
    sls_json_value_type_boolean,
    sls_json_value_type_null
} sls_json_value_type;

typedef union sls_json_value
{
    const char* str;
    double number;
    long number_long;
    int boolean;
} sls_json_value;

typedef struct sls_json_callback sls_json_callback;
struct sls_json_callback
{
    int (*event_start_object)(void* context);
    int (*event_end_object)(void* context);
    int (*event_start_array)(void* context);
    int (*event_end_array)(void* context);
    int (*event_key)(const char* key_name, 
                     void* context);
    int (*event_value)(sls_json_value_type type, 
                       const sls_json_value* value, 
                       void* context);
    int (*error)(void* context, const char* fmt, ...);
};

struct sls_buffer;
int sls_parse(struct sls_buffer* buffer, sls_json_callback* cb, void* context);

#endif

