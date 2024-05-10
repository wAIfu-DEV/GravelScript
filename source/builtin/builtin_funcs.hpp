#pragma once

#include <iostream>
#include <unordered_map>
#include <functional>

#include "../logger/logger.hpp"
#include "../helper/helper.hpp"
#include "../types/variant.hpp"

namespace BuiltinFuncs
{
    Variant Print(std::vector<Variant> &args)
    {
        Variant ret{
            .type = VALUE_TYPE::NIL,
            .flags = {},
            .d64 = 0,
        };

        if (args.size() != 1)
        {
            Logger::Error("Syntax Error: Print function takes 1 argument.", {});
            return ret;
        }
        Variant &arg0 = args.at(0);

        if (arg0.type != VALUE_TYPE::STRING)
        {
            Logger::Error("Syntax Error: argument 0 of Print must be a string.", {});
            return ret;
        }
        std::cout << Memory::strings.at(arg0.d64) << "\n";
        return ret;
    }

    Variant ToString(std::vector<Variant> &args)
    {
        Variant ret{
            .type = VALUE_TYPE::STRING,
            .flags = {},
            .d64 = 0,
        };

        if (args.size() != 1)
        {
            Logger::Error("Syntax Error: Print function takes 1 argument.", {});
            return;
        }
        Variant &arg0 = args.at(0);

        std::string s = "";
        switch (arg0.type)
        {
        case VALUE_TYPE::STRING:
            return arg0;
        case VALUE_TYPE::INT:
            s = std::to_string(std::bit_cast<int64_t>(arg0.d64));
        case VALUE_TYPE::FLOAT:
            s = std::to_string(std::bit_cast<double>(arg0.d64));
        default:
            break;
        }
        ret.d64 = Memory::strings.size();
        Memory::strings.push_back(s);
        return ret;
    }

    const std::unordered_map<std::string, std::function<Variant(std::vector<Variant> &)>> BUILTIN_MAP{
        {"Print", Print},
        {"ToString", ToString},
    };

    Variant CallBuiltIn(const std::string name, std::vector<Variant> &args)
    {
        auto func = BUILTIN_MAP.at(name);
        return func(args);
    }

    bool IsBuiltIn(const std::string name)
    {
        return Helper::UnorderedMapHasKey(BUILTIN_MAP, name);
    }
}
