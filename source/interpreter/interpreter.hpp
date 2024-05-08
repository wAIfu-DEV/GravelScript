#pragma once

#include <iostream>

#include "../logger/logger.hpp"
#include "../helper/helper.hpp"

#include "../types/error.hpp"
#include "../types/token.hpp"
#include "../types/variant.hpp"
#include "../types/instructions.hpp"
#include "../types/scope.hpp"
#include "../instructions/instructions.hpp"
#include "../builtin/builtin_funcs.hpp"

namespace Interpreter
{
    Error ExecuteScope(Scope &scope, Scope &global_scope);

    Variant ResolveName(Token::Token varname, Scope &parent_scope, Scope &global_scope)
    {
        if (!Helper::StringContains(varname.content, '.'))
        {
            if (Helper::UnorderedMapHasKey(parent_scope.vars, varname.content))
            {
                return parent_scope.vars.at(varname.content);
            }
            else if (Helper::PairVectorHasKey(parent_scope.args, varname.content))
            {
                Variant *v = nullptr;
                Helper::PairVectorGet(parent_scope.args, varname.content, &v);
                return *v;
            }
            else if (Helper::UnorderedMapHasKey(global_scope.vars, varname.content))
            {
                return global_scope.vars.at(varname.content);
            }

            Variant v = {
                .type = VALUE_TYPE::NIL,
                .d64 = 0,
            };
            return v;
        }
        else
        {
            std::vector<std::string> scopes = Helper::SplitString(varname.content, '.');

            Scope *scope;
            if (Helper::UnorderedMapHasKey(parent_scope.scopes, scopes.at(0)))
            {
                scope = &parent_scope;
            }
            else if (Helper::UnorderedMapHasKey(global_scope.scopes, scopes.at(0)))
            {
                scope = &global_scope;
            }
            else
            {
                Logger::Error("Syntax Error: could not find scope", {scopes.at(0)});
                Variant v = {
                    .type = VALUE_TYPE::NIL,
                    .d64 = 0,
                };
                return v;
            }

            for (std::string scope_name : scopes)
            {
                if (Helper::UnorderedMapHasKey(scope->scopes, scope_name))
                {
                    scope = &scope->scopes.at(scope_name);
                    continue;
                }
                else if (Helper::UnorderedMapHasKey(scope->vars, scope_name))
                {
                    return scope->vars.at(scope_name);
                }
                else if (Helper::PairVectorHasKey(scope->args, scope_name))
                {
                    Variant *v = nullptr;
                    Helper::PairVectorGet(scope->args, scope_name, &v);
                    return *v;
                }

                Variant v = {
                    .type = VALUE_TYPE::NIL,
                    .d64 = 0,
                };
                return v;
            }
        }

        Variant v = {
            .type = VALUE_TYPE::NIL,
            .d64 = 0,
        };
        return v;
    }

    Error SetArgumentsBeforeCall(Scope &scope, std::vector<Token::Token> &args, Scope &parent_scope, Scope &global_scope)
    {
        for (Token::Token t : args)
        {
            Logger::Debug("ARG:", {t.content});
        }

        for (auto [key, val] : scope.args)
        {
            Logger::Debug("VARARG:", {key});
        }

        if (args.size() > scope.args.size())
        {
            Logger::Error("Syntax Error: too many arguments for call to function", {scope.name});
            return Error::SYNTAX;
        }

        if (args.size() < scope.args.size())
        {
            Logger::Error("Syntax Error: not enough arguments for call to function", {scope.name});
            return Error::SYNTAX;
        }

        size_t i = 0;
        for (auto [key, val] : scope.args)
        {
            Logger::Debug("ARGN:", {std::to_string(i), key, args[i].content});
            if (args[i].type == Token::NAME)
            {
                Variant v = ResolveName(args[i], parent_scope, global_scope);
                Logger::Debug("SET ARG WITH VALTYPE:", {std::to_string((uint8_t)v.type)});
                scope.args[i].second = v;
            }
            else
            {
                Variant *v = nullptr;
                Helper::PairVectorGet(scope.args, key, &v);
                Instructions::Set(*v, args[i]);
            }
            ++i;
        }
        return Error::OK;
    }

    Error ExecuteInstruction(Instruction &inst, Scope &parent_scope, Scope &global_scope)
    {
        switch (inst.type)
        {
        case Token::KEYW_SET:
        {
            Token::Token &varname = inst.args.at(1);
            Token::Token &value = inst.args.at(3);

            Logger::Debug("SET", {varname.content, value.content});

            if (!Helper::StringContains(varname.content, '.'))
            {
                if (Helper::UnorderedMapHasKey(parent_scope.vars, varname.content))
                {
                    Instructions::Set(parent_scope.vars.at(varname.content), value);
                    return Error::OK;
                }
                else if (Helper::PairVectorHasKey(parent_scope.args, varname.content))
                {
                    Variant *v = nullptr;
                    Helper::PairVectorGet(parent_scope.args, varname.content, &v);
                    Instructions::Set(*v, value);
                    return Error::OK;
                }
                else if (Helper::UnorderedMapHasKey(global_scope.vars, varname.content))
                {
                    Instructions::Set(global_scope.vars.at(varname.content), value);
                    return Error::OK;
                }

                Variant v = {
                    .type = VALUE_TYPE::NIL,
                    .d64 = 0,
                };
                Instructions::Set(v, value);
                parent_scope.vars.insert({varname.content, v});
                return Error::OK;
            }
            else
            {
                std::vector<std::string> scopes = Helper::SplitString(varname.content, '.');

                Scope *scope;
                if (Helper::UnorderedMapHasKey(parent_scope.scopes, scopes.at(0)))
                {
                    scope = &parent_scope;
                }
                else if (Helper::UnorderedMapHasKey(global_scope.scopes, scopes.at(0)))
                {
                    scope = &global_scope;
                }
                else
                {
                    Logger::Error("Syntax Error: could not find scope", {scopes.at(0)});
                    return Error::SYNTAX;
                }

                for (std::string scope_name : scopes)
                {
                    if (Helper::UnorderedMapHasKey(scope->scopes, scope_name))
                    {
                        scope = &scope->scopes.at(scope_name);
                        continue;
                    }
                    else if (Helper::UnorderedMapHasKey(scope->vars, scope_name))
                    {
                        Instructions::Set(scope->vars.at(scope_name), value);
                        return Error::OK;
                    }
                    else if (Helper::PairVectorHasKey(scope->args, scope_name))
                    {
                        Variant *v = nullptr;
                        Helper::PairVectorGet(scope->args, scope_name, &v);
                        Instructions::Set(*v, value);
                        return Error::OK;
                    }

                    Variant v = {
                        .type = VALUE_TYPE::NIL,
                        .d64 = 0,
                    };
                    Instructions::Set(v, value);
                    scope->vars.insert({scope_name, v});
                    return Error::OK;
                }
            }
        }
        case Token::KEYW_CALL:
        {
            Token::Token &funcname = inst.args.at(1);

            std::vector<Token::Token> args = {};
            for (size_t i = 3; i < inst.args.size(); ++i)
            {
                Logger::Debug("CALLARG:", {inst.args[i].content});
                if (inst.args[i].type != Token::COMMA)
                    args.push_back(inst.args[i]);
            }

            Logger::Debug("CALL", {funcname.content});

            if (!Helper::StringContains(funcname.content, '.'))
            {
                if (Helper::UnorderedMapHasKey(parent_scope.scopes, funcname.content))
                {
                    Scope &func = parent_scope.scopes.at(funcname.content);
                    Error arg_err = SetArgumentsBeforeCall(func, args, parent_scope, global_scope);
                    if (arg_err)
                        return arg_err;
                    Error exec_err = ExecuteScope(func, global_scope);
                    if (exec_err)
                        return exec_err;
                    return Error::OK;
                }
                else if (Helper::UnorderedMapHasKey(global_scope.scopes, funcname.content))
                {
                    Scope &func = global_scope.scopes.at(funcname.content);

                    if (func.type != SCOPE_TYPE::FUNC)
                    {
                        Logger::Error("Syntax Error: cannot use 'call' for a scope that isn't a function.", {});
                        return Error::SYNTAX;
                    }

                    Error arg_err = SetArgumentsBeforeCall(func, args, parent_scope, global_scope);
                    if (arg_err)
                        return arg_err;
                    Error exec_err = ExecuteScope(func, global_scope);
                    if (exec_err)
                        return exec_err;
                    return Error::OK;
                }
                else if (BuiltinFuncs::IsBuiltIn(funcname.content))
                {
                    std::vector<Variant> varargs = {};
                    for (Token::Token tok : args)
                    {
                        Variant v;
                        if (tok.type == Token::NAME)
                        {
                            v = ResolveName(tok, parent_scope, global_scope);
                        }
                        else
                        {
                            Error var_err = MakeVariant(v, tok);
                            if (var_err)
                            {
                                Logger::Error("Syntax Error: failed to make value from argument to function call", {funcname.content});
                                return var_err;
                            }
                        }
                        varargs.push_back(v);
                    }
                    BuiltinFuncs::CallBuiltIn(funcname.content, varargs);
                    return Error::OK;
                }
                Logger::Error("Syntax Error: could not find function", {funcname.content});
                return Error::SYNTAX;
            }
            else
            {
                std::vector<std::string> scopes = Helper::SplitString(funcname.content, '.');

                Scope *scope;
                if (Helper::UnorderedMapHasKey(parent_scope.scopes, scopes.at(0)))
                {
                    scope = &parent_scope;
                }
                else if (Helper::UnorderedMapHasKey(global_scope.scopes, scopes.at(0)))
                {
                    scope = &global_scope;
                }
                else
                {
                    Logger::Error("Syntax Error: could not find scope", {scopes.at(0)});
                    return Error::SYNTAX;
                }

                for (std::string scope_name : scopes)
                {
                    Logger::Debug("Searching in scope:", {scope_name});

                    if (Helper::UnorderedMapHasKey(scope->scopes, scope_name))
                    {
                        scope = &scope->scopes.at(scope_name);
                        Logger::Debug("SCOPENAME:", {scope->name});
                        if (!(scope->type == SCOPE_TYPE::FUNC && scope->name == scopes.back()))
                        {
                            continue;
                        }
                    }

                    Error arg_err = SetArgumentsBeforeCall(*scope, args, parent_scope, global_scope);
                    if (arg_err)
                        return arg_err;
                    Error exec_err = ExecuteScope(*scope, global_scope);
                    if (exec_err)
                        return exec_err;
                    return Error::OK;
                }

                Logger::Error("Syntax Error: failed to find function:", {funcname.content});
                return Error::SYNTAX;
            }
        }
        default:
            break;
        }

        return Error::OK;
    }

    Error ExecuteScope(Scope &scope, Scope &global_scope)
    {
        for (Instruction &inst : scope.instructions)
        {
            Error inst_err = ExecuteInstruction(inst, scope, global_scope);
            if (inst_err)
                return inst_err;
        }
        return Error::OK;
    }

    Error RecursiveScopeExecutor(Scope *current_scope, Scope &global_scope)
    {
        for (auto [key, val] : current_scope->scopes)
        {
            if (val.type == SCOPE_TYPE::FUNC || val.type == SCOPE_TYPE::CLASS)
                continue;

            Logger::Debug("Executing scope:", {val.name});
            Error exe_err = ExecuteScope(val, global_scope);
            if (exe_err)
            {
                Logger::Error("Failed to execute scope:", {val.name});
                return exe_err;
            }

            return RecursiveScopeExecutor(&val, global_scope);
        }
        return Error::OK;
    }

    Error InterpretGlobalScope(Scope &global_scope)
    {
        Logger::Debug("Starting Interpretation...", {});

        if (!Helper::UnorderedMapHasKey(global_scope.scopes, std::string{"main"}))
        {
            Logger::Error("Syntax Error: function 'main' not found in global scope.", {});
            return Error::SYNTAX;
        }

        Scope &main = global_scope.scopes.at("main");

        if (main.type != SCOPE_TYPE::FUNC)
        {
            Logger::Error("Syntax Error: 'main' in global scope must be a function.", {});
            return Error::SYNTAX;
        }

        Logger::Debug("Executing Global Scope.", {});
        Error exe_err = ExecuteScope(global_scope, global_scope);
        if (exe_err)
            return exe_err;

        Logger::Debug("Executing Nested Scopes.", {});
        Scope *recursion_scope = &global_scope;
        Error scope_exe_err = RecursiveScopeExecutor(recursion_scope, global_scope);
        if (scope_exe_err)
            return scope_exe_err;

        Logger::Debug("Executing Main.", {});
        Error main_err = ExecuteScope(main, global_scope);
        if (main_err)
            return main_err;

        Logger::Debug("Finished Interpretation.", {});

        return Error::OK;
    }
}