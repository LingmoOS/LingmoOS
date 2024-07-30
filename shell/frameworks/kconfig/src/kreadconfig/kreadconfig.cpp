/*  Read KConfig() entries - for use in shell scripts.

    SPDX-FileCopyrightText: 2001 Red Hat, Inc.
    SPDX-FileContributor: Programmed by Bernhard Rosenkraenzer <bero@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/*
 * If --type is specified as bool, the return value is 0 if the value
 * is set, 1 if it isn't set. There is no output.
 *
 * If --type is specified as num, the return value matches the value
 * of the key. There is no output.
 *
 * If --type is not set, the value of the key is simply printed to stdout.
 *
 * Usage examples:
 *	if kreadconfig6 --group KDE --key macStyle --type bool; then
 *		echo "We're using Mac-Style menus."
 *	else
 *		echo "We're using normal menus."
 *	fi
 *
 *	TRASH=`kreadconfig6 --group Paths --key Trash`
 *	if test -n "$TRASH"; then
 *		mv someFile "$TRASH"
 *	else
 *		rm someFile
 *	fi
 */

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QCommandLineParser>
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
    parser.addOption(QCommandLineOption(QStringLiteral("default"), QCoreApplication::translate("main", "Default value"), QStringLiteral("value")));
    parser.addOption(QCommandLineOption(QStringLiteral("type"), QCoreApplication::translate("main", "Type of variable"), QStringLiteral("type")));

    parser.process(app);

    const QStringList groups = parser.values(QStringLiteral("group"));
    QString key = parser.value(QStringLiteral("key"));
    QString file = parser.value(QStringLiteral("file"));
    QString dflt = parser.value(QStringLiteral("default"));
    QString type = parser.value(QStringLiteral("type")).toLower();

    if (key.isNull() || !parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    }

    KSharedConfig::openConfig();

    KConfig *konfig;
    bool configMustDeleted = false;
    if (file.isEmpty()) {
        konfig = KSharedConfig::openConfig().data();
    } else {
        konfig = new KConfig(file, KConfig::NoGlobals);
        configMustDeleted = true;
    }
    KConfigGroup cfgGroup = konfig->group(QString());
    for (const QString &grp : groups) {
        if (grp.isEmpty()) {
            fprintf(stderr,
                    "%s: %s\n",
                    qPrintable(QCoreApplication::applicationName()),
                    qPrintable(QCoreApplication::translate("main", "Group name cannot be empty, use \"<default>\" for the root group")));
            if (configMustDeleted) {
                delete konfig;
            }
            return 2;
        }
        cfgGroup = cfgGroup.group(grp);
    }

    if (type == QLatin1String{"bool"}) {
        dflt = dflt.toLower();
        bool def = (dflt == QLatin1String{"true"} || dflt == QLatin1String{"on"} || dflt == QLatin1String{"yes"} || dflt == QLatin1String{"1"});
        bool retValue = !cfgGroup.readEntry(key, def);
        if (configMustDeleted) {
            delete konfig;
        }
        return retValue;
    } else if (type == QLatin1String{"num"} || type == QLatin1String{"int"}) {
        int retValue = cfgGroup.readEntry(key, dflt.toInt());
        if (configMustDeleted) {
            delete konfig;
        }
        return retValue;
    } else if (type == QLatin1String{"path"}) {
        fprintf(stdout, "%s\n", cfgGroup.readPathEntry(key, dflt).toLocal8Bit().data());
        if (configMustDeleted) {
            delete konfig;
        }
        return 0;
    } else {
        /* Assume it's a string... */
        fprintf(stdout, "%s\n", cfgGroup.readEntry(key, dflt).toLocal8Bit().data());
        if (configMustDeleted) {
            delete konfig;
        }
        return 0;
    }
}
