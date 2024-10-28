#include "lineDecorator.h"
#include <string.h>
#include <stdlib.h>
#include <regex.h>

#define NO_MATCH -2
#define UNKNOWN_MATCH -1

typedef struct t_match_ret {
    int pos;
    uint len;
} match_ret;

typedef struct t_kw_internal_base {
    char pattern[MAX_KW_PATTERN_LEN];
    uint_8 len;
    uint_8 style;
    uint_8 matchTimes;
    uint_8 mask;
    match_ret tmatch;
    match_ret (*func)(uint, ulong*, char*, void*);
} keyword_internal_base;

typedef struct t_kw_internal {
    char pattern[MAX_KW_PATTERN_LEN];
    uint_8 len;
    uint_8 style;
    uint_8 matchTimes;
    uint_8 mask;
    match_ret tmatch;
    match_ret (*func)(uint, ulong*, char*, void*);
    long m;
} keyword_internal;

typedef struct t_kw_internal_reg {
    char pattern[MAX_KW_PATTERN_LEN];
    uint_8 len;
    uint_8 style;
    uint_8 matchTimes;
    uint_8 mask;
    match_ret tmatch;
    match_ret (*func)(uint, ulong*, char*, void*);
    regex_t regex; 
} keyword_internal_reg;

static const char styles[MAX_STYLE_INDEX + 2][12] = {
"\033[31m", "\033[32m", "\033[33m", "\033[34m",
"\033[35m", "\033[36m", "\033[37;41m", "\033[37;42m",
"\033[37;43m", "\033[37;44m", "\033[37;45m", "\033[37;46m", "\033[0m"};

static const match_ret no_match = {NO_MATCH,0};

static const uint_8 stylelen[MAX_STYLE_INDEX + 2] = {5, 5, 5, 5, 5, 5, 8, 8, 8, 8, 8, 8, 4};

static ulong temp_m_array[MAX_LINE_LEN + 1];

static inline boolean strCompare(char* one, char* another, uint_8 len) {
    for (uint i = 0; i < len; i++) {
        if (*(one++) != *(another++)) {
            return false;
        }
    }

    return true;
}

static inline match_ret matchKw(uint remainlength, ulong* pm, char* pin, void* pkw) {
    keyword_internal* kw = (keyword_internal*)pkw;
    uint cursor = 0;
    uint kwlen = kw->len;
    ulong m = kw->m;
    match_ret ret;

    while (true) {
        if (remainlength < kwlen) {
            return no_match;
        }

        // feature value matches
        if (*(pm + kwlen) - *pm == m) {
            if (strCompare(pin, kw->pattern, kwlen)) {
                ret.pos = cursor;
                ret.len = kw->len;
                return ret;
            }
        }

        remainlength--;
        cursor++;
        pm++;
        pin++;
    }

    return no_match;
}

static inline match_ret matchWithReg(uint len, ulong* pm, char* pin, void* pkw) {
    keyword_internal_reg* kw = (keyword_internal_reg*)pkw; 
    regmatch_t matches[1];
    match_ret match_ret;
    int ret = regexec(&kw->regex, pin, 1, matches, 0);

    if (ret == REG_NOMATCH) {  
        return no_match;
    }

    if (matches[0].rm_so != -1) {
        match_ret.len = matches[0].rm_eo - matches[0].rm_so;
        match_ret.pos = matches[0].rm_so;

        return match_ret;
    }

    return no_match;
}

boolean checkKeywordMask(keyword* pkw, KW_CHECK_POINT point) {
    return (((keyword_internal_base*)pkw)->mask & (1 << point)) ? 1 : 0;
}

static keyword_internal* generateNormalKeyword(const char* pattern) {
    keyword_internal* kw = malloc(sizeof(keyword_internal));

    kw->m = 0;
    kw->len = 0;
    uint_8 i = 0;
    while (pattern[i] != '\0' && pattern[i] != '\n') {
        kw->pattern[i] = pattern[i];
        kw->m = kw->m + pattern[i];
        kw->len = kw->len + 1;
        i++;
    }
    kw->pattern[i] = '\0';
    kw->func = (kw->len > 0) ? matchKw : null;
    return kw;
}

static keyword_internal_reg* generateRegKeyword(const char* pattern) {
    char message[100];
    keyword_internal_reg* kw = malloc(sizeof(keyword_internal_reg));

    kw->len = strlen(pattern);
    strcpy(kw->pattern, pattern);

    // Compile regular expressions.
    int ret = regcomp(&kw->regex, pattern, REG_EXTENDED);

    if (ret != 0) {
        regerror(ret, &kw->regex, message, sizeof(message));
        kw->func = null;
    } else {
        kw->func = matchWithReg;
    }

    return kw;
}

keyword* generateKeyword(const char* pattern, boolean positive, boolean regular, uint_8 style) {
    keyword_internal_base* kw = regular ? (keyword_internal_base*)generateRegKeyword(pattern) :
                                          (keyword_internal_base*)generateNormalKeyword(pattern);

    kw->style = style;
    kw->mask = 0;
    kw->mask |= (positive << KW_CHECK_IS_POSITIVE);
    kw->mask |= (regular << KW_CHECK_IS_REG);

    return (keyword*)kw;
}

void releaseKeyword(keyword* keyword) {
    if (checkKeywordMask(keyword, KW_CHECK_IS_REG)) {
        regfree(&((keyword_internal_reg*)keyword)->regex);
    }
    free(keyword);
}

boolean decorateStringLineMatchAllKeywords(keyword** pkws, uint_8 nkw, int len, char* in, segHandler handler) {
    uint cursor = 0;
    char ch = in[0];

    temp_m_array[0] = 0;

    // gene m array
    while (true) {
        ch = in[cursor];
        if (ch == '\0' || ch == '\n') {
            break;
        }
        // A(n+1) = A(n) + a(n)
        temp_m_array[cursor + 1] = temp_m_array[cursor] + ch;

        cursor++;
    }

    for (uint_8 i = 0; i < nkw; i++) {
        ((keyword_internal_base*)pkws[i])->tmatch.pos = UNKNOWN_MATCH;
        ((keyword_internal_base*)pkws[i])->tmatch.len = 0;
        ((keyword_internal_base*)pkws[i])->matchTimes = 0; 
    }

    // match all
    int currentMatchStartPos = 0;
    match_ret minMatchPos;
    keyword_internal_base* minMatchKw;
    keyword_internal_base* pkw;
    boolean hasMatch = false;

    while (currentMatchStartPos < len) {
        minMatchKw = null;
        minMatchPos.pos = 99999;
        for (uint_8 i = 0; i < nkw; i++) {
            pkw = (keyword_internal_base*)pkws[i];

            if (pkw->tmatch.pos >= currentMatchStartPos) { // A matching position already exists
                if (pkw->tmatch.pos < minMatchPos.pos) {
                    minMatchPos.pos = pkw->tmatch.pos;
                    minMatchPos.len = pkw->tmatch.len;
                    minMatchKw = pkw;
                }
            } else if (pkw->tmatch.pos > NO_MATCH) { // Try to match a position 
                pkw->tmatch = pkw->func == null ? no_match : pkw->func(len - currentMatchStartPos, temp_m_array + currentMatchStartPos, in + currentMatchStartPos, pkw);

                // When there are a match position, compare with other match position.
                if (pkw->tmatch.pos != NO_MATCH) {
                    // Apply offset.
                    pkw->tmatch.pos += currentMatchStartPos;
                    if (pkw->tmatch.pos < minMatchPos.pos) {
                        minMatchPos.pos = pkw->tmatch.pos;
                        minMatchPos.len = pkw->tmatch.len;
                        minMatchKw = pkw;
                    } 
                } 
            } // else We already know that there is no match in the entire string
        }
        if (minMatchKw == null) { // has no match position.
            goto NO_MORE_MATH;
        } else {
            hasMatch = true;
            minMatchKw->matchTimes++;
        
            // [has more segments], [segment length], [segment]
            if (minMatchPos.pos > 0) {
                handler(true, minMatchPos.pos - currentMatchStartPos, in + currentMatchStartPos);
            }
            handler(true, stylelen[minMatchKw->style], (char*)styles[minMatchKw->style]);
            handler(true, minMatchPos.len, in + minMatchPos.pos);
            handler(true, stylelen[MAX_STYLE_INDEX + 1], (char*)styles[MAX_STYLE_INDEX + 1]);

            currentMatchStartPos = minMatchPos.pos + minMatchPos.len;
        }
    }

NO_MORE_MATH:
    if (hasMatch) {
        handler(false, len - currentMatchStartPos, in + currentMatchStartPos); 
        return true;
    } else {
        handler(false, len, in);
        return false;
    }
}