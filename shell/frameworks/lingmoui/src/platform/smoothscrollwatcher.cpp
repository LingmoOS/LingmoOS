/*
 *  SPDX-FileCopyrightText: 2024 Nathan Misner <nathan@infochunk.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "smoothscrollwatcher.h"

#ifdef LINGMOUI_ENABLE_DBUS
#include <QDBusConnection>
#endif

#include "lingmouiplatform_logging.h"

namespace LingmoUI
{
namespace Platform
{
Q_GLOBAL_STATIC(SmoothScrollWatcher, smoothScrollWatcherSelf)

SmoothScrollWatcher::SmoothScrollWatcher(QObject *parent)
    : QObject(parent)
{
#ifdef LINGMOUI_ENABLE_DBUS
    QDBusConnection::sessionBus().connect(QStringLiteral(""),
                                          QStringLiteral("/SmoothScroll"),
                                          QStringLiteral("org.kde.SmoothScroll"),
                                          QStringLiteral("notifyChange"),
                                          this,
                                          SLOT(setEnabled(bool)));
#endif
    m_enabled = true;
}

SmoothScrollWatcher::~SmoothScrollWatcher() = default;

bool SmoothScrollWatcher::enabled() const
{
    return m_enabled;
}

SmoothScrollWatcher *SmoothScrollWatcher::self()
{
    return smoothScrollWatcherSelf();
}

void SmoothScrollWatcher::setEnabled(bool value)
{
    m_enabled = value;
    Q_EMIT enabledChanged(value);
}

}
}

#include "moc_smoothscrollwatcher.cpp"
