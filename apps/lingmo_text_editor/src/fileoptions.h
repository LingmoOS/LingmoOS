// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPTIONS_H
#define FILEOPTIONS_H

#include "textcodec.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The properties of the .editorconfig files above a file.
struct EditorConfig
{
    std::optional<bool> insertSpaces;
    std::optional<int> indentSize;
    std::optional<int> tabWidth;
    std::string charset;
    bool charsetBom = false;
    std::optional<LineEnding> lineEnding;
    std::optional<bool> trimTrailingWhitespace;
    std::optional<bool> insertFinalNewline;

    static EditorConfig forFile(const std::string &path);
    // One file's sections applied to `fileName`, relative to that file's directory.
    void apply(std::string_view configText, const std::string &relativePath, bool *root);
    static bool globMatches(std::string_view glob, const std::string &relativePath);
};

struct Indentation
{
    bool insertSpaces = true;
    int width = 4;
};

namespace FileOptions
{
// The indentation most lines use, or nothing when too few lines are indented.
std::optional<Indentation> detectIndentation(const std::vector<std::string> &lines);
}

#endif
