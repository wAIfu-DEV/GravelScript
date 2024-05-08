#pragma once

#include <cstdint>
#include <cstdio>

#include "../types/array.hpp"
#include "../assert/runtime_assert.hpp"

namespace Logger
{
    namespace
    {
        void PrintWithPrefix(const std::string &prefix, const std::string &arg0, const Array<std::string> &args)
        {
            std::cout << prefix << ": " << arg0;
            for (size_t i = 0; i < args.size; ++i)
            {
                std::cout << " " << args[i];
            }
            std::cout << "\n";
        }
    }

    void Print(const std::string &arg0, const Array<std::string> &args)
    {
        std::cout << arg0;
        for (size_t i = 0; i < args.size; ++i)
        {
            std::cout << " " << args[i];
        }
        std::cout << "\n";
    }

    void Error(const std::string &arg0, const Array<std::string> &args)
    {
        PrintWithPrefix("ERROR", arg0, args);
    }

    void Warning(const std::string &arg0, const Array<std::string> &args)
    {
        PrintWithPrefix("WARNING", arg0, args);
    }

    void Info(const std::string &arg0, const Array<std::string> &args)
    {
        PrintWithPrefix("INFO", arg0, args);
    }

    void Debug([[maybe_unused]] const std::string &arg0, [[maybe_unused]] const Array<std::string> &args)
    {
#if !GVS_RELEASE
        PrintWithPrefix("DEBUG", arg0, args);
#endif
    }
};