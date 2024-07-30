// SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "dpms.h"
#include "kscreendpms_debug.h"
#include "waylanddpmshelper_p.h"
#include "xcbdpmshelper_p.h"

#include <QGuiApplication>
#include <QtGui/private/qtx11extras_p.h>

KScreen::Dpms::Dpms(QObject *parent)
    : QObject(parent)
{
    if (QX11Info::isPlatformX11()) {
        m_helper.reset(new XcbDpmsHelper);
    } else if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        m_helper.reset(new WaylandDpmsHelper);
    } else {
        qCWarning(KSCREEN_DPMS) << "dpms unsupported on this system";
        return;
    }

    connect(m_helper.data(), &AbstractDpmsHelper::supportedChanged, this, &Dpms::supportedChanged);
    connect(m_helper.data(), &AbstractDpmsHelper::modeChanged, this, &Dpms::modeChanged);
    connect(m_helper.data(), &AbstractDpmsHelper::hasPendingChangesChanged, this, &Dpms::hasPendingChangesChanged);
}

KScreen::Dpms::~Dpms()
{
}

void KScreen::Dpms::switchMode(KScreen::Dpms::Mode mode, const QList<QScreen *> &screens)
{
    m_helper->trigger(mode, screens.isEmpty() ? qGuiApp->screens() : screens);
}

bool KScreen::Dpms::isSupported() const
{
    return m_helper->isSupported();
}

bool KScreen::Dpms::hasPendingChanges() const
{
    return m_helper->hasPendingChanges();
}

#include "moc_dpms.cpp"
