#pragma once

#include <iostream>
#include <string>

#include "../types/token.hpp"
#include "../types/variant.hpp"
#include "../memory/memory.hpp"

#include "../logger/logger.hpp"
#include "../helper/helper.hpp"

Error MakeVariant(Variant &var, Token::Token &val, bool make_const = false)
{
    var.flags.is_const = make_const;

    switch (val.type)
    {
    case Token::STRING:
    {
        var.d64 = Memory::strings.size();
        Memory::strings.push_back(val.content);
        var.type = VALUE_TYPE::STRING;
        return Error::OK;
    }
    case Token::NUMBER:
    {
        bool is_float = Helper::StringContains(val.content, '.');
        var.d64 = is_float
                      ? std::bit_cast<uint64_t>(std::stod(val.content))
                      : std::bit_cast<uint64_t>(std::stoll(val.content));
        var.type = is_float
                       ? VALUE_TYPE::FLOAT
                       : VALUE_TYPE::INT;
        return Error::OK;
    }
    default:
        var.d64 = 0;
        var.type = VALUE_TYPE::NIL;
        return Error::REJECTED;
    }
}

Error MakeVarArray(Variant &var, std::vector<Token::Token> &values)
{
    var.type = VALUE_TYPE::ARRAY;
    var.flags = {};

    VarArray result{};

    for (Token::Token &val : values)
    {
        Variant v{};
        Error make_err = MakeVariant(v, val);
        if (make_err)
            return Error::REJECTED;
        result.push_back(v);
    }

    var.d64 = Memory::arrays.size();
    Memory::arrays.push_back(result);
    return Error::OK;
}