
// ngotrung Tue 12 Oct 2021 10:58:35 AM +07
#ifndef STRING_UTILS_H
#define STRING_UTILS_H
#include "SDL.h"

struct StrBuf
{
    char*  data;
    size_t allocated;
    size_t used; // including null character
};

void strbuf_append_str(struct StrBuf* buf, const char* str);

void strbuf_append_fmt(struct StrBuf* buf, const char* fmt, ...);
#endif // STRING_UTILS_H
