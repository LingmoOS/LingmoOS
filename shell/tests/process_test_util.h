// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <algorithm>
#include <cstdlib>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <unistd.h>
#include <utility>
#include <vector>

#include "base/process.h"

// Client programs started by tests, on the loop loop_test_util.h runs.
namespace Lingmo::Test {

// This process's environment without `removed`, with `added` set over it, as
// `NAME=value` entries for base::Process::setEnvironment.
inline std::vector<std::string> environmentWith(const std::vector<std::string_view> &removed,
                                                const std::map<std::string, std::string> &added)
{
    std::vector<std::string> result;
    for (char **entry = environ; *entry; ++entry) {
        const std::string_view variable(*entry);
        const std::string name(variable.substr(0, variable.find('=')));
        if (std::find(removed.begin(), removed.end(), name) == removed.end() && !added.contains(name))
            result.emplace_back(variable);
    }
    for (const auto &[name, value] : added)
        result.push_back(name + '=' + value);
    return result;
}

// Sets a variable of this process and puts the old value back when it goes.
class ScopedEnvironmentVariable
{
public:
    ScopedEnvironmentVariable(const char *name, const std::string &value)
        : m_name(name)
    {
        if (const char *previous = std::getenv(name))
            m_previous = previous;
        setenv(name, value.c_str(), 1);
    }

    ~ScopedEnvironmentVariable()
    {
        if (m_previous)
            setenv(m_name, m_previous->c_str(), 1);
        else
            unsetenv(m_name);
    }

    ScopedEnvironmentVariable(const ScopedEnvironmentVariable &) = delete;
    ScopedEnvironmentVariable &operator=(const ScopedEnvironmentVariable &) = delete;

private:
    const char *m_name;
    std::optional<std::string> m_previous;
};

inline bool contains(std::string_view text, std::string_view part)
{
    return text.find(part) != std::string_view::npos;
}

// A program whose output and exit the test reads; killed and waited for when
// it goes, if still running.
class ChildProcess
{
public:
    ChildProcess()
    {
        process.onFinished = [this](int code, bool crashedBySignal) {
            exitCode = code;
            crashed = crashedBySignal;
        };
    }

    // An empty environment keeps this process's.
    bool start(const std::string &program, const std::vector<std::string> &arguments,
               std::vector<std::string> childEnvironment = {})
    {
        environment = childEnvironment;
        process.setEnvironment(std::move(childEnvironment));
        return process.start(program, arguments);
    }

    bool running() const { return process.isRunning(); }

    bool saw(std::string_view marker) const
    {
        return process.standardOutput().find(marker) != std::string::npos;
    }

    const std::string &output() const { return process.standardOutput(); }
    const std::string &errors() const { return process.standardError(); }

    // The output written since the last call.
    std::string readOutput()
    {
        std::string fresh = output().substr(std::min(m_read, output().size()));
        m_read = output().size();
        return fresh;
    }

    base::Process process;
    std::vector<std::string> environment;
    int exitCode = -1;
    bool crashed = false;

private:
    size_t m_read = 0;
};

} // namespace Lingmo::Test
