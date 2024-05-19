#pragma once

#include <iostream>
#include <unordered_map>
#include <functional>
#include <cstdint>

#include "../logger/logger.hpp"
#include "../helper/helper.hpp"
#include "../types/variant.hpp"

#include "../make_variant/get_variant.hpp"

namespace BuiltinFuncs
{
    Variant AddI(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::NIL,
            .flags = {},
            .d64 = 0ULL,
        };

        VarInt result = 0LL;

        for (auto v : args)
        {
            switch (v.type)
            {
            case VALUE_TYPE::INT:
                result += VarGetInt(v);
                break;
            case VALUE_TYPE::FLOAT:
                result += static_cast<VarInt>(round(VarGetFloat(v)));
                break;
            default:
                Logger::Error("Type Error: AddI only takes arguments of type int or float.", {});
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
            .d64 = 0ULL,
        };

        VarFloat result = 0.0;

        for (auto v : args)
        {
            switch (v.type)
            {
            case VALUE_TYPE::INT:
                result += static_cast<VarFloat>(VarGetInt(v));
                break;
            case VALUE_TYPE::FLOAT:
                result += VarGetFloat(v);
                break;
            default:
                Logger::Error("Type Error: AddF only takes arguments of type int or float.", {});
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
            .d64 = 0ULL,
        };

        VarInt result = 0LL;
        bool first = true;

        for (auto v : args)
        {
            switch (v.type)
            {
            case VALUE_TYPE::INT:
                if (first)
                {
                    result = VarGetInt(v);
                    break;
                }
                result = result * VarGetInt(v);
                break;
            case VALUE_TYPE::FLOAT:
                if (first)
                {
                    result = VarGetFloat(v);
                    break;
                }
                result = static_cast<VarInt>(round(static_cast<VarFloat>(result) * VarGetFloat(v)));
                break;
            default:
                Logger::Error("Type Error: MulI only takes arguments of type int or float.", {});
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
            .d64 = 0ULL,
        };

        VarFloat result = 0.0;
        bool first = true;

        for (auto v : args)
        {
            switch (v.type)
            {
            case VALUE_TYPE::INT:
                if (first)
                {
                    result = static_cast<VarFloat>(VarGetInt(v));
                    break;
                }
                result = result * static_cast<VarFloat>(VarGetInt(v));
                break;
            case VALUE_TYPE::FLOAT:
                if (first)
                {
                    result = VarGetFloat(v);
                    break;
                }
                result = result * VarGetFloat(v);
                break;
            default:
                Logger::Error("Type Error: MulF only takes arguments of type int or float.", {});
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
            s = std::to_string(VarGetInt(arg0));
            break;
        case VALUE_TYPE::FLOAT:
            s = std::to_string(VarGetFloat(arg0));
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
            std::cout << VarGetString(as_str);
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
            Logger::Error("Type Error: StrFromChar function 1 argument of type int.", {});
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
                std::cout << VarGetString(arg);
            }
            else
            {
                Variant as_str = BuiltinFuncs::ToString({arg}, errored);
                if (errored)
                    return ret;
                std::cout << VarGetString(as_str);
            }
            first = false;
        }

        std::cout << "\n";
        return ret;
    }

    Variant Panic(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::NIL,
            .flags = {},
            .d64 = 0,
        };

        if (args.size() < 1)
        {
            Logger::Error("Syntax Error: Panic function takes at least 1 argument.", {});
            errored = true;
            return ret;
        }

        bool first = true;

        for (auto arg : args)
        {
            if (!first)
                std::cerr << " ";

            if (arg.type == VALUE_TYPE::STRING)
            {
                std::cerr << VarGetString(arg);
            }
            else
            {
                Variant as_str = BuiltinFuncs::ToString({arg}, errored);
                if (errored)
                    return ret;
                std::cerr << VarGetString(as_str);
            }
            first = false;
        }

        std::cerr << "\n";
        errored = true;
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
            std::string str0 = VarGetString(arg0);
            std::string str1 = VarGetString(arg1);

            if (str0 == str1)
            {
                ret.d64 = 1LL;
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::INT:
        {
            switch (arg1.type)
            {
            case VALUE_TYPE::NIL:
                if (VarGetInt(arg0) == 0LL)
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::INT:
                if (VarGetInt(arg0) == VarGetInt(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::FLOAT:
                if (VarGetInt(arg0) == VarGetFloat(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            default:
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::FLOAT:
        {
            switch (arg1.type)
            {
            case VALUE_TYPE::NIL:
                if (VarGetFloat(arg0) == 0.0)
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::INT:
                if (VarGetFloat(arg0) == VarGetInt(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::FLOAT:
                if (VarGetFloat(arg0) == VarGetFloat(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            default:
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::NIL:
        {
            switch (arg1.type)
            {
            case VALUE_TYPE::NIL:
                ret.d64 = 1LL;
                return ret;
            case VALUE_TYPE::INT:
                if (0LL == VarGetInt(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::FLOAT:
                if (0.0 == VarGetFloat(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            default:
                return ret;
            }
            return ret;
        }
        default:
            Logger::Error("Type Error: Equals function only handles strings, ints, floats or null.", {});
            errored = true;
            return ret;
        }
    }

    Variant NotEquals(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::INT,
            .flags = {},
            .d64 = 0,
        };

        if (args.size() < 2)
        {
            Logger::Error("Syntax Error: NotEquals function takes 2 arguments.", {});
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
            std::string str0 = VarGetString(arg0);
            std::string str1 = VarGetString(arg1);

            if (str0 != str1)
            {
                ret.d64 = 1LL;
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::INT:
        {
            switch (arg1.type)
            {
            case VALUE_TYPE::NIL:
                if (VarGetInt(arg0) != 0LL)
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::INT:
                if (VarGetInt(arg0) != VarGetInt(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::FLOAT:
                if (VarGetInt(arg0) != VarGetFloat(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            default:
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::FLOAT:
        {
            switch (arg1.type)
            {
            case VALUE_TYPE::NIL:
                if (VarGetFloat(arg0) != 0.0)
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::INT:
                if (VarGetFloat(arg0) != VarGetInt(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::FLOAT:
                if (VarGetFloat(arg0) != VarGetFloat(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            default:
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::NIL:
        {
            switch (arg1.type)
            {
            case VALUE_TYPE::NIL:
                ret.d64 = 0LL;
                return ret;
            case VALUE_TYPE::INT:
                if (0LL != VarGetInt(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::FLOAT:
                if (0.0 != VarGetFloat(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            default:
                return ret;
            }
            return ret;
        }
        default:
            Logger::Error("Type Error: NotEquals function only handles strings, ints, floats or null.", {});
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
            switch (arg1.type)
            {
            case VALUE_TYPE::NIL:
                if (VarGetInt(arg0) > 0)
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::INT:
                if (VarGetInt(arg0) > VarGetInt(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::FLOAT:
                if (VarGetInt(arg0) > VarGetFloat(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            default:
                break;
            }
            return ret;
        }
        case VALUE_TYPE::FLOAT:
        {
            switch (arg1.type)
            {
            case VALUE_TYPE::NIL:
                if (VarGetFloat(arg0) > 0)
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::INT:
                if (VarGetFloat(arg0) > VarGetInt(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::FLOAT:
                if (VarGetFloat(arg0) > VarGetFloat(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            default:
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::NIL:
        {
            switch (arg1.type)
            {
            case VALUE_TYPE::NIL:
                return ret;
            case VALUE_TYPE::INT:
                if (0LL > VarGetInt(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::FLOAT:
                if (0.0 > VarGetFloat(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            default:
                return ret;
            }
            return ret;
        }
        default:
            Logger::Error("Type Error: Greater function only handles strings, ints, floats or null.", {});
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
            switch (arg1.type)
            {
            case VALUE_TYPE::NIL:
                if (VarGetInt(arg0) < 0)
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::INT:
                if (VarGetInt(arg0) < VarGetInt(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::FLOAT:
                if (VarGetInt(arg0) < VarGetFloat(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            default:
                break;
            }
            return ret;
        }
        case VALUE_TYPE::FLOAT:
        {
            switch (arg1.type)
            {
            case VALUE_TYPE::NIL:
                if (VarGetFloat(arg0) < 0)
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::INT:
                if (VarGetFloat(arg0) < VarGetInt(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::FLOAT:
                if (VarGetFloat(arg0) < VarGetFloat(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            default:
                return ret;
            }
            return ret;
        }
        case VALUE_TYPE::NIL:
        {
            switch (arg1.type)
            {
            case VALUE_TYPE::NIL:
                return ret;
            case VALUE_TYPE::INT:
                if (0LL < VarGetInt(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            case VALUE_TYPE::FLOAT:
                if (0.0 < VarGetFloat(arg1))
                {
                    ret.d64 = 1LL;
                    return ret;
                }
                return ret;
            default:
                return ret;
            }
            return ret;
        }
        default:
            Logger::Error("Type Error: Lesser function only handles strings, ints, floats or null.", {});
            errored = true;
            return ret;
        }
    }

    Variant Len(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::NIL,
            .flags = {},
            .d64 = 0ULL,
        };

        if (args.size() < 1)
        {
            Logger::Error("Syntax Error: 'Len' function takes 1 argument.", {});
            errored = true;
            return ret;
        }

        Variant arg0 = args.at(0);

        if (arg0.type != VALUE_TYPE::ARRAY && arg0.type != VALUE_TYPE::STRING)
        {
            Logger::Error("Type Error: Argument 1 of function 'Len' must be of type 'array' or 'string'.", {});
            errored = true;
            return ret;
        }

        size_t size = 0ULL;

        if (arg0.type == VALUE_TYPE::ARRAY)
        {
            size = Memory::arrays.at(arg0.d64).size();
        }
        else if (arg0.type == VALUE_TYPE::STRING)
        {
            size = Memory::strings.at(arg0.d64).length();
        }

        if (size > static_cast<uint64_t>(INT64_MAX))
        {
            size = INT64_MAX;
        }
        ret.type = VALUE_TYPE::INT;
        ret.d64 = static_cast<VarInt>(size);
        return ret;
    }

    Variant At(std::vector<Variant> args, bool &errored)
    {
        Variant ret{
            .type = VALUE_TYPE::NIL,
            .flags = {},
            .d64 = 0ULL,
        };

        if (args.size() < 2)
        {
            Logger::Error("Syntax Error: 'At' function takes 2 arguments.", {});
            errored = true;
            return ret;
        }

        Variant arg0 = args.at(0);
        Variant arg1 = args.at(1);

        if (arg0.type != VALUE_TYPE::ARRAY && arg0.type != VALUE_TYPE::STRING)
        {
            Logger::Error("Type Error: Argument 1 of function 'At' must be of type 'array' or 'string'.", {});
            errored = true;
            return ret;
        }

        if (arg1.type != VALUE_TYPE::INT)
        {
            Logger::Error("Type Error: Argument 2 of function 'At' must be of type 'int'.", {});
            errored = true;
            return ret;
        }

        VarInt i = VarGetInt(arg1);

        if (i < 0)
        {
            Logger::Error("Runtime Error: Argument 2 of function 'At' must be a positive integer.", {});
            errored = true;
            return ret;
        }

        if (arg0.type == VALUE_TYPE::ARRAY)
        {
            VarArray arr = Memory::arrays.at(arg0.d64);

            if (static_cast<uint64_t>(i) >= arr.size())
            {
                Logger::Error("Runtime Error: Tried accessing element outside of array bounds using 'At' function.", {});
                errored = true;
                return ret;
            }

            ret = arr.at(i);
            return ret;
        }
        else if (arg0.type == VALUE_TYPE::STRING)
        {
            std::string s = Memory::strings.at(arg0.d64);

            if (static_cast<uint64_t>(i) >= s.length())
            {
                Logger::Error("Runtime Error: Tried accessing element outside of string bounds using 'At' function.", {});
                errored = true;
                return ret;
            }

            ret.type = VALUE_TYPE::INT;
            ret.d64 = static_cast<VarInt>(s.at(i));
            return ret;
        }
        return ret;
    }

    const std::unordered_map<std::string, std::function<Variant(std::vector<Variant> &, bool &)>> BUILTIN_MAP{
        {"Print", Print},
        {"Panic", Panic},
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
        {"NotEquals", NotEquals},
        {"Greater", Greater},
        {"Lesser", Lesser},
        {"At", At},
        {"Len", Len},
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
