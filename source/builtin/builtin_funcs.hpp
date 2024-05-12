#pragma once

#include <iostream>
#include <unordered_map>
#include <functional>
#include <cstdint>

#include "../logger/logger.hpp"
#include "../helper/helper.hpp"
#include "../types/variant.hpp"

namespace BuiltinFuncs
{
    Variant AddI(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::NIL,
            .flags = {},
            .d64 = 0,
        };

        int64_t result = 0;

        for (auto v : args)
        {
            switch (v.type)
            {
            case VALUE_TYPE::INT:
                result += std::bit_cast<int64_t>(v.d64);
                break;
            case VALUE_TYPE::FLOAT:
                result += static_cast<int64_t>(round(std::bit_cast<double>(v.d64)));
                break;
            default:
                Logger::Error("AddInt only takes arguments of type int or float.", {});
                errored = true;
                return ret;
            }
        }

        ret.type = VALUE_TYPE::INT;
        ret.d64 = std::bit_cast<uint64_t>(result);
        return ret;
    }

    Variant AddF(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::NIL,
            .flags = {},
            .d64 = 0,
        };

        double result = 0.0;

        for (auto v : args)
        {
            switch (v.type)
            {
            case VALUE_TYPE::INT:
                result += std::bit_cast<int64_t>(v.d64);
                break;
            case VALUE_TYPE::FLOAT:
                result += std::bit_cast<double>(v.d64);
                break;
            default:
                Logger::Error("AddFloat only takes arguments of type int or float.", {});
                errored = true;
                return ret;
            }
        }

        ret.type = VALUE_TYPE::FLOAT;
        ret.d64 = std::bit_cast<uint64_t>(result);
        return ret;
    }

    Variant Add(std::vector<Variant> args, bool &errored)
    {
        return AddF(args, errored);
    }

    Variant MulI(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::NIL,
            .flags = {},
            .d64 = 0,
        };

        int64_t result = 0;

        for (auto v : args)
        {
            switch (v.type)
            {
            case VALUE_TYPE::INT:
                result = result * std::bit_cast<int64_t>(v.d64);
                break;
            case VALUE_TYPE::FLOAT:
                result = result * static_cast<int64_t>(round(std::bit_cast<double>(v.d64)));
                break;
            default:
                Logger::Error("MultiplyInt only takes arguments of type int or float.", {});
                errored = true;
                return ret;
            }
        }

        ret.type = VALUE_TYPE::INT;
        ret.d64 = std::bit_cast<uint64_t>(result);
        return ret;
    }

    Variant MulF(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::NIL,
            .flags = {},
            .d64 = 0,
        };

        double result = 0.0;
        bool first = true;

        for (auto v : args)
        {
            switch (v.type)
            {
            case VALUE_TYPE::INT:
                if (first)
                {
                    result = std::bit_cast<int64_t>(v.d64);
                    break;
                }
                result = result * std::bit_cast<int64_t>(v.d64);
                break;
            case VALUE_TYPE::FLOAT:
                if (first)
                {
                    result = std::bit_cast<double>(v.d64);
                    break;
                }
                result = result * std::bit_cast<double>(v.d64);
                break;
            default:
                Logger::Error("AddFloat only takes arguments of type int or float.", {});
                errored = true;
                return ret;
            }
            first = false;
        }

        ret.type = VALUE_TYPE::FLOAT;
        ret.d64 = std::bit_cast<uint64_t>(result);
        return ret;
    }

    Variant Mul(std::vector<Variant> args, bool &errored)
    {
        return MulF(args, errored);
    }

    Variant ToString(std::vector<Variant> args, bool &errored)
    {
        if (args.size() != 1)
        {
            Logger::Error("Syntax Error: ToString function takes 1 argument.", {});
            errored = true;
            return Variant{
                .type = VALUE_TYPE::NIL,
                .flags = {},
                .d64 = 0,
            };
        }
        Variant &arg0 = args.at(0);

        Variant ret{
            .type = VALUE_TYPE::STRING,
            .flags = {},
            .d64 = 0,
        };

        std::string s = "";
        switch (arg0.type)
        {
        case VALUE_TYPE::STRING:
            return arg0;
        case VALUE_TYPE::INT:
            s = std::to_string(std::bit_cast<int64_t>(arg0.d64));
            break;
        case VALUE_TYPE::FLOAT:
            s = std::to_string(std::bit_cast<double>(arg0.d64));
            break;
        case VALUE_TYPE::NIL:
            s = "null";
        default:
            break;
        }
        ret.d64 = Memory::strings.size();
        Memory::strings.push_back(s);
        return ret;
    }

    Variant Print(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::NIL,
            .flags = {},
            .d64 = 0,
        };

        if (args.size() < 1)
        {
            Logger::Error("Syntax Error: Print function takes at least 1 argument.", {});
            errored = true;
            return ret;
        }

        bool first = true;

        for (auto arg : args)
        {
            if (!first)
                std::cout << " ";

            if (arg.type == VALUE_TYPE::STRING)
            {
                std::cout << Memory::strings.at(arg.d64);
            }
            else
            {
                Variant as_str = BuiltinFuncs::ToString({arg}, errored);
                if (errored)
                    return ret;
                std::cout << Memory::strings.at(as_str.d64);
            }
            first = false;
        }

        std::cout << "\n";
        return ret;
    }

    const std::unordered_map<std::string, std::function<Variant(std::vector<Variant> &, bool &)>> BUILTIN_MAP{
        {"Print", Print},
        {"ToString", ToString},
        {"AddI", AddI},
        {"AddF", AddF},
        {"Add", Add},
        {"MulI", MulI},
        {"MulF", MulF},
        {"Mul", Mul},
    };

    Variant CallBuiltIn(const std::string name, std::vector<Variant> &args, bool &errored)
    {
        auto func = BUILTIN_MAP.at(name);
        return func(args, errored);
    }

    bool IsBuiltIn(const std::string name)
    {
        return Helper::UnorderedMapHasKey(BUILTIN_MAP, name);
    }
}
