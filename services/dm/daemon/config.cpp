#include "config.h"

#include "base/settings.h"
#include "base/strings.h"

#include <unistd.h>

using namespace Lingmo;

namespace dm {

Config Config::load(const std::string &path)
{
    Config config;
    if (access(path.c_str(), F_OK) != 0)
        return config;

    const base::Settings settings(path);

    config.greeterVt = int(settings.intValue("Greeter/Vt", config.greeterVt));
    config.greeterUser = settings.stringValue("Greeter/User", config.greeterUser);
    if (settings.contains("Greeter/Command"))
        config.greeterCommand = splitCommand(settings.stringValue("Greeter/Command"));
    config.greeterService = settings.stringValue("Greeter/Service", config.greeterService);

    config.sessionVt = int(settings.intValue("Session/Vt", config.sessionVt));
    config.sessionService = settings.stringValue("Session/Service", config.sessionService);
    if (settings.contains("Session/Directories"))
        config.sessionDirectories = base::strings::split(settings.stringValue("Session/Directories"), ':',
                                                         base::strings::SplitBehavior::SkipEmptyParts);
    config.defaultSession = settings.stringValue("Session/Default", config.defaultSession);

    config.autologinUser = settings.stringValue("Autologin/User", config.autologinUser);
    config.autologinSession = settings.stringValue("Autologin/Session", config.autologinSession);
    config.autologinService = settings.stringValue("Autologin/Service", config.autologinService);

    config.runtimeDirectory = settings.stringValue("Daemon/RuntimeDirectory", config.runtimeDirectory);
    config.statePath = settings.stringValue("Daemon/StatePath", config.statePath);

    return config;
}

std::vector<std::string> splitCommand(const std::string &command)
{
    std::vector<std::string> arguments;
    std::string argument;
    int quotes = 0;
    bool inQuote = false;

    for (const char c : command) {
        if (c == '"') {
            // The third quote in a row is a literal one.
            if (++quotes == 3) {
                quotes = 0;
                argument += c;
            }
            continue;
        }
        if (quotes) {
            if (quotes == 1)
                inQuote = !inQuote;
            quotes = 0;
        }
        if (!inQuote && base::strings::isAsciiWhitespace(c)) {
            if (!argument.empty()) {
                arguments.push_back(argument);
                argument.clear();
            }
        } else {
            argument += c;
        }
    }
    if (!argument.empty())
        arguments.push_back(argument);

    return arguments;
}

} // namespace dm
