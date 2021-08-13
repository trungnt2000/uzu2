#include "text_format.h"
#include "cglm/cglm.h"
#include "toolbox/log.h"
#include <stdarg.h>

//#define TEXT_FORMAT_COLOR_NAMES_SIZE 4u
//#define TEXT_FORMAT_DEFAULT_COLOR \
//  { \
//    1.0, 1.0, 1.0, 1.0 \
//  }
// static const char* TextFormatColorNames[TEXT_FORMAT_COLOR_NAMES_SIZE] = {
//  "def",
//  "red",
//  "green",
//  "blue"
//};
//
// static const vec4 defaultColor = TEXT_FORMAT_DEFAULT_COLOR;
//
// static const vec4 TextFormatColors[TEXT_FORMAT_COLOR_NAMES_SIZE] = {
//  TEXT_FORMAT_DEFAULT_COLOR,
//  { 1.0, 0.0, 0.0, 1.0 },
//  { 0.0, 1.0, 0.0, 1.0 },
//  { 0.0, 0.0, 1.0, 1.0 }
//};

void
text_format_context_reset(TextFormatContext* ctx)
{
  ctx->lastToken          = TFTCodePoint;
  ctx->unitsSize          = 0;
  ctx->unitsCapacity      = 1024;
  ctx->colorsSize         = 0;
  ctx->colorCapacity      = 24;
  ctx->codePointsSize     = 0;
  ctx->codePointsCapacity = 1024;
  ctx->formatBufferIdx    = 0;
}

void
text_format_context_init(TextFormatContext* ctx, const vec4 defaultColor)
{
  text_format_context_reset(ctx);
  glm_vec4_copy(defaultColor, ctx->defaultColor);
}

static inline int
hex2dec(u32 codePoint)
{
  const int cp = (int)codePoint;
  if (cp >= (int)'0' && cp <= (int)'9')
    return (int)codePoint - (int)'0';
  else if (cp >= (int)'a' && cp <= (int)'f')
    return cp - (int)'a' + 10;
  else if (cp >= (int)'A' && cp <= (int)'F')
    return cp - (int)'A' + 10;
  else
  {
    ASSERT_MSG(false, "format syntax error: invalid color code");
    return -1;
  }
}

static inline int
text_format_next(u32 format, TextFormatContext* ctx)
{
  // read escape
  if (ctx->lastToken == TFTEscape)
  {
    switch (format)
    {
    case '\\':
      // escape the escape character
      ctx->codePoints[ctx->codePointsSize++] = '\\';
      ctx->lastToken                         = TFTCodePoint;
      ctx->units[ctx->unitsSize++]           = TFTCodePoint;
      return 0;
    case 'c':
      ctx->lastToken = TFTChangeColorStart;
      return TFTChangeColorStart;
    case 'n':
      ctx->lastToken               = TFTNewLine;
      ctx->units[ctx->unitsSize++] = TFTNewLine;
      return TFTNewLine;
    case 'i':
      ctx->lastToken               = TFTPopInteger;
      ctx->units[ctx->unitsSize++] = TFTPopInteger;
      return TFTPopInteger;
    case 'f':
      ctx->lastToken               = TFTPopFloat;
      ctx->units[ctx->unitsSize++] = TFTPopFloat;
      return TFTPopFloat;
    case 's':
      ctx->lastToken               = TFTPopString;
      ctx->units[ctx->unitsSize++] = TFTPopString;
      return TFTPopString;
    default:
      ASSERT_MSG(false, "format syntax error: escape unreconized token");
      return TFT_CNT;
    }
  }
  // read color enums
  else if (ctx->lastToken == TFTChangeColorStart)
  {
    // escape color change
    if (format == '\\')
    {
      ctx->formatBuffer[ctx->formatBufferIdx] = '\0';
      const char* colorCode                   = ctx->formatBuffer;

       ASSERT_MSG(ctx->formatBufferIdx == 8,
                 "format syntax error: invalid color code");
       const int r = (hex2dec(colorCode[0]) << 4u) + hex2dec(colorCode[1]);
       const int g = (hex2dec(colorCode[2]) << 4u) + hex2dec(colorCode[3]);
       const int b = (hex2dec(colorCode[4]) << 4u) + hex2dec(colorCode[5]);
       const int a = (hex2dec(colorCode[6]) << 4u) + hex2dec(colorCode[7]);
       ctx->colors[ctx->colorsSize][0] = (1.0f / 255) * r;
       ctx->colors[ctx->colorsSize][1] = (1.0f / 255) * g;
       ctx->colors[ctx->colorsSize][2] = (1.0f / 255) * b;
       ctx->colors[ctx->colorsSize][3] = (1.0f / 255) * a;

      //#ifdef NDEBUG
      //      bool isColorFound = false;
      //#endif // NDEBUG
      //      for (unsigned i = 0; i < TEXT_FORMAT_COLOR_NAMES_SIZE; ++i)
      //      {
      //        if (strcmp(ctx->formatBuffer, TextFormatColorNames[i]) == 0)
      //        {
      //          ctx->colors[ctx->colorsSize][0] = TextFormatColors[i][0];
      //          ctx->colors[ctx->colorsSize][1] = TextFormatColors[i][1];
      //          ctx->colors[ctx->colorsSize][2] = TextFormatColors[i][2];
      //          ctx->colors[ctx->colorsSize][3] = TextFormatColors[i][3];
      //#ifdef NDEBUG
      //          isColorFound = true;
      //#endif // NDEBUG
      //          break;
      //        };
      //      }
      //#ifdef NDEBUG
      //            ASSERT_MSG(isColorFound, "format syntax error: not found
      //            color");
      //#endif // NDEBUG

      ++ctx->colorsSize;
      ctx->formatBufferIdx         = 0;
      ctx->lastToken               = TFTChangeColor;
      ctx->units[ctx->unitsSize++] = TFTChangeColor;
      return 0;
    }
    else
    {
      ctx->formatBuffer[ctx->formatBufferIdx++] = (char)format;
      return TFTChangeColorStart;
    }
  }
  else
  {
    // escape start
    if (format == '\\')
    {
      ctx->lastToken = TFTEscape;
      return TFTEscape;
    }
    // code point token
    else
    {
      ctx->codePoints[ctx->codePointsSize++] = format;
      ctx->units[ctx->unitsSize++]           = TFTCodePoint;
      return 0;
    }
  }
}

int
text_format(const char* format, size_t formatSize, TextFormatContext* ctx)
{
  //va_list args;
  //va_start(args, ctx);
  //va_end(args);
  for (unsigned i = 0; i < (unsigned)formatSize; ++i)
  {
    text_format_next((u32)format[i], ctx);
  }
  return 0;
}
