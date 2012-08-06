#ifndef __WINCSTRING_H__
#define __WINCSTRING_H__

#include <cstring>

#if defined(WIN32) && !defined(__MINGW32__)

/*
 * some functions have strange names on windows
 */
#ifndef strcasecmp
#define strcasecmp   _stricmp
#endif
#ifndef strncasecmp
#define strncasecmp  _strnicmp
#endif
#ifndef snprintf
#define snprintf(buf, n, format, ...) _snprintf_s(buf, n + 1, n, format, __VA_ARGS__)
#endif

#endif

#endif
