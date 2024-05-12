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
};