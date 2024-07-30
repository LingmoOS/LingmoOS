/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "waylandbackend.h"

#include "waylandconfig.h"
#include "waylandoutputdevice.h"

#include "kscreen_kwayland_logging.h"

#include <configmonitor.h>
#include <mode.h>
#include <output.h>

#include <QEventLoop>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>

using namespace KScreen;

WaylandBackend::WaylandBackend()
    : KScreen::AbstractBackend()
    , m_internalConfig(new WaylandConfig(this))
{
    qCDebug(KSCREEN_WAYLAND) << "Loading Wayland backend.";
    connect(m_internalConfig, &WaylandConfig::configChanged, this, [this] {
        Q_EMIT configChanged(m_internalConfig->currentConfig());
    });
}

QString WaylandBackend::name() const
{
    return QStringLiteral("kwayland");
}

QString WaylandBackend::serviceName() const
{
    return QStringLiteral("org.kde.KScreen.Backend.KWayland");
}

ConfigPtr WaylandBackend::config() const
{
    // Note: This should ONLY be called from GetConfigOperation!
    return m_internalConfig->currentConfig();
}

void WaylandBackend::setConfig(const KScreen::ConfigPtr &newconfig)
{
    if (!newconfig) {
        return;
    }
    // wait for KWin reply
    QEventLoop loop;

    connect(m_internalConfig, &WaylandConfig::configChanged, &loop, &QEventLoop::quit);
    m_internalConfig->applyConfig(newconfig);

    loop.exec();
}

QByteArray WaylandBackend::edid(int outputId) const
{
    WaylandOutputDevice *output = m_internalConfig->outputMap().value(outputId);
    if (!output) {
        return QByteArray();
    }
    return output->edid();
}

bool WaylandBackend::isValid() const
{
    return m_internalConfig->isReady();
}

#include "moc_waylandbackend.cpp"
