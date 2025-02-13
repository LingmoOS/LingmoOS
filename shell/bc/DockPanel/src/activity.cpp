/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "activity.h"
#include "docksettings.h"

#include <NETWM>
#include <KWindowSystem>

static Activity *SELF = nullptr;

Activity *Activity::self()
{
    if (!SELF)
        SELF = new Activity;

    return SELF;
}

Activity::Activity(QObject *parent)
    : QObject(parent)
    , m_existsWindowMaximized(false)
{
    onActiveWindowChanged();

    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, this, &Activity::onActiveWindowChanged);
    connect(KWindowSystem::self(), static_cast<void (KWindowSystem::*)(WId)>(&KWindowSystem::windowChanged),
            this, &Activity::onActiveWindowChanged);
}

bool Activity::existsWindowMaximized() const
{
    return m_existsWindowMaximized;
}

bool Activity::launchPad() const
{
    return m_launchPad;
}
#include <QDebug>
void Activity::onActiveWindowChanged()
{
    KWindowInfo info(KWindowSystem::activeWindow(),
                     NET::WMState | NET::WMVisibleName,
                     NET::WM2WindowClass);

    bool launchPad = info.windowClassClass() == "lingmo-launcher";

    if (DockSettings::self()->visibility() == DockSettings::IntellHide) {
        bool existsWindowMaximized = false;

        for (WId wid : KWindowSystem::windows()) {
            KWindowInfo i(wid, NET::WMState, NET::WM2WindowClass);

            if (i.isMinimized() || i.hasState(NET::SkipTaskbar))
                continue;

            if (i.hasState(NET::MaxVert) || i.hasState(NET::MaxHoriz)) {
                existsWindowMaximized = true;
                break;
            }
        }

        if (m_existsWindowMaximized != existsWindowMaximized) {
            m_existsWindowMaximized = existsWindowMaximized;
            emit existsWindowMaximizedChanged();
        }
    }

    if (m_launchPad != launchPad) {
        m_launchPad = launchPad;
        emit launchPadChanged();
    }

    m_pid = info.pid();
    m_windowClass = info.windowClassClass().toLower();
}
