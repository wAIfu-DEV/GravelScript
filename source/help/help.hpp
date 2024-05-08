#pragma once

#include <iostream>

namespace HelpRoute
{
    void DisplayHelp()
    {
        const std::string HELP_MSG = "\n"
                                     "Usage: gvs <PATH> | <ARG>\n"
                                     "\n"
                                     "Args:\n"
                                     "\t-h : Shows the list of available arguments.\n"
                                     "\t-v : Show the version of the program.\n\n";
        std::cout << HELP_MSG;
    }
}