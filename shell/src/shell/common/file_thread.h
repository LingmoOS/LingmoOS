// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <string>

#include "base/settings.h"
#include "base/thread_pool.h"

namespace Lingmo::Shell {

// The one thread that reads and writes the shell's own files: histories,
// settings and caches. Work runs in the order it was handed over, so a write
// never overtakes an earlier one to the same file, and the main thread never
// waits on the disk.
base::ThreadPool &fileThread();

// Runs work() on the file thread and done(result) back on this thread's loop.
template<typename Work, typename Done>
base::Job runOnFileThread(Work work, Done done)
{
    return fileThread().run(std::move(work), std::move(done));
}

// Blocks until everything handed to the file thread so far has run: at exit,
// so the last writes reach the disk, and in tests.
void waitForFileThread();

// Changes keys of an INI file, other keys kept, on the file thread: the
// file as it is by then, with these changes on top. Several components share
// shell.conf; each touches only its own keys.
void writeSettings(const std::string &path, std::function<void(base::Settings &settings)> change);

} // namespace Lingmo::Shell

