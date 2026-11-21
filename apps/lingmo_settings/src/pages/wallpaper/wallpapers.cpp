// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/wallpaper/wallpapers.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <memory>

#include <unicode/ucol.h>

#include "base/paths.h"

namespace fs = std::filesystem;

namespace settings::wallpapers {

static bool isImage(const fs::path& path)
{
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return char(std::tolower(c)); });
    for (const char* known : {".jpg", ".jpeg", ".png", ".webp", ".bmp", ".tif", ".tiff", ".avif"}) {
        if (extension == known)
            return true;
    }
    return false;
}

// Hidden files and directories are left out, as Qt's directory listings do.
static void collect(const fs::path& directory, std::vector<std::string>& out)
{
    std::error_code error;
    fs::recursive_directory_iterator it(directory, error);
    for (; !error && it != fs::recursive_directory_iterator(); it.increment(error)) {
        const fs::path& path = it->path();
        if (path.filename().string().starts_with(".")) {
            if (it->is_directory(error))
                it.disable_recursion_pending();
            continue;
        }
        if (it->is_regular_file(error) && isImage(path))
            out.push_back(path.string());
    }
}

std::vector<std::string> system()
{
    const std::string userData = fs::path(Lingmo::base::paths::dataHome()).lexically_normal().string();
    std::vector<std::string> result;
    for (const std::string& dataPath : Lingmo::base::paths::dataDirectories()) {
        if (fs::path(dataPath).lexically_normal().string() == userData)
            continue;
        const fs::path directory = fs::path(dataPath) / "backgrounds/lingmoos";
        std::error_code error;
        if (fs::is_directory(directory, error))
            collect(directory, result);
    }
    std::vector<std::string> unique;
    for (std::string& path : result) {
        if (std::find(unique.begin(), unique.end(), path) == unique.end())
            unique.push_back(std::move(path));
    }
    sortByFileName(unique);
    return unique;
}

void sortByFileName(std::vector<std::string>& paths)
{
    // Numbers by value and case ignored, in the user's language.
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<UCollator, decltype(&ucol_close)> collator(ucol_open(nullptr, &status), ucol_close);
    if (U_FAILURE(status)) {
        std::sort(paths.begin(), paths.end());
        return;
    }
    ucol_setStrength(collator.get(), UCOL_SECONDARY);
    ucol_setAttribute(collator.get(), UCOL_NUMERIC_COLLATION, UCOL_ON, &status);
    std::stable_sort(paths.begin(), paths.end(), [&collator](const std::string& lhs, const std::string& rhs) {
        const std::string a = fs::path(lhs).filename().string();
        const std::string b = fs::path(rhs).filename().string();
        UErrorCode error = U_ZERO_ERROR;
        const UCollationResult order =
                ucol_strcollUTF8(collator.get(), a.data(), int32_t(a.size()), b.data(), int32_t(b.size()), &error);
        return order != UCOL_EQUAL ? order == UCOL_LESS : lhs < rhs;
    });
}

std::string folderTitle(const std::string& folder)
{
    std::string path = folder;
    while (!path.empty() && path.back() == '/')
        path.pop_back();
    const size_t slash = path.rfind('/');
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

} // namespace settings::wallpapers
