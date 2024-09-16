// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <cstdio>
#include <iosfwd>
#include <ios>
#include <ghc/fs_fwd.hpp>

namespace deepin_cross {

namespace fs = ghc::filesystem;

void open_utf8_path(std::ifstream& stream, const fs::path& path,
                    std::ios_base::openmode mode = std::ios_base::in);
void open_utf8_path(std::ofstream& stream, const fs::path& path,
                    std::ios_base::openmode mode = std::ios_base::out);
void open_utf8_path(std::fstream& stream, const fs::path& path,
                    std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);

} // namespace deepin_cross


#endif // FILESYSTEM_H
