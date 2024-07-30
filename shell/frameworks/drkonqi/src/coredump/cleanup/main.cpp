/*
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <string_view>

using namespace std::chrono_literals;

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "Need cache path as argument\n";
        return 1;
    }

    const std::filesystem::path cachePath = argv[1];
    if (!std::filesystem::exists(cachePath)) {
        std::cerr << "Cache path doesn't exist " << cachePath << "\n";
        return 1;
    }

    for (const auto &path : std::filesystem::directory_iterator(cachePath)) {
        if (path.path().extension() != ".ini") {
            continue;
        }
        try {
            const auto lastModified = path.last_write_time();
            const auto now = decltype(lastModified)::clock().now();
            const auto age = now - lastModified;
            // Plenty of time so we won't take away the file from underneath drkonqi.
            if (age >= std::chrono::weeks(1)) {
                std::filesystem::remove(path);
            }
        } catch (const std::filesystem::filesystem_error &error) {
            std::cerr << error.what();
            continue;
        }
    }
}
