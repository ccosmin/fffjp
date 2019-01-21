#ifndef __CSTR_H__
#define __CSTR_H__

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct sls_string sls_string;
struct sls_string
{
	char *s;
	size_t size;
	size_t b_size;
};

#define NaS ((sls_string) {NULL, 0, 0})
#define isnas(S) (!(S)->s)

static size_t strsize(sls_string *s)
{
	if (isnas(s)) return 0;
	return s->size;
}

#define STR_FREEABLE (1ULL << 63)

/* An initialized empty struct sls_string */
#define STRINIT ((sls_string) {malloc(16), 0, (16)})

static sls_string strmalloc(size_t size)
{
	if (size < 16) size = 16;
	return (sls_string) {malloc(size), 0, size | STR_FREEABLE};
}

/* Try to compact sls_string memory */
static void strrealloc(sls_string *s)
{
	char *buf;
	
	/* Not a sls_string? */
	if (isnas(s)) return;
	
	/* Can't realloc? */
	if (!(s->b_size & STR_FREEABLE)) return;
	
	/* Don't invoke undefined behaviour with realloc(x, 0) */
	if (!s->size)
	{
		free(s->s);
		s->s = malloc(16);
	}
	else
	{
		/* Try to compact */
		buf = realloc(s->s, s->size);
		if (buf) s->s = buf;
	}
}

static void strresize(sls_string *s, size_t size)
{
	char *buf;
	size_t bsize;
	
	/* Are we not a sls_string? */
	if (isnas(s)) return;
	
	/* Not resizable */
	if (!(s->b_size & STR_FREEABLE))
	{
		sls_string s2;
		
		/* Don't do anything if we want to shrink */
		if (size <= s->size) return;
		
		/* Need to alloc a new sls_string */
		s2 = strmalloc(size);
		
		/* Copy into new sls_string */
		memcpy(s2.s, s->s, s->size);
		
		/* Point to new sls_string */
		s->s = s2.s;
		s->b_size = s2.b_size;
		
		return;
	}
	
	/* Too big */
	if (size & STR_FREEABLE)
	{
		free(s->s);
		*s = NaS;
		return;
	}
	
	bsize = s->b_size - STR_FREEABLE;
	
	/* Keep at least 16 bytes */
	if (size < 16) size = 16;
	
	/* Nothing to do? */
	if ((4 * size > 3 * bsize) && (size <= bsize)) return;
	
	/* Try to double size instead of using a small increment */
	if ((size > bsize) && (size < bsize * 2)) size = bsize * 2;
	
	/* Keep at least 16 bytes */
	if (size < 16) size = 16;

	buf = realloc(s->s, size);
	
	if (!buf)
	{
		/* Failed, go to NaS state */
		free(s->s);
		*s = NaS;
	}
	else
	{
		s->s = buf;
		s->b_size = size | STR_FREEABLE;
	}
}

static void straddchar(sls_string* s, char c)
{
    if ( strsize(s) >= s->b_size ) 
        strresize(s, s->b_size * 2);
    s->s[s->size++] = c;
}

static void strfree(sls_string *s)
{
	if (s->b_size & STR_FREEABLE) free(s->s);
	
	*s = NaS;
}

/* Allocate room for a struct sls_string on the stack */
#define stralloca(S) ((sls_string) {alloca(S), 0, (S)})

/*
 * Copy a struct sls_string to the stack.
 * (Could use strdupa(), but this is more portable)
 */
#define stradupstr_aux(S)\
	__extension__ ({\
		char *_stradupstr_aux = alloca((S).size + 1);\
		memcpy(_stradupstr_aux, (S).s, (S).size);\
		strcstr_aux(_stradupstr_aux, (S).size);\
	})

#define stradupstr(S) strdupastr_aux(*(S))

/* A struct sls_string based on a C sls_string, stored on the stack */
#define S(C) stradupstr_aux(strcstr((char *)C))

static sls_string strcstr_aux(char *c, size_t len)
{
	return (sls_string) {c, len, len + 1};
}

/* A struct sls_string based on a C sls_string, stored in whatever c points to */
static sls_string strcstr(char *c)
{
	size_t len = strlen(c);
	return strcstr_aux(c, len);
}

/* Create a new sls_string as a copy of an old one */
static sls_string strdupstr(sls_string *s)
{
	sls_string s2;
	
	/* Not a sls_string? */
	if (isnas(s)) return NaS;
	
	s2 = strmalloc(s->size);
	s2.size = s->size;
	memcpy(s2.s, s->s, s->size);
	
	return s2;
}

/* Copy the memory from the source sls_string into the dest sls_string */
static void strcpystr(sls_string *dest, sls_string *src)
{
	/* Are we no a sls_string */
	if (isnas(src)) return;
	
	strresize(dest, src->size);
	
	if (isnas(dest)) return;
	dest->size = src->size;
	memcpy(dest->s, src->s, src->size);
}

static char *strtocstr(sls_string *s)
{
	size_t bsize;
	
	/* Are we not a sls_string? */
	if (isnas(s)) return NULL;
	
	/* Get real buffer size */
	bsize = s->b_size & ~STR_FREEABLE;
	
	if (s->size == bsize)
	{
		/* Increase buffer size */
		strresize(s, bsize + 1);
		
		/* Are we no longer a sls_string? */
		if (isnas(s)) return NULL;
	}
	
	/* Tack a zero on the end */
	s->s[s->size] = 0;
	
	/* Don't update the size */
	
	/* Can use this buffer as long as you don't append anything else */
	return s->s;
}

#ifdef DEBUG_BOUNDS
#define S_C(S, I)\
	(* __extension__ ({\
		assert((I) >= 0);\
		assert((I) < (S)->size);\
		assert((I) < ((S)->b_size & ~STR_FREEABLE));\
		&((S)->s[I]);\
	}))
#else
#define S_C(S, I) ((S)->s[I])
#endif

static void strncatcstr(sls_string *s, size_t len, const char *str)
{
	size_t bsize;
	
	/* Are we not a sls_string? */
	if (isnas(s)) return;
	
	/* Nothing to do? */
	if (!str || !len) return;
	
	/* Get real buffer size */
	bsize = s->b_size & ~STR_FREEABLE;
	
	if (s->size + len >= bsize)
	{
		strresize(s, s->size + len);
		
		/* Are we no longer a sls_string? */
		if (isnas(s)) return;
	}
	
	memcpy(&s->s[s->size], str, len);
	s->size += len;
}

static void strcatcstr(sls_string *s, const char *str)
{
	if (str) strncatcstr(s, strlen(str), str);
}

static void strcatstr(sls_string *s, const sls_string *s2)
{
	strncatcstr(s, s2->size, s2->s);
}

static void strcatcstrs(sls_string *s, ...)
{
	const char *str;
	va_list v;
	
	/* Are we not a sls_string? */
	if (isnas(s)) return;
	
	va_start(v, s);
	
	for (str = va_arg(v, const char *); str; str = va_arg(v, const char *))
	{
		strncatcstr(s, strlen(str), str);
	}
	
	va_end(v);
}

static void strcatstrs(sls_string *s1, ...)
{
	const sls_string *s2;
	va_list v;
	
	/* Are we not a sls_string? */
	if (isnas(s1)) return;
	
	va_start(v, s1);
	
	for (s2 = va_arg(v, const sls_string *); s2; s2 = va_arg(v, const sls_string *))
	{
		strncatcstr(s1, s2->size, s2->s);
	}
	
	va_end(v);
}

static void strprintf(sls_string *s, const char *fmt, ...)
{
	va_list v;
	size_t len;
	
	/* Are we not a sls_string? */
	if (isnas(s)) *s = STRINIT;
	
	/* Nothing to do? */
	if (!fmt) return;
	
	va_start(v, fmt);
	len = vsnprintf(NULL, 0, fmt, v) + 1;
	va_end(v);
	
	strresize(s, len);
		
	/* Are we no longer a sls_string? */
	if (isnas(s)) return;

	va_start(v, fmt);
	vsnprintf(s->s, len, fmt, v);
	va_end(v);
	s->size = len - 1;
}

/* Use a (C sls_string) format and return a stack-allocated struct sls_string */
#define straprintf(...)\
	__extension__ ({\
		size_t _straprintf_len = snprintf(NULL, 0, __VA_ARGS__) + 1;\
		char *_straprintf_buf = alloca(_straprintf_len);\
		snprintf(_straprintf_buf, _straprintf_len, __VA_ARGS__);\
		strcstr_aux(_straprintf_buf, _straprintf_len - 1);\
	})

#endif

