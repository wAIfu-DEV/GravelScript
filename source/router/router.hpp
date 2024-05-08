#pragma once

#include "../types/error.hpp"

#include "../helper/helper.hpp"
#include "../global_state/global_state.hpp"

#include "../arguments/parse_arguments.hpp"
#include "../help/help.hpp"
#include "../version/version.hpp"
#include "../script/run_script.hpp"

namespace Router
{
    Error RouteBasedOnArguments()
    {

        if (Helper::UnorderedMapHasKey(Global::args, Arguments::ARG_HELP))
        {
            HelpRoute::DisplayHelp();
            return Error::OK;
        }

        if (Helper::UnorderedMapHasKey(Global::args, Arguments::ARG_VERSION))
        {
            VersionRoute::DisplayVersionNumber();
            return Error::OK;
        }

        if (Helper::UnorderedMapHasKey(Global::args, std::string{"PATH"}))
        {
            return Script::RunFile(Global::args.at("PATH"));
        }

        return Error::ASSERTION;
    }
}