#pragma once

#include <iostream>
#include <vector>
#include <fstream>

#include "../types/error.hpp"
#include "../types/token.hpp"
#include "rules.hpp"

class Lexer
{
public:
    enum LEX_MODE
    {
        DEFAULT,
        STRING,
        CHAR,
        NUMBER,
        COMMENT,
        ANNOTATION,
    };

    std::string file_path;
    std::ifstream file_stream;
    std::string line_str = "";
    size_t line_idx = 0;
    LEX_MODE lex_mode = LEX_MODE::DEFAULT;
    bool escape_next = false;
    bool has_errored = false;
    size_t char_index = 0;
    uint16_t col = 1;
    uint16_t line = 1;
    uint16_t tok_col = 1;
    uint16_t tok_line = 1;
    std::vector<char> tok_buff = {};
    std::vector<Token::Token> &tokens;

    Lexer(const std::string &path, std::vector<Token::Token> &toks) : tokens(toks)
    {
        file_path = path;
        file_stream = std::ifstream(file_path);
        ReadLineFromFile();
    }

    ~Lexer()
    {
        file_stream.close();
    }

    bool ReadLineFromFile()
    {
        std::string l;
        if (std::getline(file_stream, l))
        {
            line_str = l + "\n";
            line_idx = 0;
            return true;
        }
        return false;
    }

    char PeakChar()
    {
        return line_str[line_idx];
    }

    char ConsumeChar()
    {
        char c = line_str[line_idx++];
        if (line_idx >= line_str.size())
        {
            ReadLineFromFile();
        }
        return c;
    }

    bool IsEndOfFile()
    {
        if (line_idx <= line_str.size())
            return false;

        return !ReadLineFromFile();
    }

    void PushTokenBuffer(Token::TYPE type)
    {
        std::string s = std::string(tok_buff.begin(), tok_buff.end());

        if (s == "")
            return;

        Token::Token t = {
            .content = s,
            .type = type,
            .line = tok_line,
            .col = tok_col,
        };

        Logger::Debug("PUSHED:", {t.content, "\t:\t", Token::TYPE_TO_STR.at(t.type)});
        tokens.push_back(t);
        tok_buff.clear();
        tok_line = line;
        tok_col = col;
    }

    bool IsComment(const char c)
    {
        if (escape_next)
            return false;
        return (c == '/' && PeakChar() == '/');
    }

    bool IsCharQuote(char c)
    {
        if (escape_next)
            return false;
        return (c == '`');
    }

    bool IsStringQuote(char c)
    {
        if (escape_next)
            return false;
        return (c == '"' || c == '\'');
    }

    bool IsAnnotation(char c)
    {
        if (escape_next)
            return false;
        return c == ':';
    }

    void SetLexModeComment()
    {
        if (lex_mode != LEX_MODE::STRING && lex_mode != LEX_MODE::CHAR)
        {
            PushTokenBuffer(TryMatchTokenBuffer());
            lex_mode = LEX_MODE::COMMENT;
        }
    }

    void SetLexModeAnnotation()
    {
        if (lex_mode != LEX_MODE::STRING && lex_mode != LEX_MODE::CHAR)
        {
            PushTokenBuffer(TryMatchTokenBuffer());
            lex_mode = LEX_MODE::ANNOTATION;
        }
    }

    void ToggleLexModeChar()
    {
        if (lex_mode == LEX_MODE::CHAR)
        {
            PushTokenBuffer(Token::CHAR);
            lex_mode = LEX_MODE::DEFAULT;
        }
        else if (lex_mode != LEX_MODE::STRING)
        {
            PushTokenBuffer(TryMatchTokenBuffer());
            lex_mode = LEX_MODE::CHAR;
        }
    }

    void ToggleLexModeString()
    {
        if (lex_mode == LEX_MODE::STRING)
        {
            PushTokenBuffer(Token::STRING);
            lex_mode = LEX_MODE::DEFAULT;
        }
        else if (lex_mode != LEX_MODE::CHAR)
        {
            PushTokenBuffer(TryMatchTokenBuffer());
            lex_mode = LEX_MODE::STRING;
        }
    }

    bool isAlphaNumeric(char c)
    {
        return ((c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == 95 || c == 36);
    }

    bool isNumeric(char c)
    {
        return ((c >= 48 && c <= 57) || c == 46 || c == 45);
    }

    bool isWhiteSpace(char c)
    {
        return (c >= 1 && c <= 32);
    }

    bool InStringOrChar()
    {
        return (lex_mode == LEX_MODE::STRING || lex_mode == LEX_MODE::CHAR);
    }

    Token::TYPE TryMatchCharToken(char c)
    {
        std::string schar = std::string(1, c);
        // Check if is single char token or resered keyword
        if (Helper::MapHasKey(GravelRules::TOK_LOOKUP_TABLE, schar))
        {
            if (c == '.' && isAlphaNumeric(PeakChar()))
            {
                return Token::NONE;
            }

            return GravelRules::TOK_LOOKUP_TABLE.at(schar);
        }
        return Token::NONE;
    }

    Token::TYPE TryMatchTokenBuffer()
    {
        std::string tok = std::string(tok_buff.begin(), tok_buff.end());

        if (!tok.size())
            return Token::NONE;

        // Check if is single char token or resered keyword
        if (Helper::MapHasKey(GravelRules::TOK_LOOKUP_TABLE, tok))
        {
            return GravelRules::TOK_LOOKUP_TABLE.at(tok);
        }

        // Check if number
        for (size_t i = 0; i < tok.length(); ++i)
        {
            if (!isNumeric(tok[i]) && !(tok[i] == '.'))
                break;
            if (i == tok.length() - 1)
            {
                return Token::NUMBER;
            }
        }

        // Check if name
        for (size_t i = 0; i < tok.length(); ++i)
        {
            if (!isAlphaNumeric(tok[i]) && !(tok[i] == '.'))
                break;
            if (i == tok.length() - 1)
            {
                return Token::NAME;
            }
        }

        Logger::Error("Failed to recognize token:", {tok});
        return Token::NONE;
    }
};

namespace Script
{
    Error LexFile(const std::string &script_path, std::vector<Token::Token> &out_tokens)
    {
        Logger::Debug("Lexing file:", {script_path});
        Lexer lexer = Lexer(script_path, out_tokens);

        while (!lexer.IsEndOfFile())
        {
            char c = lexer.ConsumeChar();

            if (lexer.IsComment(c) && !lexer.InStringOrChar())
            {
                lexer.SetLexModeComment();
                continue;
            }

            if (lexer.lex_mode == lexer.COMMENT)
            {
                if (c == '\n')
                    lexer.lex_mode = lexer.DEFAULT;
                continue;
            }

            if (lexer.IsAnnotation(c) && !lexer.InStringOrChar())
            {
                lexer.SetLexModeAnnotation();
                continue;
            }

            if (lexer.lex_mode == lexer.ANNOTATION)
            {
                char next_c = lexer.PeakChar();
                if (next_c == ',' || next_c == ';')
                    lexer.lex_mode = lexer.DEFAULT;
                continue;
            }

            if (lexer.InStringOrChar() && c == '\\')
            {
                lexer.escape_next = true;
                continue;
            }

            if (lexer.IsCharQuote(c) && !lexer.escape_next)
            {
                lexer.ToggleLexModeChar();
                continue;
            }

            if (lexer.IsStringQuote(c) && !lexer.escape_next)
            {
                lexer.ToggleLexModeString();
                continue;
            }

            if (lexer.lex_mode == lexer.STRING)
            {
                lexer.tok_buff.push_back(c);
                lexer.escape_next = false;
                continue;
            }

            // Try matching single-char tokens
            Token::TYPE char_tok_type = lexer.TryMatchCharToken(c);
            if (char_tok_type != Token::NONE)
            {
                lexer.PushTokenBuffer(lexer.TryMatchTokenBuffer());
                lexer.tok_buff.push_back(c);
                lexer.PushTokenBuffer(char_tok_type);
                continue;
            }

            if (lexer.isWhiteSpace(c))
            {
                lexer.PushTokenBuffer(lexer.TryMatchTokenBuffer());
                continue;
            }
            lexer.tok_buff.push_back(c);
        }

        return Error::OK;
    }
}