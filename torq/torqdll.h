#if ! defined TORQDLL_H
#define TORQDLL_H

#if defined _MSC_VER
#include <windows.h>
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#define WINAPI
#endif

extern "C" {

EXPORT int WINAPI torq_main(int argc, const char **argv);
EXPORT void WINAPI torq_app_version(int *pMajor, int *pMin0, int *pMin1);

}; // extern "C"

#endif // defined TORQDLL_H
