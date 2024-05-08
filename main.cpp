#include "flags.hpp"

#include <iostream>

#include "source/types/error.hpp"
#include "source/arguments/parse_arguments.hpp"
#include "source/router/router.hpp"

int32_t ManagedMain(const int32_t argc, char *argv[])
{
    Logger::Debug("Started Program.", {});

    if (Arguments::Parse(argc, argv))
    {
        Logger::Error("Failed to parse arguments.", {});
        return 1;
    }

    if (Router::RouteBasedOnArguments())
    {
        Logger::Error("Terminated program due to error.", {});
        return 1;
    }

    Logger::Debug("Successfuly Finished Program.", {});
    return 0;
}

int32_t main(int32_t argc, char *argv[])
{
    try
    {
        return ManagedMain(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        Logger::Debug("Terminated program due to interpreter error.", {});
        return 1;
    }
}