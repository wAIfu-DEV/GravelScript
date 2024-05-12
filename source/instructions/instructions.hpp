#pragma once

#include <iostream>

#include "../types/error.hpp"
#include "../types/variant.hpp"

#include "../memory/memory.hpp"
#include "../make_variant/make_variant.hpp"
#include "../helper/helper.hpp"

namespace Instructions
{

    Error Set(Variant &var, Token::Token &value, bool create_as_const)
    {
        if (var.flags.is_const)
        {
            Logger::Error("Tried setting value of constant variable with", {value.content});
            return Error::REJECTED;
        }

        Error var_err = MakeVariant(var, {value}, create_as_const);
        if (var_err)
        {
            Logger::Error("Syntax Error: expected value as second argument of 'set', got:", {value.content});
            return Error::SYNTAX;
        }
        return Error::OK;
    }
}
