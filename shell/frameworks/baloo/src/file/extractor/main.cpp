/*
    This file is part of the Nepomuk KDE project.
    SPDX-FileCopyrightText: 2010-14 Vishesh Handa <handa.vish@gmail.com>
    SPDX-FileCopyrightText: 2010-2011 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "app.h"
#include "../priority.h"

#include <KCrash>
#include <KAboutData>
#include <KLocalizedString>
#include <QStandardPaths>

#include <QGuiApplication>
#include <QSessionManager>

int main(int argc, char* argv[])
{
    lowerIOPriority();
    setIdleSchedulingPriority();
    lowerPriority();

    QGuiApplication::setDesktopSettingsAware(false);
    QGuiApplication app(argc, argv);

    KAboutData aboutData(QStringLiteral("baloo"), i18n("Baloo File Extractor"), QLatin1String(PROJECT_VERSION));
    KAboutData::setApplicationData(aboutData);

    KCrash::initialize();

    app.setQuitOnLastWindowClosed(false);

    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };
    QObject::connect(&app, &QGuiApplication::commitDataRequest, disableSessionManagement);
    QObject::connect(&app, &QGuiApplication::saveStateRequest, disableSessionManagement);

    Baloo::App appObject;
    return app.exec();
}
