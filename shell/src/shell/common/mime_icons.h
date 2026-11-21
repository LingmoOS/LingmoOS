// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <string>

namespace Lingmo::Shell {

// The icon name of a file's type, for the common cases: a folder; the type the
// shared-mime-info globs give its name, the heaviest and longest pattern
// first; else plain text or octet-stream by a look at its start. The icon is
// the "icons" file's, else the type with '/' as '-'. Touches the disk: the
// file thread's or a worker's.
std::string mimeIconNameForFile(const std::string &path);

} // namespace Lingmo::Shell

