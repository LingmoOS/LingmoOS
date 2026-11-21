// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H

#include <functional>
#include <string>

// The event loop and scratch folder the tests run in: session.json and the
// drafts go there, never to the user's data folder.
namespace TestSupport
{
const std::string &directory();
// Runs the event loop for `ms` milliseconds.
void wait(int ms);
// Runs the event loop until `done` holds or `timeoutMs` passes.
bool waitUntil(const std::function<bool()> &done, int timeoutMs);
}

#endif
