// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace Lingmo::base {

// An option: one-letter names are written -x, longer ones --name. With a
// value name the option takes a value, as `--name value` or `--name=value`
// (`-x value` or `-xvalue` for a one-letter name). Several names go in an
// explicit std::vector<std::string>: a nested brace list would also read as
// a character range.
struct CommandLineOption
{
    CommandLineOption(std::string name, std::string description = {}, std::string valueName = {},
                      std::string defaultValue = {});
    CommandLineOption(std::vector<std::string> names, std::string description = {}, std::string valueName = {},
                      std::string defaultValue = {});

    std::vector<std::string> names;
    std::string description;
    std::string valueName;
    // What value() gives when the option is not on the command line.
    std::vector<std::string> defaultValues;
};

// Parses options and positional arguments. Options and positional
// arguments may mix; everything after `--` is positional.
class CommandLineParser
{
public:
    void setApplicationDescription(std::string description) { m_description = std::move(description); }
    void setApplicationVersion(std::string version) { m_version = std::move(version); }

    // -h, --help.
    void addHelpOption();
    // -v, --version.
    void addVersionOption();
    // False when one of its names is taken already.
    bool addOption(CommandLineOption option);
    // Only for the help text; `syntax` defaults to the name.
    void addPositionalArgument(std::string name, std::string description, std::string syntax = {});

    // `arguments` start with the program. False on an unknown option or a
    // missing value, with errorText() saying which.
    bool parse(const std::vector<std::string>& arguments);
    bool parse(int argc, char** argv);
    // parse(), then prints the help or version and exits 0 when asked for
    // them, or prints the error and the hint to stderr and exits 1.
    void process(int argc, char** argv);

    bool isSet(std::string_view name) const { return isSetByName(name); }
    bool isSet(const char* name) const { return isSetByName(name); }
    bool isSet(const std::string& name) const { return isSetByName(name); }
    bool isSet(const CommandLineOption& option) const { return isSetByName(option.names.front()); }
    // The last value given, the default when none was, or empty.
    std::string value(std::string_view name) const { return valueByName(name); }
    std::string value(const char* name) const { return valueByName(name); }
    std::string value(const std::string& name) const { return valueByName(name); }
    std::string value(const CommandLineOption& option) const { return valueByName(option.names.front()); }
    // Every value given in order, or the defaults.
    std::vector<std::string> values(std::string_view name) const { return valuesByName(name); }
    std::vector<std::string> values(const char* name) const { return valuesByName(name); }
    std::vector<std::string> values(const std::string& name) const { return valuesByName(name); }
    std::vector<std::string> values(const CommandLineOption& option) const
    {
        return valuesByName(option.names.front());
    }

    const std::vector<std::string>& positionalArguments() const { return m_positional; }
    const std::string& errorText() const { return m_error; }
    const std::string& programName() const { return m_program; }

    std::string helpText() const;
    // Prints the help to stdout and exits.
    [[noreturn]] void showHelp(int exitCode = 0) const;
    [[noreturn]] void showVersion() const;

private:
    struct Positional
    {
        std::string name;
        std::string description;
        std::string syntax;
    };

    bool isSetByName(std::string_view name) const;
    std::string valueByName(std::string_view name) const;
    std::vector<std::string> valuesByName(std::string_view name) const;
    // Index into m_options, or -1.
    int find(std::string_view name) const;

    std::string m_description;
    std::string m_version;
    std::string m_program;
    std::vector<CommandLineOption> m_options;
    std::vector<Positional> m_positionalHelp;
    // Per option, the values given; an entry exists once the option is seen.
    std::vector<std::vector<std::string>> m_values;
    std::vector<bool> m_set;
    std::vector<std::string> m_positional;
    std::string m_error;
    int m_helpIndex = -1;
    int m_versionIndex = -1;
};

} // namespace Lingmo::base
