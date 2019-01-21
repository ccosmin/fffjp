#include "sls_json.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "sls_string.h"
#include "sls_buffer.h"
#include "sls_debug.h"

#define INITIAL_BUFFER_LENGTH 4096 
#define INITIAL_BUFFER_USABLE_LENGTH (INITIAL_BUFFER_LENGTH - 1)

static int eat(sls_buffer* buffer)
{
    char c = sls_buffer_getc(buffer);
    return c;
}

static int peek(sls_buffer* buffer)
{
    char c = sls_buffer_peek(buffer);
    if ( c == EOF )
        return 0;
    return c;
}

static void skip_ws(sls_buffer* buffer)
{
    char c;
    while ( (c = peek(buffer)) && isspace(c) )
        eat(buffer);
}

typedef struct sls_json_value_context
{
    const char* key_name;
    void* context;
} sls_json_value_context;

static int expect_char(sls_buffer* buffer, char expect, sls_json_callback* cb, void* context)
{
    DBG1("expecting char %c\n", expect);
    skip_ws(buffer);
    char c = eat(buffer);
    if ( c == EOF )
    {
        cb->error(context, "Unexpected EOF while expecting %c", expect);
        return 0;
    }
    if ( c != expect ) 
    {
        cb->error(context, "Expected %c but got %c", expect, c);
        return 0;
    }
    return 1;
}

static int parse_string_value2(sls_buffer* buffer, sls_string* s, sls_json_callback* cb, void* context)
{
    if ( ! expect_char(buffer, '"', cb, context) )
        return 0;
    *s = strmalloc(256);
    if ( isnas(s) ) 
    {
        cb->error(context, "Unable to allocate memory");
        return 0;
    }
    char c;
    int i = 0;
    while ( (c = peek(buffer)) != '"' )
    {
        if ( i >= strsize(s) )
            strrealloc(s);
        straddchar(s, c);
        i++;
        eat(buffer);
    }
    eat(buffer);

    if ( cb->event_value )
    {
        sls_json_value val = { 0 };
        val.str = strtocstr(s);
        cb->event_value(sls_json_value_type_string, &val, context);  
    }
    strfree(s);

    return 1;
}

static int parse_string_value(sls_buffer* buffer, sls_json_callback* cb, void* context)
{
    char scratch_buffer[INITIAL_BUFFER_LENGTH] = { 0 };
    sls_string s = { 0 };
    if ( ! expect_char(buffer, '"', cb, context) )
        return 0;
    char c;
    int i = 0;
    while ( (c = peek(buffer)) != '"' )
    {
        DBG1("c = %c\n", c);
        if ( i < INITIAL_BUFFER_USABLE_LENGTH ) 
        {
            scratch_buffer[i] = c;
        }
        else if ( i == INITIAL_BUFFER_LENGTH ) 
        {
            strncatcstr(&s, i, scratch_buffer);  
        }
        else
        {
            if ( i >= strsize(&s) )
                strrealloc(&s);
            straddchar(&s, c);
        }
        i++;
        eat(buffer);
    }
    eat(buffer);
    DBG0("parse_string_value: finished eating the string\n");

    if ( cb->event_value )
    {
        sls_json_value val = { 0 };
        if ( isnas(&s) )
        {
            DBG1("parse_string_value: duping \"%s\"\n", scratch_buffer);
            val.str = strdup(scratch_buffer);
        }
        else
        {
            val.str = strdup(strtocstr(&s));
        }
        DBG1("parse_string_value: \"%s\"\n", val.str);
        cb->event_value(sls_json_value_type_string, &val, context);  
    }
    strfree(&s);

    return 1;
}

static int parse_number(sls_buffer* buffer, long* l, double* d, int* is_floating, sls_json_callback* cb, void* context)
{
    char scratch[INITIAL_BUFFER_LENGTH] = { 0 };
    
    *is_floating = 0;
    char c = peek(buffer);
    int sign = c == '-' ? -1 : 1;
    if ( c == '+' || c == '-' )
        eat(buffer);
    int i = 0;
    while ( (c = peek(buffer)) && (isdigit(c) || c == '.' || c == 'e' || c == 'E') )
    {
        if ( i < INITIAL_BUFFER_USABLE_LENGTH )
        {
            scratch[i++] = c;
        }
        else
        {
            cb->error("number too long: %s", scratch);
            return 0;
        }

        if ( c == '.' || c == 'e' || c == 'E' )
            *is_floating = 1;
        eat(buffer);
    }
    if ( *is_floating ) 
    {
        *d = atof(scratch);
        *d = sign * *d;
    }
    else
    {
        *l = atol(scratch);
        *l = sign * *l;
    }
    if ( cb->event_value )
    {
        sls_json_value val = { 0 };
        if ( *is_floating )
        {
            val.number = *d;
            cb->event_value(sls_json_value_type_number, &val, context);
        }
        else
        {
            val.number_long = *l;
            cb->event_value(sls_json_value_type_number_long, &val, context);
        }
    }

    return 1;
}

static int read_n(sls_buffer* buffer, char** s, int n, sls_json_callback* cb, void* context)
{
    *s = (char*)malloc((n + 1) * sizeof(char));
    if ( !s )
    {
        cb->error(context, "Unable to allocate memory");
        return 0;
    }
    int i = 0;
    char c;
    while ( (i < n) && (c = eat(buffer)) != EOF )
        (*s)[i++] = c;

    (*s)[i] = '\0';
    
    return i == n;
}

static int parse_true(sls_buffer* buffer, int* boolean, sls_json_callback* cb, void* context)
{
    char* s = NULL;
    if ( ! read_n(buffer, &s, 4, cb, context) )
    {
        free(s);
        cb->error(context, "Unable to read true value");
        return 0;
    }
    
    *boolean = strcmp("true", s) == 0;
    free(s);
    return 1;
}

static int parse_false(sls_buffer* buffer, int* boolean, sls_json_callback* cb, void* context)
{
    char* s = NULL;
    if ( ! read_n(buffer, &s, 5, cb, context) )
    {
        free(s);
        cb->error(context, "Unable to read false value");
        return 0;
    }
    
    *boolean = strcmp("false", s) == 0;
    free(s);
    return 1;
}

static int parse_null(sls_buffer* buffer, int* is_null, sls_json_callback* cb, void* context)
{
    char* s = NULL;
    if ( ! read_n(buffer, &s, 4, cb, context) )
    {
        free(s);
        cb->error(context, "Unable to read null value");
        return 0;
    }
    
    *is_null = strcmp("null", s) == 0;
    free(s);
    return 1;
}

static int parse_array(sls_buffer* buffer, sls_json_callback* cb, void* context);
static int parse_object(sls_buffer* buffer, sls_json_callback* cb, void* context);

static int parse_value(sls_buffer* buffer, sls_json_callback* cb, void* context)
{
    skip_ws(buffer);
    char c = peek(buffer); 
    if ( c == '[' )
    {
        DBG0("parser_value: Starting an array\n");
        return parse_array(buffer, cb, context);
    }
    else if ( c == '{' )
    {
        DBG0("parse_value: Starting an object\n");
        return parse_object(buffer, cb, context);
    }
    else if ( c == '"' )
    {
        DBG0("parse_value: Starting a string\n");
        return parse_string_value(buffer, cb, context);
    }
    else if ( isdigit(c) || c == '-' || c == '.' )
    {
        long l;
        double d;
        int is_floating;
        return parse_number(buffer, &l, &d, &is_floating, cb, context);
    }
    else if ( c == 't' )
    {
        int b;
        return parse_true(buffer, &b, cb, context);
    }
    else if ( c == 'f' )
    {
        int b;
        return parse_false(buffer, &b, cb, context);
    }
    else if ( c == 'n' )
    {
        int is_null;
        return parse_null(buffer, &is_null, cb, context);
    }
    return 0;
}

static int parse_array(sls_buffer* buffer, sls_json_callback* cb, void* context)
{
    if ( ! expect_char(buffer, '[', cb, context) )
        return 0;
    if ( cb->event_start_array )
    {
        cb->event_start_array(context);
    }
    do 
    {
        char c = peek(buffer);
        if ( c == ']' )
        {
            eat(buffer);
            return 1;
        }
        
        if ( ! parse_value(buffer, cb, context) )
            return 0;

        skip_ws(buffer);

        c = peek(buffer);
        if ( c == ',' )
        {
            eat(buffer);
            continue;
        }

        if ( c == ']' )
        {
            eat(buffer);
            break;
        }
    }
    while ( 1 );

    if ( cb->event_end_array )
    {
        cb->event_end_array(context);
    }

    return 1;
}

static int parse_key(sls_buffer* buffer, char** s, sls_json_callback* cb, void* context)
{
    skip_ws(buffer);
    *s = (char*)malloc(256 * sizeof(char));
    if ( ! expect_char(buffer, '"', cb, context) )
        return 0;
    char c;
    int i = 0;
    while ( (c = peek(buffer)) != '"' )
    {
        (*s)[i++] = c;
        eat(buffer);
    }
    eat(buffer);
    (*s)[i] = '\0';
    DBG1("Parsed key: %s\n", *s);
    if ( cb->event_key )
    {
        cb->event_key(*s, context);
    }
    return 1;
}

static int parse_key_value(sls_buffer* buffer, sls_json_callback* cb, void* context)
{
    char* key_name = NULL;
    if ( ! parse_key(buffer, &key_name, cb, context) )
        return 0;
    if ( ! expect_char(buffer, ':', cb, context) )
        return 0;
    if ( ! parse_value(buffer, cb, context) ) 
        return 0;
    return 1;
}

static int parse_object(sls_buffer* buffer, sls_json_callback* cb, void* context)
{
    if ( ! expect_char(buffer, '{', cb, context) )
        return 0;
    if ( cb->event_start_object ) 
    {
        cb->event_start_object(context);
    }
    char c;
    while ( c = peek(buffer) )
    {
        skip_ws(buffer);

        if ( c == '}' )
        {
            DBG0("Found end of object\n");
            eat(buffer);
            break;
        }
        
        if ( ! parse_key_value(buffer, cb, context) )
            return 0;

        skip_ws(buffer);

        c = peek(buffer);
        if ( c == ',' )
        {
            eat(buffer);
            continue;
        } 
        else if ( c == '}' )
        {
            eat(buffer);
            break;
        }
        else
        {
            cb->error(context, "Illegal character %c", c);
            return 0;
        }
    }
    if ( cb->event_end_object ) 
    {
        cb->event_end_object(context);
    }
    return 1;
}

int sls_parse(sls_buffer* buffer, sls_json_callback* cb, void* context)
{
    return parse_value(buffer, cb, context);
}

