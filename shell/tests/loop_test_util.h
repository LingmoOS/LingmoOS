// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <chrono>
#include <functional>
#include <optional>

// Kept for tests that still reach these through this header.
#include "base/event_loop.h"
#include "base/timer.h"
#include "core/main_loop.h"

// Waiting in tests on gtest_toolkit_main.cpp: the main loop runs while they
// wait.
namespace Lingmo::Test {

// Runs the main loop until done() holds or the timeout passes; returns done().
bool runUntil(const std::function<bool()>& done,
              std::chrono::milliseconds timeout = std::chrono::seconds(5));

// Runs the main loop for the given time.
void runFor(std::chrono::milliseconds duration);

// Runs whatever is ready on the main loop once, without waiting.
void processPendingEvents();

} // namespace Lingmo::Test
