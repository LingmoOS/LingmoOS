/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "config-kcm.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <qcommandlineoption.h>
#include <qcommandlineparser.h>

#include "ktar.h"
#include "kzip.h"
#include <KArchive>
#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KConfigGroup>
#include <KSharedConfig>
#include <klocalizedstring.h>

int main(int argc, char **argv)
{
    QCommandLineParser parser;
    QCoreApplication app(argc, argv);

    const QString description = i18n("Plymouth theme installer");

    app.setApplicationVersion(QStringLiteral(PROJECT_VERSION));
    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription(description);
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("i") << QStringLiteral("install"), i18n("Install a theme.")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("u") << QStringLiteral("uninstall"), i18n("Uninstall a theme.")));

    parser.addPositionalArgument(QStringLiteral("themefile"), i18n("The theme to install, must be an existing archive file."));

    parser.process(app);

    if (!parser.isSet(QStringLiteral("install")) && !parser.isSet(QStringLiteral("uninstall"))) {
        qWarning() << "You need to specify either install or uninstall operation..";
        return 0;
    }
    const QStringList args = parser.positionalArguments();

    if (args.isEmpty()) {
        qWarning() << "No theme file specified.";
        return 0;
    }

    QString themefile = args.first();
    themefile.replace(QStringLiteral("//"), QStringLiteral("/"));
    if (parser.isSet(QStringLiteral("install")) && !QFile::exists(themefile)) {
        qWarning() << "Specified theme file does not exists";
        return 0;
    }

    QVariantMap helperargs;
    helperargs[QStringLiteral("themearchive")] = themefile;

    // support uninstalling from an archive
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(themefile);
    bool isArchive = false;
    if (parser.isSet(QStringLiteral("uninstall"))) {
        QScopedPointer<KArchive> archive;
        if (mimeType.inherits(QStringLiteral("application/zip"))) {
            archive.reset(new KZip(themefile));
            // clang-format off
        } else if (mimeType.inherits(QStringLiteral("application/tar"))
            || mimeType.inherits(QStringLiteral("application/x-gzip"))
            || mimeType.inherits(QStringLiteral("application/x-bzip"))
            || mimeType.inherits(QStringLiteral("application/x-lzma"))
            || mimeType.inherits(QStringLiteral("application/x-xz"))
            || mimeType.inherits(QStringLiteral("application/x-bzip-compressed-tar"))
            || mimeType.inherits(QStringLiteral("application/x-compressed-tar"))) {
            archive.reset(new KTar(themefile));
            // clang-format on
        }
        if (archive) {
            isArchive = true;
            bool success = archive->open(QIODevice::ReadOnly);
            if (!success) {
                qCritical() << "Cannot open archive file '" << themefile << "'";
                exit(-1);
            }
            const KArchiveDirectory *dir = archive->directory();
            // if there is more than an item in the file,
            // plugin is a subdirectory with the same name as the file
            if (dir->entries().count() > 1) {
                helperargs[QStringLiteral("theme")] = QFileInfo(archive->fileName()).baseName();
            } else {
                helperargs[QStringLiteral("theme")] = dir->entries().constFirst();
            }
        } else {
            helperargs[QStringLiteral("theme")] = themefile;
        }
    }

    KAuth::Action action(parser.isSet(QStringLiteral("install")) ? QStringLiteral("org.kde.kcontrol.kcmplymouth.install")
                                                                 : QStringLiteral("org.kde.kcontrol.kcmplymouth.uninstall"));
    action.setHelperId(QStringLiteral("org.kde.kcontrol.kcmplymouth"));
    action.setArguments(helperargs);

    KAuth::ExecuteJob *job = action.execute();
    bool rc = job->exec();
    if (!rc) {
        qWarning() << i18n("Unable to authenticate/execute the action: %1, %2", job->error(), job->errorString());
        return -1;
    }

    KConfigGroup cg(KSharedConfig::openConfig(QStringLiteral("kplymouththemeinstallerrc")), QStringLiteral("DownloadedThemes"));
    if (parser.isSet(QStringLiteral("install"))) {
        cg.writeEntry(job->data().value(QStringLiteral("plugin")).toString(), themefile);
    } else {
        if (!isArchive) {
            // try to take the file name from the config file
            themefile = cg.readEntry(job->data().value(QStringLiteral("plugin")).toString(), QString());
        }

        if (themefile.isEmpty()) {
            // remove archive
            QFile(themefile).remove();
            // remove screenshot
            QFile::remove(QString(themefile + QStringLiteral(".png")));
        }

        cg.deleteEntry(job->data().value(QStringLiteral("plugin")).toString());
    }

    return app.exec();
}
