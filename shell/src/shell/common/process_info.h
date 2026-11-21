// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <string>

namespace Lingmo::Shell {

// The program a process runs, as /proc says it, for matching processes to
// their desktop entries. procfs is answered from memory, so these never wait
// on a disk.
struct ProcessCommand {
    std::string program;
    // The program's file name.
    std::string name;
};

// Empty when /proc cannot say. A script started through its shebang runs as
// the interpreter, which would match every application written in that
// language: the script stands for it instead, and inline code (-c, -e) or a
// module (-m) for nothing.
ProcessCommand processCommand(int64_t pid);

// The parent process, or 0.
int64_t parentProcess(int64_t pid);

} // namespace Lingmo::Shell

