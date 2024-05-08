#pragma once

#include <exception>
#include <stdexcept>

void Assert(const bool condition, const char *message)
{
    if (!condition)
        throw std::runtime_error(message);
}