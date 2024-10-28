#include "stdio.h"
#include <string.h>
#include <unistd.h>
#include "lineDecorator.h"

#define MAX_N_KEYWORD 16
#define MAX_SEG 64
#define NOT_SET 2
#define IS_KW_SATISFIED(kw) (kw)->matchTimes > 0 == checkKeywordMask(kw, KW_CHECK_IS_POSITIVE)
#define IS_KW_NOT_SATISFIED(kw) (kw)->matchTimes > 0 != checkKeywordMask(kw, KW_CHECK_IS_POSITIVE)

boolean hasFileName;
char fileName[512];
char filelinePrefix[] = "#{file]>>";
keyword* keywords[MAX_N_KEYWORD];
uint_8 nkeywords;
uint_8 currentStyleIndex;
boolean isSatisfyAllKeywords; // or satisfy any keyword.

typedef struct t_segment {
    uint len;
    char* p;
} segment;

segment segments[MAX_SEG];
uint_8 currentSegIndex;

void handleSeg(boolean hasMore, uint len, char* seg) {
    segments[currentSegIndex].p = seg;
    segments[currentSegIndex].len = len;
    currentSegIndex++;
}

void pprintSeg() {
    for (uint i = 0; i < currentSegIndex; i++) {
        printf("%.*s", segments[i].len, segments[i].p);
    }
}

keyword* parseKeyword(char* arg) {
    char ch = arg[0];
    uint_8 offset = 0;
    uint_8 isNegative = NOT_SET;
    uint_8 isRegex = NOT_SET;
    boolean hasValidPrefix = false;

    while(ch == '\\') {
        offset++;
        ch = arg[offset];
    }

    if (ch == '-') {
        offset++;
        while (true) {
            ch = arg[offset++];
            if (ch == 'n') {
                if (isNegative == NOT_SET) {
                    isNegative = true;
                } else {
                    //Invalid prefix
                    goto NO_VALID_PREFIX; 
                }
            } else if (ch == 'r') {
                if (isRegex == NOT_SET) {
                    isRegex = true;
                } else {
                    goto NO_VALID_PREFIX;
                }
            } else if (ch == '.') {
                hasValidPrefix = true;
                goto GEN_KW;
            } else { // end of string or other characters.
                //Invalid prefix
                goto NO_VALID_PREFIX;
            }
        }
    }

NO_VALID_PREFIX:
    hasValidPrefix = false;
    isRegex = false;
    isNegative = false;
    offset = 0;

GEN_KW:
    if (arg[0] == '\\') {
        isRegex = false;
        isNegative = false;
        if (hasValidPrefix) {
            offset = 1;
        } else {
            offset = 0;
        }
    }

    if (arg[offset] == '\0' || arg[offset] == '\n') {
        return null;
    }

    isNegative = isNegative == NOT_SET ? false : isNegative;
    isRegex = isRegex == NOT_SET ? false : isRegex;
    
    uint_8 style = isNegative ? 0 : currentSegIndex++;

    if (currentSegIndex > MAX_STYLE_INDEX) {
        currentSegIndex = 0;
    }

    return generateKeyword(arg + offset, !isNegative, isRegex, style); 
}

void parseArgs(int argc, char** argv) {
    isSatisfyAllKeywords = false;
    nkeywords = 0;
    currentStyleIndex = 0;
    keyword* kw;
    char c;
    uint_8 len;
    for (uint i = 1; i < argc; i++) {
        c = argv[i][0];
        len = strlen(argv[i]);
        if (len == 2 && argv[i][0] == '-' && argv[i][1] == 'a') {
            isSatisfyAllKeywords = true; 
        } else {
            kw = parseKeyword(argv[i]);
            if (kw && nkeywords < MAX_N_KEYWORD) {
                keywords[nkeywords] = kw; 
                nkeywords++;
            }
        }
    }
}

boolean shouldPringSeg() {
    if (isSatisfyAllKeywords) {
        for (int i = 0; i < nkeywords; i++) {
            if (IS_KW_NOT_SATISFIED(keywords[i])) {
                return false;
            }
        } 
        return true;
    } else { // any
        for (int i = 0; i < nkeywords; i++) {
            if (IS_KW_SATISFIED(keywords[i])) {
                return true;
            }
        }
        return false;
    }
}

boolean isFileNameLine(char* src) {
    for (uint i = 0; i < 9; i++) {
        if (src[i] != filelinePrefix[i]) {
            return false;
        }
    }
    return true;
}

int main(int argc, char** argv) {
    parseArgs(argc, argv);

    if (isatty(STDIN_FILENO)) {
        // Input from terminal
    } else {
        // Input from pip
        char buffer[MAX_LINE_LEN];
        while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (nkeywords == 0) {  
                printf("%s", buffer);
            } else {
                if (isFileNameLine(buffer)) {
                    strcpy(fileName, buffer);
                    hasFileName = true;
                } else {
                    currentSegIndex = 0;
                    decorateStringLineMatchAllKeywords(keywords, nkeywords, strlen(buffer), buffer, handleSeg);
                    if (shouldPringSeg()) {
                        if (hasFileName) {
                            printf("%s", fileName);
                            hasFileName = false;
                        }
                        pprintSeg();
                    }
                }
            }
        }
    }

    for (int i = 0; i < nkeywords; i++) { 
        releaseKeyword(keywords[i]);
    }

    return 0;
}