/*
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kurlhandler_p.h"

#include <kguiaddons_debug.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QLocale>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>

static const char s_khelpcenter_exec[] = "khelpcenter";

static bool openWithKHelpCenter(const QUrl &url)
{
    const QString helpcenter = QStandardPaths::findExecutable(QString::fromLatin1(s_khelpcenter_exec));
    if (!helpcenter.isEmpty()) {
        QUrl u(url);
        if (u.path() == QLatin1Char('/')) {
            const QString appName = QCoreApplication::applicationName();
            u.setPath(appName);
        }

        QProcess::startDetached(helpcenter, QStringList(u.toString()));
        return true;
    }

    return false;
}

KUrlHandler::KUrlHandler(QObject *parent)
    : QObject(parent)
{
}

void KUrlHandler::openHelp(const QUrl &url) const
{
    if (openWithKHelpCenter(url)) {
        return;
    }

    const QUrl docUrl = concatDocsUrl(url);
    if (docUrl.isValid()) {
        QDesktopServices::openUrl(docUrl);
    } else {
        qCWarning(KGUIADDONS_LOG) << "Could not find a suitable handler for" << url.toString();
    }
}

QUrl KUrlHandler::concatDocsUrl(const QUrl &url) const
{
    if (QCoreApplication::organizationDomain() != QLatin1String("kde.org")) {
        return {};
    }

    // KHelpCenter is not available and it's a KDE application, open the docs at docs.kde.org
    // with the default web browser on the system

    QString path = url.path();
    const QString fragment = url.fragment();
    const QString common = QLatin1String("https://docs.kde.org/index.php?branch=stable5&language=") + QLocale().name();

    const QString appName = QCoreApplication::applicationName();

    // Special case for KCModules
    if (appName == QLatin1String("systemsettings") && path.startsWith(QLatin1String("/kcontrol"))) {
        // E.g. change "/kcontrol/fonts/index.html" to "&application=kcontrol/fonts&path=index.html"
        // docs.kde.org will resolve the url and add the proper package name, e.g. plasma-workspace:
        // https://docs.kde.org/stable5/en/plasma-workspace/kcontrol/fonts/index.html
        QString kcmAppName(path);
        kcmAppName.remove(0, 1); // Remove leading "/"
        const int idx = kcmAppName.indexOf(QLatin1String("/index.html"));
        if (idx > 0) {
            kcmAppName.truncate(idx);
        }

        // Some KCModules have a valid fragment, e.g. kcontrol/powerdevil/index.html#advanced-settings
        const QString tail = QLatin1String("index.html") + (!fragment.isEmpty() ? QLatin1Char('#') + fragment : QString{});

        return QUrl(common + QLatin1String("&application=") + kcmAppName + QLatin1String("&path=") + tail);
    }

    // E.g. "help:/" and appName is "okular", e.g. opening Help -> Okular HandBook
    if (path == QLatin1Char('/')) {
        return QUrl(common + QLatin1String("&application=") + appName + QLatin1String("&path=") + QLatin1String("index.html"));
    }

    // E.g. "help:/okular/configure.html", don't repeat "appName"; e.g. clicking Help button in
    // the "Settings -> Configure Okular" dialog
    const QString redundant = QLatin1Char('/') + appName + QLatin1Char('/');
    if (path.startsWith(redundant)) {
        path.remove(0, redundant.size());

        if (!fragment.isEmpty()) {
            // E.g. "help:/kinfocenter/index.html#kcm_memory", it's actually "kinfocenter/kcm_memory.html"
            if (path == QLatin1String("index.html")) {
                qCWarning(KGUIADDONS_LOG) << "X-DocPath entry in a .desktop file in" << appName << "is:" << appName + QLatin1String("/index.html#") + fragment
                                          << ", however it should be:" << appName + QLatin1Char('/') + fragment + QLatin1String(".html");

                path = fragment + QLatin1String(".html");
            } else {
                // E.g. "help:/okular/signatures.html#adding_digital_signatures"
                path += QLatin1Char('#') + fragment;
            }
        }

        return QUrl(common + QLatin1String("&application=") + appName + QLatin1String("&path=") + path);
    }

    return {};
}

Q_GLOBAL_STATIC(KUrlHandler, s_handler)

static void initializeGlobalSettings()
{
    QDesktopServices::setUrlHandler(QStringLiteral("help"), s_handler, "openHelp");
}

Q_COREAPP_STARTUP_FUNCTION(initializeGlobalSettings)

#include "moc_kurlhandler_p.cpp"
