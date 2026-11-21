// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/command_line.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <optional>

namespace Lingmo::base {

CommandLineOption::CommandLineOption(std::string name, std::string description, std::string valueName,
                                     std::string defaultValue)
    : CommandLineOption(std::vector<std::string>{std::move(name)}, std::move(description), std::move(valueName),
                        std::move(defaultValue))
{
}

CommandLineOption::CommandLineOption(std::vector<std::string> names, std::string description,
                                     std::string valueName, std::string defaultValue)
    : names(std::move(names))
    , description(std::move(description))
    , valueName(std::move(valueName))
{
    if (!defaultValue.empty())
        defaultValues.push_back(std::move(defaultValue));
}

void CommandLineParser::addHelpOption()
{
    if (addOption(CommandLineOption(std::vector<std::string>{"h", "help"}, "Displays help on command line options.")))
        m_helpIndex = int(m_options.size()) - 1;
}

void CommandLineParser::addVersionOption()
{
    if (addOption(CommandLineOption(std::vector<std::string>{"v", "version"}, "Displays version information.")))
        m_versionIndex = int(m_options.size()) - 1;
}

bool CommandLineParser::addOption(CommandLineOption option)
{
    if (option.names.empty())
        return false;
    for (const std::string& name : option.names) {
        if (name.empty() || name.front() == '-' || find(name) >= 0)
            return false;
    }
    m_options.push_back(std::move(option));
    return true;
}

void CommandLineParser::addPositionalArgument(std::string name, std::string description, std::string syntax)
{
    if (syntax.empty())
        syntax = name;
    m_positionalHelp.push_back({std::move(name), std::move(description), std::move(syntax)});
}

int CommandLineParser::find(std::string_view name) const
{
    for (size_t i = 0; i < m_options.size(); ++i) {
        if (std::find(m_options[i].names.begin(), m_options[i].names.end(), name) != m_options[i].names.end())
            return int(i);
    }
    return -1;
}

bool CommandLineParser::parse(int argc, char** argv)
{
    return parse(std::vector<std::string>(argv, argv + argc));
}

bool CommandLineParser::parse(const std::vector<std::string>& arguments)
{
    m_values.assign(m_options.size(), {});
    m_set.assign(m_options.size(), false);
    m_positional.clear();
    m_error.clear();
    m_program = arguments.empty() ? std::string() : std::filesystem::path(arguments.front()).filename().string();

    bool optionsEnded = false;
    for (size_t i = 1; i < arguments.size(); ++i) {
        const std::string& argument = arguments[i];
        if (optionsEnded || argument.size() < 2 || argument.front() != '-') {
            m_positional.push_back(argument);
            continue;
        }
        if (argument == "--") {
            optionsEnded = true;
            continue;
        }

        std::string name;
        std::optional<std::string> attached;
        if (argument.starts_with("--")) {
            const size_t equals = argument.find('=');
            name = argument.substr(2, equals == std::string::npos ? std::string::npos : equals - 2);
            if (equals != std::string::npos)
                attached = argument.substr(equals + 1);
        } else {
            name = argument.substr(1, 1);
            if (argument.size() > 2)
                attached = argument.substr(2);
        }

        const int index = find(name);
        if (index < 0) {
            m_error = "Unknown option '" + name + "'.";
            return false;
        }
        const CommandLineOption& option = m_options[size_t(index)];
        m_set[size_t(index)] = true;
        if (option.valueName.empty()) {
            if (attached && argument.starts_with("--")) {
                m_error = "Unexpected value after '" + argument.substr(0, argument.find('=')) + "'.";
                return false;
            }
            // Compacted one-letter flags: -abc.
            if (attached) {
                for (char c : *attached) {
                    const int flag = find(std::string(1, c));
                    if (flag < 0 || !m_options[size_t(flag)].valueName.empty()) {
                        m_error = "Unknown option '" + std::string(1, c) + "'.";
                        return false;
                    }
                    m_set[size_t(flag)] = true;
                }
            }
            continue;
        }
        if (attached) {
            m_values[size_t(index)].push_back(*attached);
        } else if (i + 1 < arguments.size()) {
            m_values[size_t(index)].push_back(arguments[++i]);
        } else {
            m_error = "Missing value after '" + argument + "'.";
            return false;
        }
    }
    return true;
}

void CommandLineParser::process(int argc, char** argv)
{
    if (!parse(argc, argv)) {
        std::fprintf(stderr, "%s\n", m_error.c_str());
        if (m_helpIndex >= 0)
            std::fprintf(stderr, "Try '%s --help' for more information.\n", m_program.c_str());
        std::exit(1);
    }
    if (m_helpIndex >= 0 && m_set[size_t(m_helpIndex)])
        showHelp(0);
    if (m_versionIndex >= 0 && m_set[size_t(m_versionIndex)])
        showVersion();
}

bool CommandLineParser::isSetByName(std::string_view name) const
{
    const int index = find(name);
    return index >= 0 && size_t(index) < m_set.size() && m_set[size_t(index)];
}

std::string CommandLineParser::valueByName(std::string_view name) const
{
    const std::vector<std::string> all = valuesByName(name);
    return all.empty() ? std::string() : all.back();
}

std::vector<std::string> CommandLineParser::valuesByName(std::string_view name) const
{
    const int index = find(name);
    if (index < 0)
        return {};
    if (size_t(index) < m_values.size() && !m_values[size_t(index)].empty())
        return m_values[size_t(index)];
    return m_options[size_t(index)].defaultValues;
}

std::string CommandLineParser::helpText() const
{
    std::string text = "Usage: " + m_program;
    if (!m_options.empty())
        text += " [options]";
    for (const Positional& positional : m_positionalHelp)
        text += " " + positional.syntax;
    text += "\n";
    if (!m_description.empty())
        text += m_description + "\n";

    std::vector<std::pair<std::string, std::string>> rows;
    for (const CommandLineOption& option : m_options) {
        std::string names;
        for (const std::string& name : option.names) {
            if (!names.empty())
                names += ", ";
            names += (name.size() == 1 ? "-" : "--") + name;
        }
        if (!option.valueName.empty())
            names += " <" + option.valueName + ">";
        std::string description = option.description;
        if (!option.defaultValues.empty())
            description += " [default: " + option.defaultValues.front() + "]";
        rows.emplace_back(std::move(names), std::move(description));
    }
    size_t width = 0;
    for (const auto& row : rows)
        width = std::max(width, row.first.size());
    for (const Positional& positional : m_positionalHelp)
        width = std::max(width, positional.name.size());

    const auto append = [&text, width](const std::string& left, const std::string& right) {
        text += "  " + left + std::string(width - left.size() + 2, ' ') + right + "\n";
    };
    if (!rows.empty()) {
        text += "\nOptions:\n";
        for (const auto& row : rows)
            append(row.first, row.second);
    }
    if (!m_positionalHelp.empty()) {
        text += "\nArguments:\n";
        for (const Positional& positional : m_positionalHelp)
            append(positional.name, positional.description);
    }
    return text;
}

void CommandLineParser::showHelp(int exitCode) const
{
    std::fputs(helpText().c_str(), stdout);
    std::fflush(stdout);
    std::exit(exitCode);
}

void CommandLineParser::showVersion() const
{
    std::printf("%s %s\n", m_program.c_str(), m_version.c_str());
    std::fflush(stdout);
    std::exit(0);
}

} // namespace Lingmo::base
