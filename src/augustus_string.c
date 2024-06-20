#include "augustus_string.h"
#include <stdlib.h>
#include <string.h>

String String_concat(String lhs, String rhs) {
    usize len = lhs.len + rhs.len;
    char* raw = malloc(len + 1);

    memcpy(raw, lhs.raw, lhs.len);
    memcpy(raw + lhs.len, rhs.raw, rhs.len);

    raw[len] = '\0';

    return (String) { raw, len };
}
