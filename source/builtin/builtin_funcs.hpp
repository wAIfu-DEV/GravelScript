#pragma once

#include <iostream>
#include <unordered_map>
#include <functional>

#include "../logger/logger.hpp"
#include "../helper/helper.hpp"
#include "../types/variant.hpp"

namespace BuiltinFuncs
{
    void Print(std::vector<Variant> &args)
    {
        if (args.size() != 1)
        {
            Logger::Error("Syntax Error: Print function takes 1 argument.", {});
            return;
        }
        Variant &arg0 = args.at(0);
        Logger::Debug("ARGTYPE:", {std::to_string((uint8_t)arg0.type)});
        if (arg0.type != VALUE_TYPE::STRING)
        {
            Logger::Error("Syntax Error: argument 0 of Print must be a string.", {});
            return;
        }
        std::cout << Memory::strings.at(arg0.d64) << "\n";
    }

    const std::unordered_map<std::string, std::function<void(std::vector<Variant> &)>> BUILTIN_MAP{
        {"Print", Print},
    };

    void CallBuiltIn(const std::string name, std::vector<Variant> &args)
    {
        auto func = BUILTIN_MAP.at(name);
        func(args);
    }

    bool IsBuiltIn(const std::string name)
    {
        return Helper::UnorderedMapHasKey(BUILTIN_MAP, name);
    }
}
