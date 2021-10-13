#include "string_utils.h"

void
strbuf_append_str(struct StrBuf* buf, const char* str)
{
    if (!str || !buf)
        return;
    char* curr = buf->data + buf->used;

    const size_t max_len = (buf->allocated - buf->used) - 1u;

    int count = SDL_snprintf(curr, max_len, "%s", str);

    if (count > 0)
    {
        buf->used += (size_t)count;
        buf->data[buf->used] = '\0';
    }
}

void
strbuf_append_fmt(struct StrBuf* buf, const char* fmt, ...)
{
    if (!fmt || !buf)
        return;

    char*        curr    = buf->data + buf->used;
    const size_t max_len = (buf->allocated - buf->used) - 1u;
    va_list      args;

    va_start(args, fmt);
    int count = SDL_vsnprintf(curr, max_len, fmt, args);
    if (count > 0)
    {
        buf->used += (size_t)count;
        buf->data[buf->used] = '\0';
    }
    va_end(args);
}
