/*
 *  SPDX-FileCopyrightText: 2023 Jakob Petsovits <jpetso@petsovits.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <powerdevilmigrateconfig.h>

#include <KSharedConfig>
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QTemporaryDir>
#include <QTimer>

using namespace Qt::StringLiterals;

int main(int argc, char **argv)
{
    // Set XDG_CONFIG_HOME to ~/.qttest/config (and other XDG homes), which we're using to
    // temporarily store and access our config file before moving it to its destination path.
    QStandardPaths::setTestModeEnabled(true);

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(u"migrateconfig"_s);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        u"Test helper: Migrate an existing Plasma 5 PowerDevil configuration its Plasma 6 equivalent, but write the output to a user-specified path."_s);

    parser.addOption(
        {u"src-powerdevilrc"_s,
         u"Path of an original global configuration file from Plasma 5 times. Doesn't have to be called powerdevilrc, but will be interpreted as such."_s,
         u"path"_s});
    parser.addOption({u"src-profilesrc"_s,
                      u"Path of an original profile configuration file from Plasma 5 times. Doesn't have to be called powermanagementprofilesrc, but will be "
                      "interpreted as such."_s,
                      u"path"_s});
    parser.addOption({u"dest-powerdevilrc"_s,
                      u"Path of the newly migrated config file for global settings. The PowerDevil daemon would overwrite the existing powerdevilrc, but this "
                      "tool can save it under any name."_s,
                      u"path"_s});
    parser.addOption({u"dest-profilesrc"_s,
                      u"Path of the newly migrated config file for profile-specific settings. The PowerDevil daemon would overwrite the existing "
                      "powermanagementprofilesrc, but this tool can save it under any name."_s,
                      u"path"_s});

    parser.addOption({u"assert-no-powerdevilrc-after-migration"_s, u"Abort with a non-zero error code if powerdevilrc exists after migrating."_s});

    parser.addOption({u"mobile"_s, u"Assume running on a mobile device (i.e. phones, tablets running Plasma Mobile) instead of regular desktop/laptop."_s});
    parser.addOption({u"vm"_s, u"Assume running in a virtual machine environment instead of bare metal."_s});
    parser.addOption({u"cannot-suspend"_s, u"Assume that the device does not support suspending to RAM a.k.a. Sleep."_s});

    parser.addHelpOption();
    parser.process(app);

    bool assertNoPowerdevilrcAfterMigration = parser.isSet(u"assert-no-powerdevilrc-after-migration"_s);
    bool isMobile = parser.isSet(u"mobile"_s);
    bool isVM = parser.isSet(u"vm"_s);
    bool canSuspend = !parser.isSet(u"cannot-suspend"_s);

    QString src_powerdevilrc_path = parser.value(u"src-powerdevilrc"_s);
    QString src_profilesrc_path = parser.value(u"src-profilesrc"_s);
    QString dest_powerdevilrc_path = parser.value(u"dest-powerdevilrc"_s);
    QString dest_profilesrc_path = parser.value(u"dest-profilesrc"_s);

    QString test_config_dir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QString temp_powerdevilrc_path = test_config_dir + u"/powerdevilrc";
    QString temp_profilesrc_path = test_config_dir + u"/powermanagementprofilesrc";

    QTimer::singleShot(0, [&] {
        // Successful or not, make sure we never end up with configs from a previous run.
        if (!dest_powerdevilrc_path.isEmpty()) {
            QFile::remove(dest_powerdevilrc_path);
        }
        if (!dest_profilesrc_path.isEmpty()) {
            QFile::remove(dest_profilesrc_path);
        }
        QFile::remove(temp_powerdevilrc_path);
        QFile::remove(temp_profilesrc_path);

        if (!src_powerdevilrc_path.isEmpty()) {
            if (!QFile::copy(src_powerdevilrc_path, temp_powerdevilrc_path)) {
                qDebug() << "Unable to copy config file to its temporary location.";
                app.exit(1);
            }
        }
        if (!src_profilesrc_path.isEmpty()) {
            if (!QFile::copy(src_profilesrc_path, temp_profilesrc_path)) {
                qDebug() << "Unable to copy config file to its temporary location.";
                app.exit(1);
            }
        }

        PowerDevil::migrateConfig(isMobile, isVM, canSuspend);

        if (assertNoPowerdevilrcAfterMigration && QFile::exists(temp_powerdevilrc_path)) {
            qDebug() << "Unexpected powerdevilrc exists after migration.";
            app.exit(1);
            return;
        }
        if (!dest_powerdevilrc_path.isEmpty()) {
            if (!QFile::rename(temp_powerdevilrc_path, dest_powerdevilrc_path)) {
                qDebug() << "Unable to move global config file to destination.";
                app.exit(1);
                return;
            }
        }
        if (!dest_profilesrc_path.isEmpty()) {
            if (!QFile::rename(temp_profilesrc_path, dest_profilesrc_path)) {
                qDebug() << "Unable to move profile config file to destination.";
                app.exit(1);
                return;
            }
        }

        app.exit(0);
    });
    return app.exec();
}
