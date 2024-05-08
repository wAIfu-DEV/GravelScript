#pragma once

#include <iostream>
#include <vector>

#include "../types/error.hpp"
#include "../types/token.hpp"
#include "../types/variant.hpp"
#include "../types/scope.hpp"

#include "lexer.hpp"
#include "../parser/parser.hpp"
#include "../interpreter/interpreter.hpp"

namespace Script
{
    Error RunFile(const std::string &script_path)
    {
        std::vector<Token::Token> tokens{};

        Error lex_err = Script::LexFile(script_path, tokens);
        if (lex_err)
            return lex_err;

#if !GVS_RELEASE
        Logger::Debug("Tokens size:", {std::to_string(tokens.size())});

        for (Token::Token t : tokens)
        {
            std::cout << t.content << ((t.type == Token::SEMI_COLON) ? "\n" : " ");
        }
        std::cout << "\n";
#endif

        Scope global{
            .type = SCOPE_TYPE::GLOBAL,
            .args = {},
            .vars = {},
            .scopes = {},
        };

        Error parse_err = Parser::ParseTokens(tokens, global);
        if (parse_err)
            return parse_err;

        Error interpret_err = Interpreter::InterpretGlobalScope(global);
        if (interpret_err)
            return interpret_err;

        return Error::OK;
    }
}