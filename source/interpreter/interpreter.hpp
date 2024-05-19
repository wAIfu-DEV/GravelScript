#pragma once

#include <iostream>
#include <filesystem>

#include "../logger/logger.hpp"
#include "../helper/helper.hpp"

#include "../types/error.hpp"
#include "../types/token.hpp"
#include "../types/variant.hpp"
#include "../types/instructions.hpp"
#include "../types/scope.hpp"
#include "../instructions/instructions.hpp"
#include "../builtin/builtin_funcs.hpp"
#include "../make_variant/make_variant.hpp"

namespace Interpreter
{
    Error ExecuteScope(Scope &scope, Scope &global_scope);
    Error RecursiveScopeExecutor(Scope &current_scope, Scope &global_scope);

    void PrintTreeComposition(Scope &scope, size_t depth = 0)
    {
        std::string prefix;
        for (size_t i = 0; i < depth; ++i)
        {
            prefix += "  ";
        }

        Logger::Debug(prefix + "Scope:", {scope.name});

        Logger::Debug(prefix + "Args:", {});
        for (auto &[key, val] : scope.args)
        {
            Logger::Debug(prefix + key, {});
        }

        Logger::Debug(prefix + "Vars:", {});
        for (auto &[key, val] : scope.vars)
        {
            Logger::Debug(prefix + key, {});
        }

        Logger::Debug(prefix + "Scopes:", {});
        for (auto &[key, val] : scope.scopes)
        {
            Logger::Debug(prefix + key, {});
            PrintTreeComposition(val, depth + 1);
        }
    }

    bool IsBoolConvertible(VALUE_TYPE type)
    {
        return (type == VALUE_TYPE::INT || type == VALUE_TYPE::NIL || type == VALUE_TYPE::FLOAT);
    }

    Variant ResolveName(Token::Token varname, Scope &parent_scope)
    {
        if (!Helper::StringContains(varname.content, '.'))
        {
            if (Helper::UnorderedMapHasKey(parent_scope.vars, varname.content))
            {
                return parent_scope.vars.at(varname.content);
            }
            else if (parent_scope.type == SCOPE_TYPE::FUNC &&
                     Helper::PairVectorHasKey(parent_scope.args, varname.content))
            {
                Variant *v = nullptr;
                Helper::PairVectorGet(parent_scope.args, varname.content, &v);
                return *v;
            }

            // Recursive scope walking
            Scope *next_parent_scope = parent_scope.parent;
            while (next_parent_scope)
            {
                if (Helper::UnorderedMapHasKey(next_parent_scope->vars, varname.content))
                {
                    return next_parent_scope->vars.at(varname.content);
                }
                next_parent_scope = next_parent_scope->parent;
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

            Scope *scope = nullptr;
            if (Helper::UnorderedMapHasKey(parent_scope.scopes, scopes.at(0)))
            {
                scope = &parent_scope;
            }
            /*
            else if (Helper::UnorderedMapHasKey(global_scope.scopes, scopes.at(0)))
            {
                scope = &global_scope;
            }*/
            else
            {
                // Recursive scope walking
                Scope *next_parent_scope = parent_scope.parent;
                while (next_parent_scope)
                {
                    if (Helper::UnorderedMapHasKey(next_parent_scope->scopes, scopes.at(0)))
                    {
                        scope = next_parent_scope;
                        break;
                    }
                    next_parent_scope = next_parent_scope->parent;
                }

                if (!scope)
                {
                    Logger::Error("Syntax Error: could not find scope", {scopes.at(0)});
                    Variant v = {
                        .type = VALUE_TYPE::NIL,
                        .d64 = 0,
                    };
                    return v;
                }
            }

            for (std::string &scope_name : scopes)
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

    Error SetArgumentsBeforeCall(Scope &scope, std::vector<Token::Token> &args, Scope &parent_scope)
    {
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
        for (auto &[key, val] : scope.args)
        {
            if (args[i].type == Token::NAME)
            {
                Variant v = ResolveName(args[i], parent_scope);
                scope.args[i].second = v;
            }
            else
            {
                Variant *v = nullptr;
                Helper::PairVectorGet(scope.args, key, &v);
                Error set_err = Instructions::Set(*v, args[i], false);
                if (set_err)
                    return set_err;
            }
            ++i;
        }
        return Error::OK;
    }

    Error FunctionCall(Instruction inst, Scope &parent_scope, Scope &global_scope)
    {
        if (inst.args.size() < 1)
        {
            Logger::Error("Syntax Error: not enough arguments for instruction call.", {});
            return Error::SYNTAX;
        }

        Token::Token &funcname = inst.args.at(1);

        std::vector<Token::Token> args = {};
        for (size_t i = 3; i < inst.args.size(); ++i)
        {
            if (inst.args[i].type != Token::COMMA)
                args.push_back(inst.args[i]);
        }

        Logger::Debug("CALL", {funcname.content});

        if (!Helper::StringContains(funcname.content, '.'))
        {
            if (Helper::UnorderedMapHasKey(parent_scope.scopes, funcname.content))
            {
                Scope &func = parent_scope.scopes.at(funcname.content);
                Error arg_err = SetArgumentsBeforeCall(func, args, parent_scope);
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

                Error arg_err = SetArgumentsBeforeCall(func, args, parent_scope);
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
                for (Token::Token &tok : args)
                {
                    Variant v;
                    if (tok.type == Token::NAME)
                    {
                        v = ResolveName(tok, parent_scope);
                    }
                    else
                    {
                        Error var_err = MakeVariant(v, {tok});
                        if (var_err)
                        {
                            Logger::Error("Syntax Error: failed to make value from argument to function call", {funcname.content});
                            return var_err;
                        }
                    }
                    varargs.push_back(v);
                }
                bool builtin_error = false;
                Variant return_val = BuiltinFuncs::CallBuiltIn(funcname.content, varargs, builtin_error);
                if (builtin_error)
                    return Error::UNHANDLED;
                global_scope.vars.insert_or_assign("retVal", return_val);
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

            for (std::string &scope_name : scopes)
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

                Error arg_err = SetArgumentsBeforeCall(*scope, args, parent_scope);
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

    Error ExecuteInstruction(Instruction &inst, Scope &parent_scope, Scope &global_scope)
    {
        Logger::Debug("INST", {inst.args.at(0).content});
        switch (inst.type)
        {
        case Token::KEYW_FETCH:
        {
            std::vector<Token::Token> args{};
            for (size_t i = 2; i < inst.args.size(); ++i)
            {
                args.push_back(inst.args[i]);
            }

            Instruction temp_inst = {
                .type = Token::KEYW_CALL,
                .args = args,
            };
            Error call_err = FunctionCall(temp_inst, parent_scope, global_scope);
            if (call_err)
                return call_err;
        }
        case Token::KEYW_VAR:
        case Token::KEYW_CONST:
        case Token::KEYW_SET:
        {
            if (inst.args.size() < 4)
            {
                Logger::Error("Syntax Error: not enough arguments for instruction 'set/var/const'.", {});
                return Error::SYNTAX;
            }

            Token::Token &varname = inst.args.at(1);
            Token::Token &value = inst.args.at(3);

            bool is_const = inst.type == Token::KEYW_CONST;
            bool no_override = inst.type == Token::KEYW_CONST || inst.type == Token::KEYW_VAR;
            bool create_new = inst.type != Token::KEYW_SET;

            Variant var_val{};

            if (inst.type == Token::KEYW_FETCH)
            {
                var_val = global_scope.vars.at("retVal");
            }
            else if (value.type == Token::NAME)
            {
                var_val = ResolveName(value, parent_scope);
            }
            else
            {
                Error make_err = MakeVariant(var_val, value, is_const);
                if (make_err)
                    return make_err;
            }

            Logger::Debug("SET", {varname.content, value.content});

            if (!Helper::StringContains(varname.content, '.'))
            {
                Logger::Debug("setting:", {varname.content, "in scope:", parent_scope.name});

                if (Helper::UnorderedMapHasKey(parent_scope.vars, varname.content))
                {
                    if (no_override)
                    {
                        Logger::Error("Syntax Error: variable", {varname.content, "already exists in scope", parent_scope.name});
                        return Error::SYNTAX;
                    }
                    parent_scope.vars.insert_or_assign(varname.content, var_val);
                    return Error::OK;
                }
                else if (parent_scope.type == SCOPE_TYPE::FUNC &&
                         Helper::PairVectorHasKey(parent_scope.args, varname.content))
                {
                    if (no_override)
                    {
                        Logger::Error("Syntax Error: variable", {varname.content, "already exists as an argument of", parent_scope.name});
                        return Error::SYNTAX;
                    }
                    Variant *v = nullptr;
                    Helper::PairVectorGet(parent_scope.args, varname.content, &v);
                    *v = var_val;
                    return Error::OK;
                }

                // Recursive scope walking
                Scope *next_parent_scope = parent_scope.parent;
                while (next_parent_scope)
                {
                    if (Helper::UnorderedMapHasKey(next_parent_scope->vars, varname.content))
                    {
                        if (no_override)
                        {
                            Logger::Error("Syntax Error: variable", {varname.content, "already exists in scope", next_parent_scope->name});
                            return Error::SYNTAX;
                        }
                        next_parent_scope->vars.insert_or_assign(varname.content, var_val);
                        return Error::OK;
                    }
                    next_parent_scope = next_parent_scope->parent;
                }

                if (!create_new)
                {
                    Logger::Error("Syntax Error: cannot set undeclared variable:", {varname.content});
                    return Error::SYNTAX;
                }

                parent_scope.vars.insert({varname.content, var_val});
                return Error::OK;
            }
            else
            {
                std::vector<std::string> scopes = Helper::SplitString(varname.content, '.');

                for (std::string &s : scopes)
                {
                    Logger::Debug("COMPOSITE:", {s});
                }

                Scope *scope = nullptr;
                if (Helper::UnorderedMapHasKey(parent_scope.scopes, scopes.at(0)))
                {
                    scope = &parent_scope;
                }
                else
                {
                    // Recursive scope walking
                    Scope *next_parent_scope = parent_scope.parent;
                    while (next_parent_scope)
                    {
                        if (Helper::UnorderedMapHasKey(next_parent_scope->scopes, scopes.at(0)))
                        {
                            scope = next_parent_scope;
                            break;
                        }
                        next_parent_scope = next_parent_scope->parent;
                    }

                    if (!scope)
                    {
                        Logger::Error("Syntax Error: could not find scope", {scopes.at(0)});
                        return Error::SYNTAX;
                    }
                }

                for (std::string &scope_name : scopes)
                {
                    if (Helper::UnorderedMapHasKey(scope->scopes, scope_name))
                    {
                        scope = &scope->scopes.at(scope_name);
                        continue;
                    }
                    else if (Helper::UnorderedMapHasKey(scope->vars, scope_name))
                    {
                        if (no_override)
                        {
                            Logger::Error("Syntax Error: variable", {varname.content, "already exists in scope", scope->name});
                            return Error::SYNTAX;
                        }
                        scope->vars.insert_or_assign(scope_name, var_val);
                        return Error::OK;
                    }
                    else if (scope->type == SCOPE_TYPE::FUNC && Helper::PairVectorHasKey(scope->args, scope_name))
                    {
                        if (no_override)
                        {
                            Logger::Error("Syntax Error: variable", {varname.content, "already exists in scope", scope->name});
                            return Error::SYNTAX;
                        }
                        Variant *v = nullptr;
                        Helper::PairVectorGet(scope->args, scope_name, &v);
                        *v = var_val;
                        return Error::OK;
                    }

                    Logger::Debug("Failed to find name:", {scope_name, "in scope:", scope->name});

#if !GVS_RELEASE
                    PrintTreeComposition(global_scope);
#endif

                    if (!create_new)
                    {
                        Logger::Error("Syntax Error: cannot set undeclared variable:", {varname.content});
                        return Error::SYNTAX;
                    }

                    scope->vars.insert({scope_name, var_val});
                    return Error::OK;
                }
            }
        }
        case Token::KEYW_ARRAY:
        {
            if (inst.args.size() < 2)
            {
                Logger::Error("Syntax Error: not enough arguments for instruction 'array'.", {});
                return Error::SYNTAX;
            }

            Token::Token &varname = inst.args.at(1);

            Logger::Debug("setting:", {varname.content, "in scope:", parent_scope.name});

            if (Helper::UnorderedMapHasKey(parent_scope.vars, varname.content))
            {
                Logger::Error("Syntax Error: variable", {varname.content, "already exists in scope", parent_scope.name});
                return Error::SYNTAX;
            }
            else if (parent_scope.type == SCOPE_TYPE::FUNC &&
                     Helper::PairVectorHasKey(parent_scope.args, varname.content))
            {
                Logger::Error("Syntax Error: variable", {varname.content, "already exists as an argument of", parent_scope.name});
                return Error::SYNTAX;
            }

            // Recursive scope walking
            Scope *next_parent_scope = parent_scope.parent;
            while (next_parent_scope)
            {
                if (Helper::UnorderedMapHasKey(next_parent_scope->vars, varname.content))
                {
                    Logger::Error("Syntax Error: variable", {varname.content, "already exists in scope", next_parent_scope->name});
                    return Error::SYNTAX;
                }
                next_parent_scope = next_parent_scope->parent;
            }

            std::vector<Token::Token> values{};

            for (size_t i = 2; i < inst.args.size(); ++i)
            {
                Token::Token t = inst.args.at(i);
                if (t.type != Token::COMMA)
                    values.push_back(t);
            }

            Variant v = {
                .type = VALUE_TYPE::NIL,
                .d64 = 0,
            };
            Error make_err = MakeVarArray(v, values);
            if (make_err)
                return make_err;
            parent_scope.vars.insert({varname.content, v});
            return Error::OK;
        }
        case Token::KEYW_CALL:
        {
            Error call_err = FunctionCall(inst, parent_scope, global_scope);
            if (call_err)
                return call_err;
            return Error::OK;
        }
        case Token::KEYW_IMPORT:
        {
            namespace fs = std::filesystem;

            if (inst.args.size() < 4)
            {
                Logger::Error("Syntax Error: not enough arguments for instruction 'import'.", {});
                return Error::SYNTAX;
            }

            Token::Token &path = inst.args.at(1);
            Token::Token &alias = inst.args.at(3);

            Logger::Debug("IMPORT", {path.content, alias.content});

            if (path.type != Token::STRING)
            {
                Logger::Error("Syntax Error: first argument of 'import' must be of type string.", {});
                return Error::SYNTAX;
            }
            if (alias.type != Token::NAME)
            {
                Logger::Error("Syntax Error: second argument of 'import' must be a name.", {});
                return Error::SYNTAX;
            }

            fs::path abs_path;
            try
            {
                abs_path = fs::canonical(path.content);
                Logger::Debug("Found importable file at path:", {path.content});
            }
            catch ([[maybe_unused]] const std::exception &e)
            {
                Logger::Error("Path used in 'import' instruction could not be resolve:", {path.content});
                return Error::REJECTED;
            }

            std::vector<Token::Token> tokens{};

            Error lex_err = Script::LexFile(abs_path.string(), tokens);
            if (lex_err)
                return lex_err;

            global_scope.scopes.insert_or_assign(alias.content, Scope{
                                                                    .type = SCOPE_TYPE::GLOBAL,
                                                                    .parent = &global_scope,
                                                                    .name = std::string("#") + alias.content,
                                                                    .args = {},
                                                                    .vars = {},
                                                                    .scopes = {},
                                                                });
            Scope &imported_global = global_scope.scopes.at(alias.content);

            Error parse_err = Parser::ParseTokens(tokens, imported_global);
            if (parse_err)
                return parse_err;

            Error exe_err = ExecuteScope(imported_global, imported_global);
            if (exe_err)
                return exe_err;

            Error scope_exe_err = RecursiveScopeExecutor(imported_global, imported_global);
            if (scope_exe_err)
                return scope_exe_err;
            return Error::OK;
        }
        case Token::KEYW_RETURN:
        {
            Variant v{
                .type = VALUE_TYPE::NIL,
                .flags = {},
                .d64 = 0,
            };

            if (inst.args.size() < 2)
            {
                global_scope.vars.insert_or_assign("retVal", v);
                return Error::EARLY_RETURN;
            }

            Token::Token value = inst.args.at(1);

            Error make_err = MakeVariant(v, value, false);
            if (make_err)
            {
                Logger::Error("Could not make variant out of return value.", {});
                return make_err;
            }

            global_scope.vars.insert_or_assign("retVal", v);
            return Error::EARLY_RETURN;
        }
        case Token::KEYW_ELIF:
        case Token::KEYW_IF:
        {
            if (inst.args.size() < 2)
            {
                Logger::Error("Syntax Error: not enough arguments for instruction 'if/elif'.", {});
                return Error::SYNTAX;
            }

            Token::Token compare_func = inst.args.at(1);
            Token::Token func_tok = Token::Token{
                .content = "func",
                .type = Token::KEYW_FUNC,
            };
            Instruction temp_func_inst = Instruction{
                .type = Token::KEYW_FUNC,
                .args = {func_tok},
            };
            for (size_t i = 1; i < inst.args.size(); ++i)
            {
                temp_func_inst.args.push_back(inst.args.at(i));
            }
            Error call_err = FunctionCall(temp_func_inst, parent_scope, global_scope);
            if (call_err)
                return call_err;
            Variant return_val = global_scope.vars.at("retVal");

            if (!IsBoolConvertible(return_val.type))
            {
                Logger::Error("Syntax Error: Function used in 'if' instruction must return a type convertible to boolean expression (int, float, null).", {});
                return Error::SYNTAX;
            }

            bool boolean_val;
            switch (return_val.type)
            {
            case VALUE_TYPE::INT:
                boolean_val = std::bit_cast<int64_t>(return_val.d64) != 0LL;
                break;
            case VALUE_TYPE::FLOAT:
                boolean_val = std::bit_cast<double>(return_val.d64) != 0.0;
                break;
            case VALUE_TYPE::NIL:
                boolean_val = false;
                break;
            default:
                boolean_val = false;
                break;
            }

            Logger::Debug("IF RESULT:", {std::to_string(boolean_val)});

            if (boolean_val)
                return Error::EXE_UPTO_IF;
            else
                return Error::SKIP_TO_IF;
        }
        case Token::KEYW_ELSE:
        {
            return Error::OK;
        }
        case Token::KEYW_ENDIF:
        {
            return Error::OK;
        }
        default:
            Logger::Error("Syntax Error: Unexpected instruction:", {inst.args.at(0).content});
            return Error::REJECTED;
        }

        return Error::OK;
    }

    enum class EXEC_BEHAVIOUR
    {
        NORMAL,
        EXEC_UPTO_ELIF,
        SKIP_TO_ELIF,
        SKIP_TO_END,
    };

    struct Behaviour
    {
        EXEC_BEHAVIOUR type;
        size_t from_depth;
    };

    void PrintBehaviourStack(std::vector<Behaviour> stack)
    {
        std::vector<std::string> behavs{};
        for (auto b : stack)
        {
            switch (b.type)
            {
            case EXEC_BEHAVIOUR::NORMAL:
                behavs.push_back("NORMAL");
                break;
            case EXEC_BEHAVIOUR::EXEC_UPTO_ELIF:
                behavs.push_back("EXEC_UPTO_ELIF");
                break;
            case EXEC_BEHAVIOUR::SKIP_TO_ELIF:
                behavs.push_back("SKIP_TO_ELIF");
                break;
            case EXEC_BEHAVIOUR::SKIP_TO_END:
                behavs.push_back("SKIP_TO_END");
                break;
            default:
                break;
            }
        }
        Logger::Debug("BEHAVIOURS:", behavs);
    }

    Error ExecuteScope(Scope &scope, Scope &global_scope)
    {
        scope.runtime_vars = {
            .if_depth = 0,
        };

        std::vector<Behaviour> behaviour_stack{Behaviour{
            .type = EXEC_BEHAVIOUR::NORMAL,
            .from_depth = 0,
        }};

        for (size_t i = 0; i < scope.instructions.size(); ++i)
        {
            Instruction &inst = scope.instructions.at(i);
            Behaviour behav = behaviour_stack.back();

            switch (behav.type)
            {
            case EXEC_BEHAVIOUR::EXEC_UPTO_ELIF:
            {
                if (!inst.args.size())
                    continue;
                Token::TYPE inst_type = inst.args.at(0).type;

                if (inst_type == Token::KEYW_IF)
                    scope.runtime_vars.if_depth += 1;

                if (behav.from_depth == scope.runtime_vars.if_depth && (inst_type == Token::KEYW_ELIF || inst_type == Token::KEYW_ELSE))
                {
                    behaviour_stack.pop_back();
                    behaviour_stack.push_back(Behaviour{
                        .type = EXEC_BEHAVIOUR::SKIP_TO_END,
                        .from_depth = scope.runtime_vars.if_depth,
                    });
                    PrintBehaviourStack(behaviour_stack);
                    continue;
                }
                else if (behav.from_depth == scope.runtime_vars.if_depth && inst_type == Token::KEYW_ENDIF)
                {
                    scope.runtime_vars.if_depth -= 1;
                    behaviour_stack.pop_back();
                    PrintBehaviourStack(behaviour_stack);
                    continue;
                }

                if (inst_type == Token::KEYW_ENDIF)
                    scope.runtime_vars.if_depth -= 1;
            }
            case EXEC_BEHAVIOUR::SKIP_TO_ELIF:
            {
                if (behav.type == EXEC_BEHAVIOUR::SKIP_TO_ELIF)
                {
                    if (!inst.args.size())
                        continue;

                    Token::TYPE inst_type = inst.args.at(0).type;
                    bool fallthrough = false;

                    if (inst_type == Token::KEYW_IF)
                        scope.runtime_vars.if_depth += 1;

                    if (behav.from_depth == scope.runtime_vars.if_depth && (inst_type == Token::KEYW_ELIF || inst_type == Token::KEYW_ELSE))
                    {
                        behaviour_stack.pop_back();
                        behaviour_stack.push_back(Behaviour{
                            .type = EXEC_BEHAVIOUR::EXEC_UPTO_ELIF,
                            .from_depth = scope.runtime_vars.if_depth,
                        });
                        PrintBehaviourStack(behaviour_stack);
                        fallthrough = true;
                    }
                    else if (behav.from_depth == scope.runtime_vars.if_depth && inst_type == Token::KEYW_ENDIF)
                    {
                        scope.runtime_vars.if_depth -= 1;
                        behaviour_stack.pop_back();
                        PrintBehaviourStack(behaviour_stack);
                        continue;
                    }

                    if (inst_type == Token::KEYW_ENDIF)
                        scope.runtime_vars.if_depth -= 1;

                    if (!fallthrough)
                        continue;
                }
            }
            case EXEC_BEHAVIOUR::NORMAL:
            {
                Error inst_err = ExecuteInstruction(inst, scope, global_scope);
                if (inst_err == Error::EARLY_RETURN)
                    return Error::OK;
                if (inst_err == Error::SKIP_TO_IF)
                {
                    behaviour_stack.push_back(Behaviour{
                        .type = EXEC_BEHAVIOUR::SKIP_TO_ELIF,
                        .from_depth = scope.runtime_vars.if_depth,
                    });
                    PrintBehaviourStack(behaviour_stack);
                    continue;
                }
                if (inst_err == Error::SKIP_TO_END)
                {
                    behaviour_stack.push_back(Behaviour{
                        .type = EXEC_BEHAVIOUR::SKIP_TO_END,
                        .from_depth = scope.runtime_vars.if_depth,
                    });
                    PrintBehaviourStack(behaviour_stack);
                    continue;
                }
                if (inst_err == Error::EXE_UPTO_IF)
                {
                    if (behav.type == EXEC_BEHAVIOUR::SKIP_TO_ELIF)
                    {
                        behaviour_stack.pop_back();
                        PrintBehaviourStack(behaviour_stack);
                    }
                    behaviour_stack.push_back(Behaviour{
                        .type = EXEC_BEHAVIOUR::EXEC_UPTO_ELIF,
                        .from_depth = scope.runtime_vars.if_depth,
                    });
                    PrintBehaviourStack(behaviour_stack);
                    continue;
                }
                if (inst_err)
                {
                    Logger::Debug("SCOPE ERROR:", {std::to_string(inst_err)});
                    return inst_err;
                }
                continue;
            }
            case EXEC_BEHAVIOUR::SKIP_TO_END:
            {
                if (!inst.args.size())
                    continue;
                Token::TYPE inst_type = inst.args.at(0).type;

                if (inst_type == Token::KEYW_IF)
                    scope.runtime_vars.if_depth += 1;

                if (inst_type == Token::KEYW_ENDIF)
                {
                    Logger::Debug("FOUND ENDIF", {});
                    if (behav.from_depth == scope.runtime_vars.if_depth)
                    {
                        behaviour_stack.pop_back();
                        PrintBehaviourStack(behaviour_stack);
                    }
                    else
                    {
                        Logger::Debug("NOT SAME DEPTH.", {});
                    }
                }

                if (inst_type == Token::KEYW_ENDIF)
                    scope.runtime_vars.if_depth -= 1;
                continue;
            }
            default:
                break;
            }
        }
        return Error::OK;
    }

    Error RecursiveScopeExecutor(Scope &current_scope, Scope &global_scope)
    {
        Logger::Debug("Recursing over scopes in:", {current_scope.name});
        Error retval = Error::OK;
        for (auto &[key, val] : current_scope.scopes)
        {
            if (val.type == SCOPE_TYPE::FUNC || val.type == SCOPE_TYPE::CLASS)
                continue;

            if (val.name.starts_with("#"))
                continue;

            Logger::Debug("Executing scope:", {"key:", key, "name:", val.name});
            Error exe_err = ExecuteScope(val, global_scope);
            if (exe_err)
            {
                Logger::Error("Failed to execute scope:", {val.name});
                return exe_err;
            }

            retval = RecursiveScopeExecutor(val, global_scope);
            if (retval)
            {
                Logger::Debug("Finished executing scope due to error.", {});
                return retval;
            }
        }
        Logger::Debug("Finished executing scope:", {current_scope.name});
        return Error::OK;
    }

    Error InterpretGlobalScope(Scope &global_scope)
    {
        Logger::Debug("Starting Interpretation...", {});

        if (!Helper::UnorderedMapHasKey(global_scope.scopes, std::string{"Main"}))
        {
            Logger::Error("Syntax Error: function 'Main' not found in global scope.", {});
            return Error::SYNTAX;
        }

        Variant default_retval{
            .type = VALUE_TYPE::NIL,
            .flags = {},
            .d64 = 0,
        };
        global_scope.vars.insert_or_assign("retVal", default_retval);

        Variant null{
            .type = VALUE_TYPE::NIL,
            .flags = {
                .is_const = true,
            },
            .d64 = 0,
        };
        global_scope.vars.insert_or_assign("null", null);

        Scope &main = global_scope.scopes.at("Main");

        if (main.type != SCOPE_TYPE::FUNC)
        {
            Logger::Error("Syntax Error: 'Main' in global scope must be a function.", {});
            return Error::SYNTAX;
        }

        Logger::Debug("Executing Global Scope.", {});
        Error exe_err = ExecuteScope(global_scope, global_scope);
        if (exe_err)
            return exe_err;

        Logger::Debug("Scopes in global:", {std::to_string(global_scope.scopes.size())});

        Logger::Debug("Executing Nested Scopes.", {});
        Error scope_exe_err = RecursiveScopeExecutor(global_scope, global_scope);
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