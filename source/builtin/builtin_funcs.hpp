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
                result = static_cast<int64_t>(round(static_cast<double>(result) * std::bit_cast<double>(v.d64)));
                break;
            default:
                Logger::Error("MultiplyInt only takes arguments of type int or float.", {});
                errored = true;
                return ret;
            }
            first = false;
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

    Variant GetLine(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::STRING,
            .flags = {},
            .d64 = 0,
        };

        if (args.size() > 1)
        {
            Logger::Error("Syntax Error: GetLine function takes 1 or no arguments.", {});
            errored = true;
            return ret;
        }

        if (args.size() == 1)
        {
            Variant arg = args.at(0);
            Variant as_str = BuiltinFuncs::ToString({arg}, errored);
            if (errored)
                return ret;
            std::cout << Memory::strings.at(as_str.d64);
        }

        std::string input;
        std::getline(std::cin, input);

        ret.d64 = Memory::strings.size();
        Memory::strings.push_back(input);
        return ret;
    }

    Variant GetChar(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::INT,
            .flags = {},
            .d64 = 0,
        };

        if (args.size() > 0)
        {
            Logger::Error("Syntax Error: GetChar function takes no arguments.", {});
            errored = true;
            return ret;
        }

        int c = Helper::GetUnbufferedChar();
        ret.d64 = std::bit_cast<uint64_t>(static_cast<int64_t>(c));
        return ret;
    }

    Variant StrFromChar(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::STRING,
            .flags = {},
            .d64 = 0,
        };

        if (args.size() != 1)
        {
            Logger::Error("Syntax Error: StrFromChar function takes 1 argument.", {});
            errored = true;
            return ret;
        }

        Variant arg0 = args.at(0);
        if (arg0.type != VALUE_TYPE::INT)
        {
            Logger::Error("Syntax Error: StrFromChar function 1 argument of type int.", {});
            errored = true;
            return ret;
        }

        std::string s = std::string(1, static_cast<char>(std::bit_cast<int64_t>(arg0.d64)));
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

    Variant Equals(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::INT,
            .flags = {},
            .d64 = 0,
        };

        if (args.size() < 2)
        {
            Logger::Error("Syntax Error: Equals function takes 2 arguments.", {});
            errored = true;
            return ret;
        }

        Variant arg0 = args.at(0);
        Variant arg1 = args.at(1);

        switch (arg0.type)
        {
        case VALUE_TYPE::STRING:
        {
            if (arg1.type != VALUE_TYPE::STRING)
            {
                return ret;
            }
            std::string str0 = Memory::strings.at(arg0.d64);
            std::string str1 = Memory::strings.at(arg1.d64);

            if (str0 != str1)
                return ret;
            ret.d64 = 1LL;
            return ret;
        }
        case VALUE_TYPE::INT:
        {
            if (arg1.type != VALUE_TYPE::INT && arg1.type != VALUE_TYPE::FLOAT && arg1.type != VALUE_TYPE::NIL)
            {
                return ret;
            }

            if (arg1.type == VALUE_TYPE::NIL)
            {
                if (std::bit_cast<int64_t>(arg0.d64) != 0)
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::INT)
            {
                if (std::bit_cast<int64_t>(arg0.d64) != std::bit_cast<int64_t>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::FLOAT)
            {
                if (std::bit_cast<int64_t>(arg0.d64) != std::bit_cast<double>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::FLOAT:
        {
            if (arg1.type != VALUE_TYPE::INT && arg1.type != VALUE_TYPE::FLOAT && arg1.type != VALUE_TYPE::NIL)
            {
                return ret;
            }

            if (arg1.type == VALUE_TYPE::NIL)
            {
                if (std::bit_cast<double>(arg0.d64) != 0)
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::INT)
            {
                if (std::bit_cast<double>(arg0.d64) != std::bit_cast<int64_t>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::FLOAT)
            {
                if (std::bit_cast<double>(arg0.d64) != std::bit_cast<double>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::NIL:
        {
            if (arg1.type != VALUE_TYPE::INT && arg1.type != VALUE_TYPE::FLOAT && arg1.type != VALUE_TYPE::NIL)
            {
                return ret;
            }

            if (arg1.type == VALUE_TYPE::NIL)
            {
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::INT)
            {
                if (0LL != std::bit_cast<int64_t>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::FLOAT)
            {
                if (0.0 != std::bit_cast<double>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }
            return ret;
        }
        default:
            Logger::Error("Syntax Error: Equals function only handles strings, ints, floats or null.", {});
            errored = true;
            return ret;
        }
    }

    Variant Greater(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::INT,
            .flags = {},
            .d64 = 0,
        };

        if (args.size() < 2)
        {
            Logger::Error("Syntax Error: Equals function takes 2 arguments.", {});
            errored = true;
            return ret;
        }

        Variant arg0 = args.at(0);
        Variant arg1 = args.at(1);

        switch (arg0.type)
        {
        case VALUE_TYPE::INT:
        {
            if (arg1.type != VALUE_TYPE::INT && arg1.type != VALUE_TYPE::FLOAT && arg1.type != VALUE_TYPE::NIL)
            {
                return ret;
            }

            if (arg1.type == VALUE_TYPE::NIL)
            {
                if (std::bit_cast<int64_t>(arg0.d64) <= 0)
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::INT)
            {
                if (std::bit_cast<int64_t>(arg0.d64) <= std::bit_cast<int64_t>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::FLOAT)
            {
                if (std::bit_cast<int64_t>(arg0.d64) <= std::bit_cast<double>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::FLOAT:
        {
            if (arg1.type != VALUE_TYPE::INT && arg1.type != VALUE_TYPE::FLOAT && arg1.type != VALUE_TYPE::NIL)
            {
                return ret;
            }

            if (arg1.type == VALUE_TYPE::NIL)
            {
                if (std::bit_cast<double>(arg0.d64) <= 0)
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::INT)
            {
                if (std::bit_cast<double>(arg0.d64) <= std::bit_cast<int64_t>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::FLOAT)
            {
                if (std::bit_cast<double>(arg0.d64) <= std::bit_cast<double>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::NIL:
        {
            if (arg1.type != VALUE_TYPE::INT && arg1.type != VALUE_TYPE::FLOAT && arg1.type != VALUE_TYPE::NIL)
            {
                return ret;
            }

            if (arg1.type == VALUE_TYPE::NIL)
            {
                return ret;
            }

            if (arg1.type == VALUE_TYPE::INT)
            {
                if (0LL <= std::bit_cast<int64_t>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::FLOAT)
            {
                if (0.0 <= std::bit_cast<double>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }
            return ret;
        }
        default:
            Logger::Error("Syntax Error: Equals function only handles strings, ints, floats or null.", {});
            errored = true;
            return ret;
        }
    }

    Variant Lesser(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::INT,
            .flags = {},
            .d64 = 0,
        };

        if (args.size() < 2)
        {
            Logger::Error("Syntax Error: Equals function takes 2 arguments.", {});
            errored = true;
            return ret;
        }

        Variant arg0 = args.at(0);
        Variant arg1 = args.at(1);

        switch (arg0.type)
        {
        case VALUE_TYPE::INT:
        {
            if (arg1.type != VALUE_TYPE::INT && arg1.type != VALUE_TYPE::FLOAT && arg1.type != VALUE_TYPE::NIL)
            {
                return ret;
            }

            if (arg1.type == VALUE_TYPE::NIL)
            {
                if (std::bit_cast<int64_t>(arg0.d64) >= 0)
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::INT)
            {
                if (std::bit_cast<int64_t>(arg0.d64) >= std::bit_cast<int64_t>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::FLOAT)
            {
                if (std::bit_cast<int64_t>(arg0.d64) >= std::bit_cast<double>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::FLOAT:
        {
            if (arg1.type != VALUE_TYPE::INT && arg1.type != VALUE_TYPE::FLOAT && arg1.type != VALUE_TYPE::NIL)
            {
                return ret;
            }

            if (arg1.type == VALUE_TYPE::NIL)
            {
                if (std::bit_cast<double>(arg0.d64) >= 0)
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::INT)
            {
                if (std::bit_cast<double>(arg0.d64) >= std::bit_cast<int64_t>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::FLOAT)
            {
                if (std::bit_cast<double>(arg0.d64) >= std::bit_cast<double>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::NIL:
        {
            if (arg1.type != VALUE_TYPE::INT && arg1.type != VALUE_TYPE::FLOAT && arg1.type != VALUE_TYPE::NIL)
            {
                return ret;
            }

            if (arg1.type == VALUE_TYPE::NIL)
            {
                return ret;
            }

            if (arg1.type == VALUE_TYPE::INT)
            {
                if (0LL >= std::bit_cast<int64_t>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }

            if (arg1.type == VALUE_TYPE::FLOAT)
            {
                if (0.0 >= std::bit_cast<double>(arg1.d64))
                {
                    return ret;
                }
                ret.d64 = 1LL;
                return ret;
            }
            return ret;
        }
        default:
            Logger::Error("Syntax Error: Equals function only handles strings, ints, floats or null.", {});
            errored = true;
            return ret;
        }
    }

    const std::unordered_map<std::string, std::function<Variant(std::vector<Variant> &, bool &)>> BUILTIN_MAP{
        {"Print", Print},
        {"GetLine", GetLine},
        {"GetChar", GetChar},
        {"ToString", ToString},
        {"StrFromChar", StrFromChar},
        {"AddI", AddI},
        {"AddF", AddF},
        {"Add", Add},
        {"MulI", MulI},
        {"MulF", MulF},
        {"Mul", Mul},
        {"Equals", Equals},
        {"Greater", Greater},
        {"Lesser", Lesser},
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
