#pragma once

#include <cstdint>
#include <iostream>
#include <unordered_map>

#include "variant.hpp"
#include "instructions.hpp"

enum class SCOPE_TYPE : uint8_t
{
    GLOBAL,
    CLASS,
    FUNC,
    NAMESPACE,
};

struct ScopeRuntimeVars
{
    size_t if_depth = 0;
};

struct Scope
{
    SCOPE_TYPE type;
    Scope *parent;
    ScopeRuntimeVars runtime_vars;
    std::string name;
    std::vector<std::pair<std::string, Variant>> args;
    std::unordered_map<std::string, Variant> vars;
    std::unordered_map<std::string, Scope> scopes;
    std::vector<Instruction> instructions;
};