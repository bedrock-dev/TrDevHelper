//
// Created by xhy on 2021/9/3.
//

#include "tools.h"
#include <cstdarg>
#include <cstdio>

namespace {
    bool DEBUG = false;

}

void enableDebug() {
    DEBUG = true;
}

void info(const char *fmt, ...) {
    if (!DEBUG)return;
    va_list args;
            va_start(args, fmt);
    vprintf(fmt, args);
            va_end(args);
}
