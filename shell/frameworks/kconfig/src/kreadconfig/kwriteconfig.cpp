/*  Write KConfig() entries - for use in shell scripts.

    SPDX-FileCopyrightText: 2001 Red Hat , Inc.
    SPDX-FileCopyrightText: 2001 Luís Pedro Coelho <luis_pedro@netcabo.pt>

    Programmed by Luís Pedro Coelho <luis_pedro@netcabo.pt>
    based on kreadconfig by Bernhard Rosenkraenzer <bero@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KConfig>
#include <KConfigGroup>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <stdio.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(
        QCommandLineOption(QStringLiteral("file"), QCoreApplication::translate("main", "Use <file> instead of global config"), QStringLiteral("file")));
    parser.addOption(
        QCommandLineOption(QStringLiteral("group"),
                           QCoreApplication::translate("main", "Group to look in. Use \"<default>\" for the root group, or use repeatedly for nested groups."),
                           QStringLiteral("group"),
                           QStringLiteral("KDE")));
    parser.addOption(QCommandLineOption(QStringLiteral("key"), QCoreApplication::translate("main", "Key to look for"), QStringLiteral("key")));
    parser.addOption(
        QCommandLineOption(QStringLiteral("type"),
                           QCoreApplication::translate("main", "Type of variable. Use \"bool\" for a boolean, otherwise it is treated as a string"),
                           QStringLiteral("type")));
    parser.addOption(QCommandLineOption(QStringLiteral("delete"), QCoreApplication::translate("main", "Delete the designated key if enabled")));
    parser.addPositionalArgument(QStringLiteral("value"), QCoreApplication::translate("main", "The value to write. Mandatory, on a shell use '' for empty"));

    parser.process(app);

    const QStringList groups = parser.values(QStringLiteral("group"));
    QString key = parser.value(QStringLiteral("key"));
    QString file = parser.value(QStringLiteral("file"));
    QString type = parser.value(QStringLiteral("type")).toLower();
    bool del = parser.isSet(QStringLiteral("delete"));

    QString value;
    if (del) {
        value = QString{};
    } else if (parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    } else {
        value = parser.positionalArguments().at(0);
    }

    KConfig *konfig;
    if (file.isEmpty()) {
        konfig = new KConfig(QStringLiteral("kdeglobals"), KConfig::NoGlobals);
    } else {
        konfig = new KConfig(file, KConfig::NoGlobals);
    }

    KConfigGroup cfgGroup = konfig->group(QString());
    for (const QString &grp : groups) {
        if (grp.isEmpty()) {
            fprintf(stderr,
                    "%s: %s\n",
                    qPrintable(QCoreApplication::applicationName()),
                    qPrintable(QCoreApplication::translate("main", "Group name cannot be empty, use \"<default>\" for the root group")));
            return 2;
        }
        cfgGroup = cfgGroup.group(grp);
    }

    if (konfig->accessMode() != KConfig::ReadWrite || cfgGroup.isEntryImmutable(key)) {
        return 2;
    }

    if (del) {
        cfgGroup.deleteEntry(key);
    } else if (type == QLatin1String{"bool"}) {
        // For symmetry with kreadconfig we accept a wider range of values as true than Qt
        /* clang-format off */
        bool boolvalue = value == QLatin1String{"true"}
                         || value == QLatin1String{"on"}
                         || value == QLatin1String{"yes"}
                         || value == QLatin1String{"1"}; /* clang-format on */
        cfgGroup.writeEntry(key, boolvalue);
    } else if (type == QLatin1String{"path"}) {
        cfgGroup.writePathEntry(key, value);
    } else {
        cfgGroup.writeEntry(key, value);
    }
    konfig->sync();
    delete konfig;
    return 0;
}
