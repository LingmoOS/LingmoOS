// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/desks/desks_restore_util.cc
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65

#include "wm/desks/desk_preferences.h"

#include <algorithm>
#include <string>
#include <vector>

#include "base/auto_reset.h"
#include "core/preferences.h"
#include "wm/desks/desk.h"

namespace Lingmo::WM {

namespace {

const char kCount[] = "Desks/Count";
const char kActive[] = "Desks/Active";
// One entry per desk, empty for a desk with its default name.
const char kNames[] = "Desks/Names";

} // namespace

DeskPreferences::DeskPreferences(DesksController *desks)
    : DeskPreferences(desks, preferencesFile("compositor"))
{
}

DeskPreferences::DeskPreferences(DesksController *desks, const std::string &file)
    : m_desks(desks)
    , m_settings(file)
{
    restore();
    m_desks->addObserver(this);
}

DeskPreferences::~DeskPreferences()
{
    m_desks->removeObserver(this);
}

void DeskPreferences::restore()
{
    const base::AutoReset restoring(m_restoring, true);
    const int count = std::clamp(int(m_settings.intValue(kCount, 1)), 1, DesksController::kMaxDesks);
    const std::vector<std::string> names = m_settings.stringListValue(kNames);
    while (int(m_desks->desks().size()) < count)
        m_desks->newDesk(DesksCreationRemovalSource::DesksRestore);
    for (int index = 0; index < count && index < int(names.size()); ++index) {
        if (!names[index].empty())
            m_desks->setDeskName(m_desks->deskAt(index), names[index]);
    }
    if (Desk *active = m_desks->deskAt(int(m_settings.intValue(kActive, 0))))
        m_desks->activateDesk(active, DesksSwitchSource::DeskRestored);
}

void DeskPreferences::save()
{
    if (m_restoring)
        return;
    std::vector<std::string> names;
    for (const auto &desk : m_desks->desks())
        names.push_back(desk->isNameSetByUser() ? desk->name() : std::string());
    m_settings.setValue(kCount, int(m_desks->desks().size()));
    m_settings.setValue(kActive, m_desks->activeDeskIndex());
    m_settings.setValue(kNames, names);
    m_settings.sync();
}

} // namespace Lingmo::WM
