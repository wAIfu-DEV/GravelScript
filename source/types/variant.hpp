#pragma once

#include <cstdint>
#include <iostream>
#include <variant>

#include "../types/error.hpp"
#include "../memory/memory.hpp"

enum class VALUE_TYPE : uint8_t
{
    NIL,
    INT,
    FLOAT,
    STRING,
    ARRAY,
    MAP,
};

struct Variant
{
    VALUE_TYPE type;
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

Error MakeVariant(Variant &var, const Token::Token &value)
{
    switch (value.type)
    {
    case Token::STRING:
    {
        var.d64 = Memory::strings.size();
        Memory::strings.push_back(value.content);
        var.type = VALUE_TYPE::STRING;
        return Error::OK;
    }
    case Token::NUMBER:
    {
        if (Helper::StringContains(value.content, '.'))
        {
            var.d64 = std::bit_cast<uint64_t>(std::stod(value.content));
            var.type = VALUE_TYPE::FLOAT;
        }
        else
        {
            var.d64 = std::bit_cast<uint64_t>(std::stoll(value.content));
            var.type = VALUE_TYPE::INT;
        }
        return Error::OK;
    }
    default:
    {
        var.d64 = 0;
        var.type = VALUE_TYPE::NIL;
        return Error::REJECTED;
    }
    }
}