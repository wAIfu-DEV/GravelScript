#pragma once

#include <iostream>
#include <unordered_map>

#include "../logger/logger.hpp"
#include "../help/help.hpp"

#include "../types/error.hpp"
#include "../types/token.hpp"
#include "../types/variant.hpp"

namespace Parser
{
    Error HandleInstruction(const std::vector<Token::Token> &tokens, std::vector<Scope *> &scope_stack)
    {
        if (!tokens.size())
            return Error::OK;

        const Token::Token &inst_tok = tokens.at(0);
        Token::TYPE inst_type = inst_tok.type;
        size_t inst_size = tokens.size();

        switch (inst_type)
        {
        case Token::KEYW_SET:
        {
            if (inst_size < 3)
            {
                Logger::Error("Syntax Error: 'set' instruction requires 2 arguments.", {});
                return Error::SYNTAX;
            }
            Instruction inst = {
                .type = Token::KEYW_SET,
                .args = {},
            };
            for (const Token::Token &tok : tokens)
                inst.args.push_back(tok);
            scope_stack.back()->instructions.push_back(inst);
            break;
        }
        case Token::KEYW_CALL:
        {
            if (inst_size < 2)
            {
                Logger::Error("Syntax Error: 'call' instruction requires at least 1 argument.", {});
                return Error::SYNTAX;
            }
            Instruction inst = {
                .type = Token::KEYW_CALL,
                .args = {},
            };
            for (const Token::Token &tok : tokens)
                inst.args.push_back(tok);
            scope_stack.back()->instructions.push_back(inst);
            break;
        }
        case Token::KEYW_RETURN:
        {
            if (scope_stack.back()->type != SCOPE_TYPE::FUNC)
            {
                Logger::Error("Syntax Error: cannot return outside of a function.", {});
                return Error::SYNTAX;
            }
            Instruction inst = {
                .type = Token::KEYW_RETURN,
                .args = {},
            };
            for (const Token::Token &tok : tokens)
                inst.args.push_back(tok);
            scope_stack.back()->instructions.push_back(inst);
            break;
        }
        case Token::KEYW_END:
        {
            if (scope_stack.back()->type == SCOPE_TYPE::GLOBAL)
            {
                Logger::Error("Syntax Error: cannot use the 'end' keyword while in global scope.", {});
                return Error::SYNTAX;
            }
            scope_stack.pop_back();
            break;
        }
        case Token::KEYW_CLASS:
        {
            if (inst_size < 2)
            {
                Logger::Error("Syntax Error: 'class' instruction requires at least 1 argument.", {});
                return Error::SYNTAX;
            }
            if (Helper::UnorderedMapHasKey(scope_stack.back()->scopes, tokens.at(1).content))
            {
                Logger::Error("Syntax Error: member", {tokens.at(1).content, "already exists in scope."});
                return Error::SYNTAX;
            }
            scope_stack.back()
                ->scopes
                .insert({tokens.at(1).content, Scope{
                                                   .type = SCOPE_TYPE::CLASS,
                                                   .name = tokens.at(1).content,
                                                   .args = {},
                                                   .vars = {},
                                                   .instructions = {},
                                               }});
            scope_stack.push_back(&scope_stack.back()->scopes.at(tokens.at(1).content));
            break;
        }
        case Token::KEYW_NAMESPACE:
        {
            if (inst_size < 2)
            {
                Logger::Error("Syntax Error: 'namespace' instruction requires at least 1 argument.", {});
                return Error::SYNTAX;
            }
            if (Helper::UnorderedMapHasKey(scope_stack.back()->scopes, tokens.at(1).content))
            {
                Logger::Error("Syntax Error: member", {tokens.at(1).content, "already exists in scope."});
                return Error::SYNTAX;
            }
            scope_stack.back()
                ->scopes
                .insert({tokens.at(1).content, Scope{
                                                   .type = SCOPE_TYPE::NAMESPACE,
                                                   .name = tokens.at(1).content,
                                                   .args = {},
                                                   .vars = {},
                                                   .instructions = {},
                                               }});
            scope_stack.push_back(&scope_stack.back()->scopes.at(tokens.at(1).content));
            break;
        }
        case Token::KEYW_FUNC:
        {
            if (inst_size < 2)
            {
                Logger::Error("Syntax Error: 'class' instruction requires at least 1 argument.", {});
                return Error::SYNTAX;
            }
            if (Helper::UnorderedMapHasKey(scope_stack.back()->scopes, tokens.at(1).content))
            {
                Logger::Error("Syntax Error: member", {tokens.at(1).content, "already exists in scope."});
                return Error::SYNTAX;
            }
            scope_stack
                .back()
                ->scopes
                .insert({tokens.at(1).content,
                         Scope{
                             .type = SCOPE_TYPE::FUNC,
                             .name = tokens.at(1).content,
                             .args = {},
                             .vars = {},
                             .instructions = {},
                         }});

            scope_stack.push_back(&scope_stack.back()->scopes.at(tokens.at(1).content));

            for (size_t i = 2; i < inst_size; ++i)
            {
                if (tokens.at(i).type != Token::NAME)
                    continue;
                scope_stack.back()
                    ->args.push_back({tokens.at(i).content, Variant{
                                                                .type = VALUE_TYPE::NIL,
                                                                .d64 = 0,
                                                            }});
            }
            break;
        }
        case Token::KEYW_IMPORT:
        {
            if (inst_size < 3)
            {
                Logger::Error("Syntax Error: 'import' instruction requires 2 arguments 'path' and 'alias'.", {});
                return Error::SYNTAX;
            }
            if (scope_stack.back()->type != SCOPE_TYPE::GLOBAL)
            {
                Logger::Error("Syntax Error: cannot import outside of the global scope.", {});
                return Error::SYNTAX;
            }
            Instruction inst = {
                .type = Token::KEYW_IMPORT,
                .args = {},
            };
            for (const Token::Token &tok : tokens)
                inst.args.push_back(tok);
            scope_stack.back()->instructions.push_back(inst);
            break;
        }
        default:
            break;
        }
        return Error::OK;
    }

    Error ParseTokens(const std::vector<Token::Token> &tokens, Scope &out_global)
    {
        Logger::Debug("Starting token parsing.", {});

        std::vector<Token::Token> inst_buffer = {};

        std::vector<Scope *> scope_stack = {};
        scope_stack.push_back(&out_global);

        for (Token::Token tok : tokens)
        {
            if (tok.type != Token::SEMI_COLON)
            {
                inst_buffer.push_back(tok);
            }
            else
            {
                Error handle_err = HandleInstruction(inst_buffer, scope_stack);
                if (handle_err)
                    return handle_err;
                inst_buffer.clear();
            }
        }

        if (scope_stack.back()->type != SCOPE_TYPE::GLOBAL)
        {
            Logger::Error("Syntax Error: a scope was not correctly ended.", {});
            return Error::SYNTAX;
        }

        Logger::Debug("Finished token parsing.", {});

        return Error::OK;
    }
}