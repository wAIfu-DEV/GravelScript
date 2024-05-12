#pragma once

#include <cstdint>
#include <iostream>
#include <variant>

#include "../types/error.hpp"

enum class VALUE_TYPE : uint8_t
{
    NIL,
    INT,
    FLOAT,
    STRING,
    ARRAY,
    MAP,
};

struct VariantFlags
{
    uint8_t is_const : 1;
};

struct Variant
{
    VALUE_TYPE type;
    VariantFlags flags;

    union
    {
        uint8_t d8;
        uint16_t d16;
        uint32_t d32;
        uint64_t d64;
    };
};

typedef std::nullptr_t VarNull;
typedef std::string VarString;
typedef std::vector<Variant> VarArray;
typedef std::unordered_map<std::string, Variant> VarMap;