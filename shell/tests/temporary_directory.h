// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_TESTS_TEMPORARY_DIRECTORY_H
#define LINGMO_TESTS_TEMPORARY_DIRECTORY_H

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include "base/file.h"

namespace Lingmo::Test {

// A directory under /tmp, removed with everything in it when this goes.
class TemporaryDirectory : public base::file::TemporaryDirectory
{
public:
    TemporaryDirectory()
        : base::file::TemporaryDirectory(std::filesystem::temp_directory_path().string(),
                                         "lingmo-test-")
    {
    }

    std::string filePath(std::string_view name) const { return path() + '/' + std::string(name); }

    // Writes the file, creating the directories it is in.
    bool write(std::string_view name, std::string_view contents) const
    {
        const std::filesystem::path file = filePath(name);
        std::error_code error;
        std::filesystem::create_directories(file.parent_path(), error);
        std::ofstream stream(file, std::ios::binary | std::ios::trunc);
        stream.write(contents.data(), std::streamsize(contents.size()));
        return bool(stream);
    }
};

} // namespace Lingmo::Test

#endif // LINGMO_TESTS_TEMPORARY_DIRECTORY_H
