// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <vector>

#include "base/dbus.h"
#include "base/file_watcher.h"
#include "base/settings.h"

namespace Lingmo::Cursor {
class Cursor;

class Preferences
{
public:
    explicit Preferences(Cursor &cursor);

    void reload();

private:
    Cursor &m_cursor;
    base::Settings m_settings;
    base::Settings m_pointerSettings;
    base::FileWatch m_watch;
    std::vector<base::dbus::Match> m_appearanceSignals;
};
} // namespace Lingmo::Cursor
