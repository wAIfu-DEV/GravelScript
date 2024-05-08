#pragma once

#include <unordered_map>
#include <filesystem>

#include "../types/error.hpp"
#include "../logger/logger.hpp"
#include "../assert/runtime_assert.hpp"
#include "../helper/helper.hpp"
#include "../global_state/global_state.hpp"

namespace Arguments
{
    const std::string ARG_HELP{"-h"};
    const std::string ARG_VERSION{"-v"};

    const std::unordered_map<std::string, bool> AVAILABLE_ARGS{
        {ARG_HELP, false},
        {ARG_VERSION, false},
    };

    Error Parse(const int32_t argc, char *argv[])
    {
        namespace fs = std::filesystem;

        if (argc <= 1)
        {
            Logger::Error("No arguments passed.", {});
            return Error::ASSERTION;
        }

        std::string previous_arg;
        bool next_arg = false;

        if (argc > 0)
        {
            Logger::Debug("Arg0:", {argv[0]});
        }

        for (int16_t i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];

            if (next_arg)
            {
                Global::args[previous_arg] = arg;
                continue;
            }

            // Is not available arg, may be path.
            if (!Helper::UnorderedMapHasKey(AVAILABLE_ARGS, arg))
            {
                try
                {
                    fs::path abs_path = fs::canonical(arg);
                    Global::args.insert_or_assign("PATH", abs_path.string());
                    next_arg = false;
                    continue;
                }
                catch (const std::exception &e)
                {
                    Logger::Error("Could not recognize argument:", {arg});
                    return Error::ASSERTION;
                }
            }
            else
            {
                next_arg = AVAILABLE_ARGS.at(arg);
            }

            Logger::Debug("Found argument:", {arg});
            Global::args[arg] = "";
            previous_arg = arg;
        }
        return Error::OK;
    }
};