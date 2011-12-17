#if ! defined FFUNCRENAMER_H
#define FFUNCRENAMER_H

#include <stdio.h>

#if defined _MSC_VER

#if defined NOLOCK_WRITE

#define FWRITE _fwrite_nolock
#define FREAD _fread_nolock
#define FGETC _fgetc_nolock
#define FPUTC  _fputc_nolock
#define FTELL64 _ftelli64_nolock
#define FTELL _ftell_nolock
#define FSEEK64 _fseeki64_nolock
#define FSEEK _fseek_nolock

#else

#define FWRITE fwrite
#define FREAD fread
#define FGETC fgetc
#define FPUTC fputc
#define FTELL64 _ftelli64
#define FTELL ftell
#define FSEEK64 _fseeki64
#define FSEEK fseek

#if _MSC_VER >= 1300 && _MSC_VER < 1400

extern "C" {

__int64 __cdecl _ftelli64 (FILE *stream);
int __cdecl _fseeki64 (FILE *stream, __int64 offset, int whence);

};

#endif

#endif

#elif defined __GNUC__

#define FWRITE fwrite
#define FREAD fread
#define FGETC fgetc
#define FPUTC fputc
#define FTELL64 ftello64
#define FTELL ftell
#define FSEEK64 fseeko64
#define FSEEK fseek

#endif

inline void FWRITEBYTES(const char *buf, size_t bufBytes, FILE *pf) { FWRITE(buf, sizeof(char), bufBytes, pf); }

#endif // FFUNCRENAMER_H

