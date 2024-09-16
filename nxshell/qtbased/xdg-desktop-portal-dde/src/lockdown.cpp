// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lockdown.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(XdgDesktopDDELockDown, "xdg-dde-lockdown")

LockdownPortal::LockdownPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_printing(false)
    , m_save_to_disk(false)
    , m_application_handlers(false)
    , m_location(false)
    , m_camera(false)
    , m_microphone(false)
    , m_sound_output(false)
{
    qCDebug(XdgDesktopDDELockDown) << "lockdown init";
}
