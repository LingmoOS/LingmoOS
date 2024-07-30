/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

int main(int argc, char **argv)
{
    QCommandLineParser parser;
    QApplication app(argc, argv); // because GHNS doesn't do it's own error reporting on installation failing..

    const QString description = i18n("SDDM theme installer");
    const char version[] = "1.0";

    app.setApplicationVersion(QString::fromLatin1(version));
    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription(description);
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("i") << QStringLiteral("install"), i18n("Install a theme.")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("u") << QStringLiteral("uninstall"), i18n("Uninstall a theme.")));

    parser.addPositionalArgument(QStringLiteral("themefile"), i18n("The theme to install, must be an existing archive file."));

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        qWarning() << "No theme file specified.";
        return 0;
    }

    if (parser.isSet(QStringLiteral("install"))) {
        const QFileInfo themefile(args.first());
        if (!themefile.exists()) {
            qWarning() << "Specified theme file does not exist";
            return 0;
        }

        KAuth::Action action(QStringLiteral("org.kde.kcontrol.kcmsddm.installtheme"));
        action.setHelperId(QStringLiteral("org.kde.kcontrol.kcmsddm"));

        QFile theme(themefile.absoluteFilePath());
        if (!theme.open(QIODevice::ReadOnly)) {
            qWarning() << "Unable to open file";
            return 0;
        }

        const QDBusUnixFileDescriptor themefileFd(theme.handle());

        action.addArgument(QStringLiteral("filedescriptor"), QVariant::fromValue(themefileFd));

        KAuth::ExecuteJob *job = action.execute();
        bool rc = job->exec();
        if (!rc) {
            QString errorString = job->errorString();
            if (!errorString.isEmpty()) {
                KMessageBox::error(nullptr, errorString, i18n("Unable to install theme"));
            }
            return -1;
        }

        KConfigGroup cg(KSharedConfig::openConfig(QStringLiteral("sddmthemeinstallerrc"), KConfig::SimpleConfig), QStringLiteral("DownloadedThemes"));
        cg.writeEntry(themefile.absoluteFilePath(), job->data().value(QStringLiteral("installedPaths")).toStringList());
        return 0;
    }
    if (parser.isSet(QStringLiteral("uninstall"))) {
        KConfigGroup cg(KSharedConfig::openConfig(QStringLiteral("sddmthemeinstallerrc"), KConfig::SimpleConfig), QStringLiteral("DownloadedThemes"));
        const QStringList installed = cg.readEntry(args.first(), QStringList());
        for (const QString &installedTheme : installed) {
            KAuth::Action action(QStringLiteral("org.kde.kcontrol.kcmsddm.uninstalltheme"));
            action.setHelperId(QStringLiteral("org.kde.kcontrol.kcmsddm"));
            action.addArgument(QStringLiteral("filePath"), installedTheme);
            KAuth::ExecuteJob *job = action.execute();
            // We want KNS to be able to tell if the entry was manually deleted, see BUG: 416255
            if (job->exec()) {
                QFile::remove(args.first());
                cg.deleteEntry(args.first());
            }
        }
        return 0;
    }
    qWarning() << "either install or uninstall must be passed as an argument";
    return -1;
}
