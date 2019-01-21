#ifndef __SLS_DEBUG__
#define __SLS_DEBUG__

#ifdef DEBUG
#define DBG0(fmt) do { fprintf(stderr, fmt); } while (0)
#define DBG1(fmt, arg0) do { fprintf(stderr, fmt, arg0); } while (0)
#define DBG2(fmt, arg0, arg1) do { fprintf(stderr, fmt, arg0, arg1); } while (0)
#else
#define DBG0(fmt) do { } while (0)
#define DBG1(fmt, arg0) do { } while (0)
#define DBG2(fmt, arg0, arg1) do { } while (0)
#endif

#endif

