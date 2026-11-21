// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/preferences.h"

#include "base/paths.h"

namespace Lingmo {

namespace {

std::string &testConfigHome()
{
    static std::string directory;
    return directory;
}

} // namespace

std::string preferencesDirectory()
{
    const std::string &test = testConfigHome();
    return (test.empty() ? base::paths::configHome() : test) + "/lingmo";
}

std::string preferencesFile(std::string_view process)
{
    return preferencesDirectory() + '/' + std::string(process) + ".conf";
}

void setConfigHomeForTesting(std::string configHome)
{
    testConfigHome() = std::move(configHome);
}

} // namespace Lingmo
