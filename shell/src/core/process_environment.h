// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_CORE_PROCESS_ENVIRONMENT_H_
#define LINGMO_CORE_PROCESS_ENVIRONMENT_H_

#include <string>
#include <vector>

namespace Lingmo {

// The environment to start a child process from, as NAME=value entries: the
// compositor's current one.
std::vector<std::string> childEnvironment();

} // namespace Lingmo

#endif // LINGMO_CORE_PROCESS_ENVIRONMENT_H_
