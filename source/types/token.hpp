#pragma once

#include <iostream>
#include <map>

namespace Token
{
    enum TYPE
    {
        NONE = 0,
        /* one char */
        DOT = 1,
        COMMA = 2,
        SEMI_COLON = 3,
        /* multiple char */
        STRING = 100,
        CHAR = 101,
        NUMBER = 102,
        NAME = 103,
        /* reserved names */
        KEYW_SET = 200,
        KEYW_CALL = 201,
        KEYW_STRUCT = 202,
        KEYW_FUNC = 203,
        KEYW_IMPORT = 204,
        KEYW_RETURN = 205,
        KEYW_END = 206,
        KEYW_NAMESPACE = 207,
        KEYW_CONST = 208,
        KEYW_VAR = 209,
        KEYW_ARRAY = 210,
        KEYW_IF = 211,
        KEYW_ELIF = 212,
        KEYW_ELSE = 213,
        KEYW_ENDIF = 214,
        KEYW_FETCH = 215,
    };

    const std::map<TYPE, std::string> TYPE_TO_STR{
        {NONE, "none"},
        {DOT, "dot"},
        {COMMA, "comma"},
        {SEMI_COLON, "semi-colon"},

        {STRING, "string"},
        {CHAR, "char"},
        {NUMBER, "number"},
        {NAME, "name"},

        {KEYW_SET, "keyword-set"},
        {KEYW_CALL, "keyword-call"},
        {KEYW_STRUCT, "keyword-struct"},
        {KEYW_FUNC, "keyword-func"},
        {KEYW_FETCH, "keyword-func"},
        {KEYW_IMPORT, "keyword-import"},
        {KEYW_RETURN, "keyword-return"},
        {KEYW_END, "keyword-end"},
        {KEYW_NAMESPACE, "keyword-namespace"},
        {KEYW_CONST, "keyword-const"},
        {KEYW_VAR, "keyword-var"},
        {KEYW_ARRAY, "keyword-array"},
        {KEYW_IF, "keyword-if"},
        {KEYW_ELIF, "keyword-elif"},
        {KEYW_ELSE, "keyword-else"},
        {KEYW_ENDIF, "keyword-endif"},
    };

    struct Token
    {
        std::string content = "";
        TYPE type = TYPE::NONE;
        uint16_t line = 1;
        uint16_t col = 1;
    };
}