#include "augustus_hash_functions.h"

Hash hash_string(const char* str) {
    u32 b = 378551;
    u32 a = 63689;

    Hash hash = 0;

    usize len = strlen(str);

    for(i32 i = 0; i < len; i++) {
        char c = str[i];
        hash = hash * a + c;
        a = a * b;
    }

    return hash;
}
