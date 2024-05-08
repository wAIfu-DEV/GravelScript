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
        {"call", Token::KEYW_CALL},
        {"class", Token::KEYW_CLASS},
        {"func", Token::KEYW_FUNC},
        {"namespace", Token::KEYW_NAMESPACE},
        {"import", Token::KEYW_IMPORT},
        {"ret", Token::KEYW_RETURN},
        {"end", Token::KEYW_END},
    };
}