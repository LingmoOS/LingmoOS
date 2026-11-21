// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/common/process_info.h"

#include <optional>
#include <vector>

#include "base/file.h"
#include "base/strings.h"
#include <string_view>

namespace Lingmo::Shell {

// python3.12, perl5, lua5.4 and the like: a name, then digits and dots.
static bool isScriptInterpreter(const std::string &name)
{
    for (const char *interpreter : {"python", "perl", "ruby", "lua"}) {
        const std::string_view prefix(interpreter);
        if (name.starts_with(prefix)
            && name.find_first_not_of("0123456789.", prefix.size()) == std::string::npos)
            return true;
    }
    for (const char *exact : {"node", "nodejs", "gjs", "sh", "bash", "dash"}) {
        if (name == exact)
            return true;
    }
    return false;
}

static std::string fileName(const std::string &path)
{
    const size_t slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

ProcessCommand processCommand(int64_t pid)
{
    if (pid <= 0)
        return {};

    const std::optional<std::string> cmdline = base::file::read("/proc/" + std::to_string(pid) + "/cmdline");
    if (!cmdline || cmdline->empty() || cmdline->front() == '\0')
        return {};

    const std::vector<std::string> arguments = base::strings::split(*cmdline, '\0');
    std::string program = arguments.front();

    if (isScriptInterpreter(fileName(program))) {
        program.clear();
        for (size_t i = 1; i < arguments.size(); ++i) {
            const std::string &argument = arguments[i];
            if (argument.empty() || argument == "-c" || argument == "-e" || argument == "-m")
                break;
            if (argument.starts_with('-'))
                continue;
            program = argument;
            break;
        }
        if (program.empty())
            return {};
    }

    return {program, fileName(program)};
}

int64_t parentProcess(int64_t pid)
{
    const std::optional<std::string> stat = base::file::read("/proc/" + std::to_string(pid) + "/stat");
    if (!stat)
        return 0;

    // The parent follows the state, after the name in parentheses that may
    // itself hold any character.
    const size_t close = stat->rfind(')');
    if (close == std::string::npos || close + 2 >= stat->size())
        return 0;
    const std::vector<std::string> fields = base::strings::split(std::string_view(*stat).substr(close + 2), ' ');
    if (fields.size() < 2)
        return 0;
    return base::strings::toNumber<int64_t>(fields[1]).value_or(0);
}

} // namespace Lingmo::Shell
