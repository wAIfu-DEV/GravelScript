#pragma once

#include <iostream>

#include "../types/error.hpp"
#include "../types/variant.hpp"

#include "../memory/memory.hpp"
#include "../helper/helper.hpp"

namespace Instructions
{

    Error Set(Variant &var, Token::Token &value)
    {
        Error var_err = MakeVariant(var, value);
        if (var_err)
        {
            Logger::Error("Syntax Error: expected value as second argument of 'set', got:", {value.content});
            return Error::SYNTAX;
        }
        return Error::OK;
    }
}
