#pragma once

#include <iostream>
#include <map>

#include "../types/token.hpp"

namespace GravelRules
{
    const std::map<std::string, Token::TYPE> TOK_LOOKUP_TABLE{
        /* single char tokens */
        {".", Token::DOT},
        {",", Token::COMMA},
        {";", Token::SEMI_COLON},
        /* reserved word */
        {"set", Token::KEYW_SET},
        {"const", Token::KEYW_CONST},
        {"var", Token::KEYW_VAR},
        {"array", Token::KEYW_ARRAY},
        {"call", Token::KEYW_CALL},
        {"struct", Token::KEYW_STRUCT},
        {"func", Token::KEYW_FUNC},
        {"namespace", Token::KEYW_NAMESPACE},
        {"import", Token::KEYW_IMPORT},
        {"return", Token::KEYW_RETURN},
        {"end", Token::KEYW_END},
        {"if", Token::KEYW_IF},
        {"elif", Token::KEYW_ELIF},
        {"else", Token::KEYW_ELSE},
        {"endif", Token::KEYW_ENDIF},
        {"fetch", Token::KEYW_FETCH},
    };
}