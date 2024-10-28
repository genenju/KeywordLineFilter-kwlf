#pragma once

#include "base/utils.h"

#define MAX_KW_PATTERN_LEN 128
#define MAX_LINE_LEN 2048
#define MAX_STYLE_INDEX 11
#define KW_CHECK_IS_POSITIVE 0
#define KW_CHECK_IS_REG 1
#define KW_CHECK_POINT uint_8

typedef struct t_kw {
    char pattern[MAX_KW_PATTERN_LEN];
    uint_8 len;
    uint_8 style;
    uint_8 matchTimes;
} keyword;

// [has more segments], [segment length], [segment]
typedef void (*segHandler)(boolean, uint, char*);

/**
 * '\d' is not supported which should replaced with [0-9].
 * 
*/
keyword* generateKeyword(const char* pattern, boolean positive, boolean regular, uint_8 style);

void releaseKeyword(keyword* keyword);

boolean checkKeywordMask(keyword* pkw, KW_CHECK_POINT);

void initRegularKeyword(const char* reg, boolean positive, keyword* pkw, uint_8 stypeIndex);

boolean decorateStringLineMatchAllKeywords(keyword** pkws, uint_8 nkw, int len, char* in, segHandler handler);