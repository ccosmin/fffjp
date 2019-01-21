#ifndef __SLS_JSON_NODE_H__
#define __SLS_JSON_NODE_H__

#include <stdio.h>
#include "sls_json.h"

typedef enum sls_json_node_type
{
    sls_json_node_object,
    sls_json_node_array,
    sls_json_node_string,
    sls_json_node_number,
    sls_json_node_boolean,
    sls_json_node_null,
};

struct sls_json_node;

typedef struct sls_json_node_kv
{
    const char* key;
    struct sls_json_node* value;
};

typedef struct sls_json_node_object
{
    sls_json_node_kv* kvs;
    size_t size;
};

typedef struct sls_json_node_array
{
    sls_json_node* array;
    size_t size;
};

typedef union sls_json_node_content
{
    sls_json_node_array* array;
    sls_json_node_object* object;
    sls_json_value* value;
};

typedef struct sls_json_node
{
    sls_json_node_content content;
    sls_json_node_type type;
};

sls_json_node* sls_json_node_parse(struct sls_buffer* buffer);

#endif

