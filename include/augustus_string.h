#ifndef AUGUSTUS_STRING_H
#define AUGUSTUS_STRING_H

#include "augustus_common.h"

typedef struct {
    const char* raw;
    usize len;
} String;

#define STR(x) ((String) { x, strlen(x) })

String String_concat(String lhs, String rhs);

#endif//AUGUSTUS_STRING_H
