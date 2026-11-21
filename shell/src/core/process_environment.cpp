// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/process_environment.h"

#include <unistd.h>

namespace Lingmo {

std::vector<std::string> childEnvironment()
{
    std::vector<std::string> environment;
    for (char **entry = environ; *entry; ++entry)
        environment.emplace_back(*entry);
    return environment;
}

} // namespace Lingmo
