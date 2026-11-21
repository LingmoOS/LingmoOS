// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoptions.h"
#include "regexp.h"

#include "base/file.h"
#include "base/strings.h"

#include <filesystem>
#include <map>

namespace strings = Lingmo::base::strings;

// The EditorConfig glob syntax as a regular expression: * stays within a
// path component, ** crosses them, {a,b} and [...] as in the shell.
static std::string globToRegex(std::string_view glob)
{
    std::string result;
    int braces = 0;
    for (size_t i = 0; i < glob.size(); ++i) {
        const char c = glob[i];
        if (c == '*') {
            if (i + 1 < glob.size() && glob[i + 1] == '*') {
                result += ".*";
                ++i;
            } else {
                result += "[^/]*";
            }
        } else if (c == '?') {
            result += "[^/]";
        } else if (c == '[') {
            const size_t end = glob.find(']', i + 1);
            if (end == std::string_view::npos) {
                result += "\\[";
            } else {
                std::string set(glob.substr(i + 1, end - i - 1));
                if (!set.empty() && set[0] == '!')
                    set[0] = '^';
                result += '[' + set + ']';
                i = end;
            }
        } else if (c == '{') {
            ++braces;
            result += "(?:";
        } else if (c == '}' && braces > 0) {
            --braces;
            result += ')';
        } else if (c == ',' && braces > 0) {
            result += '|';
        } else if (c == '/') {
            result += c;
        } else if (c == '\\' && i + 1 < glob.size()) {
            result += RegExp::escape(glob.substr(++i, 1));
        } else {
            result += RegExp::escape(glob.substr(i, 1));
        }
    }
    return result;
}

bool EditorConfig::globMatches(std::string_view glob, const std::string &relativePath)
{
    std::string pattern(glob);
    if (!pattern.empty() && pattern[0] == '/')
        pattern.erase(0, 1);
    else if (pattern.find('/') == std::string::npos)
        pattern.insert(0, "**/");
    std::string regex = globToRegex(pattern);
    // "**/" also matches no directory at all.
    regex = strings::replaceAll(regex, ".*/", "(?:.*/)?");
    const RegExp expression("\\A(?:" + regex + ")\\z");
    return expression.match(relativePath).hasMatch();
}

void EditorConfig::apply(std::string_view configText, const std::string &relativePath, bool *root)
{
    bool matches = false;
    bool preamble = true;
    for (const std::string_view raw : strings::splitViews(configText, '\n')) {
        const std::string_view line = strings::trimmed(raw);
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
        if (line.front() == '[' && line.back() == ']') {
            preamble = false;
            matches = globMatches(line.substr(1, line.size() - 2), relativePath);
            continue;
        }
        const size_t equals = line.find('=');
        if (equals == std::string_view::npos)
            continue;
        const std::string key = strings::toAsciiLower(strings::trimmed(line.substr(0, equals)));
        const std::string value = strings::toAsciiLower(strings::trimmed(line.substr(equals + 1)));
        if (preamble) {
            if (key == "root" && root)
                *root = value == "true";
            continue;
        }
        if (!matches)
            continue;

        const std::optional<int> size = strings::toNumber<int>(value);
        if (key == "indent_style") {
            if (value == "space" || value == "tab")
                insertSpaces = value == "space";
        } else if (key == "indent_size") {
            if (size && *size > 0 && *size <= 16)
                indentSize = *size;
            else if (value == "tab")
                indentSize.reset();
        } else if (key == "tab_width") {
            if (size && *size > 0 && *size <= 16)
                tabWidth = *size;
        } else if (key == "end_of_line") {
            if (value == "lf")
                lineEnding = LineEnding::Unix;
            else if (value == "crlf")
                lineEnding = LineEnding::Windows;
            else if (value == "cr")
                lineEnding = LineEnding::ClassicMac;
        } else if (key == "charset") {
            charsetBom = value == "utf-8-bom";
            if (value == "utf-8" || value == "utf-8-bom")
                charset = "UTF-8";
            else if (value == "utf-16le")
                charset = "UTF-16LE";
            else if (value == "utf-16be")
                charset = "UTF-16BE";
            else if (value == "latin1")
                charset = "ISO-8859-1";
        } else if (key == "trim_trailing_whitespace") {
            trimTrailingWhitespace = value == "true";
        } else if (key == "insert_final_newline") {
            insertFinalNewline = value == "true";
        }
    }
}

EditorConfig EditorConfig::forFile(const std::string &path)
{
    namespace fs = std::filesystem;
    std::error_code error;
    const fs::path absolute = fs::absolute(fs::path(path), error).lexically_normal();

    // Nearer files override farther ones, so they are applied last.
    std::vector<std::pair<fs::path, std::string>> configs;
    fs::path dir = absolute.parent_path();
    while (true) {
        const fs::path config = dir / ".editorconfig";
        if (fs::is_regular_file(config, error) && fs::file_size(config, error) < 1024 * 1024) {
            if (std::optional<std::string> text = Lingmo::base::file::read(config.string())) {
                bool root = false;
                EditorConfig probe;
                probe.apply(*text, std::string(), &root);
                configs.insert(configs.begin(), {dir, std::move(*text)});
                if (root)
                    break;
            }
        }
        if (!dir.has_parent_path() || dir.parent_path() == dir)
            break;
        dir = dir.parent_path();
    }

    EditorConfig result;
    for (const auto &[folder, text] : configs) {
        const std::string relative = absolute.lexically_relative(folder).string();
        result.apply(text, relative, nullptr);
    }
    if (result.indentSize && !result.tabWidth)
        result.tabWidth = result.indentSize;
    return result;
}

std::optional<Indentation> FileOptions::detectIndentation(const std::vector<std::string> &lines)
{
    int tabs = 0;
    int spaces = 0;
    std::map<int, int> steps;
    int previous = 0;
    const size_t limit = std::min<size_t>(lines.size(), 2000);
    for (size_t i = 0; i < limit; ++i) {
        const std::string &line = lines[i];
        if (strings::trimmed(line).empty())
            continue;
        if (line[0] == '\t') {
            ++tabs;
            previous = 0;
            continue;
        }
        int indent = 0;
        while (indent < int(line.size()) && line[size_t(indent)] == ' ')
            ++indent;
        // A " * " comment continuation is not indentation.
        if (indent > 0 && indent < int(line.size()) && line[size_t(indent)] == '*')
            continue;
        if (indent > 0)
            ++spaces;
        const int step = indent - previous;
        if (step >= 2 && step <= 8)
            ++steps[step];
        previous = indent;
    }

    if (tabs + spaces < 3)
        return std::nullopt;
    if (tabs > spaces)
        return Indentation{false, 0};

    int best = 0;
    int bestCount = 0;
    for (const auto &[step, count] : steps) {
        if (count > bestCount) {
            best = step;
            bestCount = count;
        }
    }
    if (best == 0)
        return std::nullopt;
    return Indentation{true, best};
}
