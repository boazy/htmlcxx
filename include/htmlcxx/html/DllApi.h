// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the HTMLCXX_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// HTMLCXX_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#if defined(HTMLCXX_LIB)
#  define HTMLCXX_API
#elif defined(HTMLCXX_EXPORTS)
#  define HTMLCXX_API __declspec(dllexport)
#else
#  define HTMLCXX_API __declspec(dllimport)
#endif

#ifdef _MSC_VER
#  pragma warning(disable: 4251)
#endif

