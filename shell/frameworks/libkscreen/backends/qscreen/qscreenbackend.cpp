/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *  SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "qscreenbackend.h"
#include "qscreenconfig.h"

#include <config.h>

using namespace KScreen;

Q_LOGGING_CATEGORY(KSCREEN_QSCREEN, "kscreen.qscreen")

QScreenConfig *QScreenBackend::s_internalConfig = nullptr;

QScreenBackend::QScreenBackend()
    : KScreen::AbstractBackend()
    , m_isValid(true)
{
    if (s_internalConfig == nullptr) {
        s_internalConfig = new QScreenConfig();
        connect(s_internalConfig, &QScreenConfig::configChanged, this, &QScreenBackend::configChanged);
    }
}

QScreenBackend::~QScreenBackend()
{
}

QString QScreenBackend::name() const
{
    return QStringLiteral("QScreen");
}

QString QScreenBackend::serviceName() const
{
    return QStringLiteral("org.kde.KScreen.Backend.QScreen");
}

ConfigPtr QScreenBackend::config() const
{
    return s_internalConfig->toKScreenConfig();
}

void QScreenBackend::setConfig(const ConfigPtr &config)
{
    if (!config) {
        return;
    }

    qWarning() << "The QScreen backend for libkscreen is read-only,";
    qWarning() << "setting a configuration is not supported.";
    qWarning() << "You can force another backend using the KSCREEN_BACKEND env var.";
}

bool QScreenBackend::isValid() const
{
    return m_isValid;
}

#include "moc_qscreenbackend.cpp"
