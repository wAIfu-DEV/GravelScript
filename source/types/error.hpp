#pragma once

#include <cstdint>

enum Error
{
    OK = 0,
    UNHANDLED = 1,
    REJECTED = 2,
    FORBIDDEN = 3,
    SYNTAX = 4,
    ASSERTION = 5,
    EARLY_RETURN = 6,
    SKIP_TO_IF = 7,
    EXE_UPTO_IF = 8,
    SKIP_TO_END = 9,
};