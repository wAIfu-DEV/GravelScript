#pragma once

#include "../../flags.hpp"

namespace VersionRoute
{

    void DisplayVersionNumber()
    {
        const std::string HELP_MSG = GVS_VERSION "\n";
        std::cout << HELP_MSG;
    }
}