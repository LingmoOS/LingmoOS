// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdexcept>
#include <string>

namespace Lingmo::Files {

struct Error
{
    // An errno value; 0 for none.
    int code = 0;
    std::string path;
    std::string message;

    explicit operator bool() const { return code != 0; }
};

class Failure : public std::runtime_error
{
public:
    Failure(int code, const std::string& path);
    // A message that says more than the code's own.
    Failure(int code, const std::string& path, const std::string& message);

    Error error;
};

} // namespace Lingmo::Files
