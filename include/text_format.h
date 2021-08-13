#ifndef TEXT_FORMAT
#define TEXT_FORMAT

#include "cglm/types.h"
#include "toolbox/common.h"

typedef enum TextOverflow
{
  TextOverFlowInitial,
  TextOverFlowHidden,
  TextOverFlowVisible,
  TextOverflow_CNT
} TextOverflow;

typedef enum TextWordWarp
{
  TextWordWarpInitial,
  TextWordWarpNormal,
  TextWordWarpBreakWord,
  TextWordWarp_CNT
} TextWordWarp;

typedef enum TextAlignment
{
  TextAlignmentLeft,
  TextAlignmentRight,
  TextAlignmentCenter,
  TextAlignment_CNT
} TextAlignment;

typedef struct ParagraphFormat
{
  pt            before;
  pt            after;
  pt            leftIndent;
  pt            rightIndent;
  pt            firstIndent;
  pt            hangingIndent;
  TextOverflow  overflow;
  TextWordWarp  wordWrap;
  TextAlignment alignment;
  float         lineSpacingScale;
} ParagraphFormat;

typedef enum TextFormatToken
{
  TFTCodePoint,
  TFTEscape,
  TFTChangeColorStart,
  TFTChangeColor,
  TFTNewLine,
  TFTPopInteger,
  TFTPopFloat,
  TFTPopString,
  TFT_CNT
} TextFormatToken;

typedef struct TextFormatContext
{
  TextFormatToken lastToken;

  TextFormatToken units[1024];
  int             unitsSize;
  int             unitsCapacity;

  vec4 defaultColor;

  vec4 colors[24];
  int  colorsSize;
  int  colorCapacity;

  u32 codePoints[1024];
  int codePointsSize;
  int codePointsCapacity;

  char formatBuffer[12];
  int  formatBufferIdx;
} TextFormatContext;

void text_format_context_init(TextFormatContext* ctx, const vec4 defaultColor);
void text_format_context_reset(TextFormatContext* ctx);

int
text_format(const char* format, size_t formatSize, TextFormatContext* ctx);

#endif // !TEXT_FORMAT
