// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// this header must come first so that it picks up the filesystem implementation
#include <ghc/fs_impl.hpp>

#include "filesystem.h"
#include <fstream>

namespace deepin_cross {

namespace {

template<class Stream>
void open_utf8_path_impl(Stream& stream, const fs::path& path, std::ios_base::openmode mode)
{
    stream.open(path.native().c_str(), mode);
}

} // namespace

void open_utf8_path(std::ifstream& stream, const fs::path& path, std::ios_base::openmode mode)
{
    open_utf8_path_impl(stream, path, mode);
}

void open_utf8_path(std::ofstream& stream, const fs::path& path, std::ios_base::openmode mode)
{
    open_utf8_path_impl(stream, path, mode);
}

void open_utf8_path(std::fstream& stream, const fs::path& path, std::ios_base::openmode mode)
{
    open_utf8_path_impl(stream, path, mode);
}

} // namespace deepin_cross
