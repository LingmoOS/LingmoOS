/*
  SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
  SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
  SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

  SPDX-License-Identifier: GPL-2.0-or-later

*/

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QIcon>
#include <QRegularExpression>
#include <QStandardPaths>

#include <KAboutData>
#include <KAuthorized>
#include <KCModule>
#include <KCMultiDialog>
#include <KLocalizedString>
#include <KPageDialog>
#include <KPluginMetaData>
#include <KShell>
#include <kcmutils_debug.h>

#if __has_include(<KStartupInfo>)
#include <KStartupInfo>
#include <private/qtx11extras_p.h>
#define HAVE_X11 1
#endif

#if HAVE_QTDBUS
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#endif

#include <algorithm>
#include <iostream>

inline QList<KPluginMetaData> findKCMsMetaData()
{
    QList<KPluginMetaData> metaDataList = KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms"));
    metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/systemsettings"));
    metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/systemsettings_qwidgets"));
    metaDataList << KPluginMetaData::findPlugins(QStringLiteral("plasma/kcms/kinfocenter"));
    return metaDataList;
}

class KCMShellMultiDialog : public KCMultiDialog
{
    Q_OBJECT

public:
    /**
     * Constructor. Parameter @p dialogFace is passed to KCMultiDialog
     * unchanged.
     */
    explicit KCMShellMultiDialog(KPageDialog::FaceType dialogFace)
        : KCMultiDialog()
    {
        setFaceType(dialogFace);
        setModal(false);

#if HAVE_QTDBUS
        connect(this, &KCMShellMultiDialog::currentPageChanged, this, [](KPageWidgetItem *newPage) {
            if (KCModule *activeModule = newPage->widget()->findChild<KCModule *>()) {
                if (QDBusConnection::sessionBus().isConnected()
                    && QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.ActivityManager"))) {
                    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.ActivityManager"),
                                                                      QStringLiteral("/ActivityManager/Resources"),
                                                                      QStringLiteral("org.kde.ActivityManager.Resources"),
                                                                      QStringLiteral("RegisterResourceEvent"));

                    const QString appId = QStringLiteral("org.kde.systemsettings");
                    const uint winId = 0;
                    const QString url = QLatin1String("kcm:") + activeModule->metaData().pluginId();
                    const uint eventType = 0; // Accessed

                    msg.setArguments({appId, winId, url, eventType});

                    QDBusConnection::sessionBus().asyncCall(msg);
                }
            }
        });
#endif
    }
};

int main(int argc, char *argv[])
{
    const bool qpaVariable = qEnvironmentVariableIsSet("QT_QPA_PLATFORM");
    QApplication app(argc, argv);
    if (!qpaVariable) {
        // don't leak the env variable to processes we start
        qunsetenv("QT_QPA_PLATFORM");
    }
    KLocalizedString::setApplicationDomain("kcmshell6");
    KAboutData aboutData(QStringLiteral("kcmshell6"),
                         QString(),
                         QLatin1String(PROJECT_VERSION),
                         i18n("A tool to start single system settings modules"),
                         KAboutLicense::GPL,
                         i18n("(c) 1999-2023, The KDE Developers"));

    aboutData.addAuthor(i18n("Frans Englich"), i18n("Maintainer"), QStringLiteral("frans.englich@kde.org"));
    aboutData.addAuthor(i18n("Daniel Molkentin"), QString(), QStringLiteral("molkentin@kde.org"));
    aboutData.addAuthor(i18n("Matthias Hoelzer-Kluepfel"), QString(), QStringLiteral("hoelzer@kde.org"));
    aboutData.addAuthor(i18n("Matthias Elter"), QString(), QStringLiteral("elter@kde.org"));
    aboutData.addAuthor(i18n("Matthias Ettrich"), QString(), QStringLiteral("ettrich@kde.org"));
    aboutData.addAuthor(i18n("Waldo Bastian"), QString(), QStringLiteral("bastian@kde.org"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.addOption(QCommandLineOption(QStringLiteral("list"), i18n("List all possible modules")));
    parser.addPositionalArgument(QStringLiteral("module"), i18n("Configuration module to open"));
    parser.addOption(QCommandLineOption(QStringLiteral("args"), i18n("Space separated arguments for the module"), QLatin1String("arguments")));
    parser.addOption(QCommandLineOption(QStringLiteral("icon"), i18n("Use a specific icon for the window"), QLatin1String("icon")));
    parser.addOption(QCommandLineOption(QStringLiteral("caption"), i18n("Use a specific caption for the window"), QLatin1String("caption")));
    parser.addOption(QCommandLineOption(QStringLiteral("highlight"), i18n("Show an indicator when settings have changed from their default value")));

    parser.parse(app.arguments());
    aboutData.processCommandLine(&parser);

    parser.process(app);

    if (parser.isSet(QStringLiteral("list"))) {
        std::cout << i18n("The following modules are available:").toLocal8Bit().constData() << '\n';

        QList<KPluginMetaData> plugins = findKCMsMetaData();
        int maxLen = 0;

        for (const auto &plugin : plugins) {
            const int len = plugin.pluginId().size();
            maxLen = std::max(maxLen, len);
        }

        for (const auto &plugin : plugins) {
            QString comment = plugin.description();
            if (comment.isEmpty()) {
                comment = i18n("No description available");
            }

            const QString entry = QStringLiteral("%1 - %2").arg(plugin.pluginId().leftJustified(maxLen, QLatin1Char(' ')), comment);

            std::cout << entry.toLocal8Bit().constData() << '\n';
        }

        std::cout << std::endl;

        return 0;
    }

    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp();
        return -1;
    }

    QList<KPluginMetaData> metaDataList;

    QStringList args = parser.positionalArguments();
    args.removeDuplicates();
    for (const QString &arg : args) {
        if (KPluginMetaData data(arg, KPluginMetaData::AllowEmptyMetaData); data.isValid()) {
            metaDataList << data;
        } else {
            // Look in the namespaces for systemsettings/kinfocenter
            const static auto knownKCMs = findKCMsMetaData();
            const QStringList possibleIds{arg, QStringLiteral("kcm_") + arg, QStringLiteral("kcm") + arg};
            bool found = std::any_of(knownKCMs.begin(), knownKCMs.end(), [&possibleIds, &metaDataList](const KPluginMetaData &data) {
                bool idMatches = possibleIds.contains(data.pluginId());
                if (idMatches) {
                    metaDataList << data;
                }
                return idMatches;
            });
            if (!found) {
                metaDataList << KPluginMetaData(arg); // So that we show an error message in the dialog
                qCWarning(KCMUTILS_LOG) << "Could not find KCM with given Id" << arg;
            }
        }
    }
    if (metaDataList.isEmpty()) {
        return -1;
    }

    const bool multipleKCMs = metaDataList.size() > 1;
    KPageDialog::FaceType ftype = multipleKCMs ? KPageDialog::List : KPageDialog::Plain;
    auto dlg = new KCMShellMultiDialog(ftype);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    if (parser.isSet(QStringLiteral("caption"))) {
        dlg->setWindowTitle(parser.value(QStringLiteral("caption")));
    } else if (!multipleKCMs) { // We will have the "Configure" window title set by KCMultiDialog
        dlg->setWindowTitle(metaDataList.constFirst().name());
    }

    const QStringList argSplit = KShell::splitArgs(parser.value(QStringLiteral("args")));
    QVariantList pluginArgs(argSplit.begin(), argSplit.end());
    if (metaDataList.size() == 1) {
        KPageWidgetItem *item = dlg->addModule(*metaDataList.cbegin(), pluginArgs);
        // This makes sure the content area is focused by default
        item->widget()->setFocus(Qt::MouseFocusReason);
    } else {
        for (const KPluginMetaData &m : std::as_const(metaDataList)) {
            dlg->addModule(m, pluginArgs);
        }
    }

    if (parser.isSet(QStringLiteral("icon"))) {
        dlg->setWindowIcon(QIcon::fromTheme(parser.value(QStringLiteral("icon"))));
    } else {
        dlg->setWindowIcon(QIcon::fromTheme(metaDataList.constFirst().iconName()));
    }

    if (parser.isSet(QStringLiteral("highlight"))) {
        dlg->setDefaultsIndicatorsVisible(true);
    }

    if (app.desktopFileName() == QLatin1String("org.kde.kcmshell6")) {
        const QString path = metaDataList.constFirst().fileName();

        if (path.endsWith(QLatin1String(".desktop"))) {
            app.setDesktopFileName(path);
        } else {
            app.setDesktopFileName(metaDataList.constFirst().pluginId());
        }
    }

    dlg->show();

    app.exec();

    return 0;
}

#include "main.moc"
