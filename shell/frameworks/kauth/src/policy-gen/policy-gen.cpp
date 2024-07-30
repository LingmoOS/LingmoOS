/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "policy-gen.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QSettings>
#include <QStringList>

#include <cerrno>
#include <cstdio>

using namespace std;

QList<Action> parse(QSettings &ini);
QMap<QString, QString> parseDomain(const QSettings &ini);

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        qCritical("Too few arguments");
        return 1;
    }

    QSettings ini(QFile::decodeName(argv[1]), QSettings::IniFormat);
    if (ini.status()) {
        qCritical("Error loading file: %s", argv[1]);
        return 1;
    }

    if (argc == 3) {
        // Support an optional 2nd argument pointing to the output file
        //
        // This is safer to use in build systems than
        // "kauth-policy-gen foo.actions > foo.policy" because when using a
        // redirection "foo.policy" is created even if kauth-policy-gen fails.
        // This means the first call to make fails, but a second call succeeds
        // because an empty "foo.policy" exists.
        if (!freopen(argv[2], "w", stdout)) {
            qCritical("Failed to open %s for writing: %s", argv[2], strerror(errno));
            return 1;
        }
    }

    output(parse(ini), parseDomain(ini));
}

QList<Action> parse(QSettings &ini)
{
    QList<Action> actions;

    // example: [org.kde.kcontrol.kcmfoo.save]
    const QRegularExpression actionExp(QRegularExpression::anchoredPattern(QStringLiteral("[0-9a-z]+(\\.[0-9a-z]+)*")));

    // example: Description[ca]=Mòdul de control del Foo.
    const QRegularExpression descriptionExp(QRegularExpression::anchoredPattern(QStringLiteral("description(?:\\[(\\w+)\\])?")),
                                            QRegularExpression::CaseInsensitiveOption);

    // example: Name[ca]=Mòdul de control del Foo
    const QRegularExpression nameExp(QRegularExpression::anchoredPattern(QStringLiteral("name(?:\\[(\\w+)\\])?")), QRegularExpression::CaseInsensitiveOption);

    // example: Policy=auth_admin
    const QRegularExpression policyExp(QRegularExpression::anchoredPattern(QStringLiteral("(?:yes|no|auth_self|auth_admin)")));

    const auto listChilds = ini.childGroups();
    for (const QString &name : listChilds) {
        Action action;

        if (name == QLatin1String("Domain")) {
            continue;
        }

        if (!actionExp.match(name).hasMatch()) {
            qCritical("Wrong action syntax: %s\n", name.toLatin1().data());
            exit(1);
        }

        action.name = name;
        ini.beginGroup(name);

        const auto listChildKeys = ini.childKeys();
        for (const QString &key : listChildKeys) {
            QRegularExpressionMatch match;
            if ((match = descriptionExp.match(key)).hasMatch()) {
                QString lang = match.captured(1);

                if (lang.isEmpty()) {
                    lang = QString::fromLatin1("en");
                }

                action.descriptions.insert(lang, ini.value(key).toString());

            } else if ((match = nameExp.match(key)).hasMatch()) {
                QString lang = match.captured(1);

                if (lang.isEmpty()) {
                    lang = QString::fromLatin1("en");
                }

                action.messages.insert(lang, ini.value(key).toString());

            } else if (key.toLower() == QLatin1String("policy")) {
                QString policy = ini.value(key).toString();
                if (!policyExp.match(policy).hasMatch()) {
                    qCritical("Wrong policy: %s", policy.toLatin1().data());
                    exit(1);
                }
                action.policy = policy;

            } else if (key.toLower() == QLatin1String("policyinactive")) {
                QString policyInactive = ini.value(key).toString();
                if (!policyExp.match(policyInactive).hasMatch()) {
                    qCritical("Wrong policy: %s", policyInactive.toLatin1().data());
                    exit(1);
                }
                action.policyInactive = policyInactive;

            } else if (key.toLower() == QLatin1String("persistence")) {
                QString persistence = ini.value(key).toString();
                if (persistence != QLatin1String("session") && persistence != QLatin1String("always")) {
                    qCritical("Wrong persistence: %s", persistence.toLatin1().data());
                    exit(1);
                }
                action.persistence = persistence;
            }
        }

        if (action.policy.isEmpty() || action.messages.isEmpty() || action.descriptions.isEmpty()) {
            qCritical("Missing option in action: %s", name.toLatin1().data());
            exit(1);
        }
        ini.endGroup();

        actions.append(action);
    }

    return actions;
}

QMap<QString, QString> parseDomain(const QSettings &ini)
{
    QMap<QString, QString> rethash;

    if (ini.childGroups().contains(QString::fromLatin1("Domain"))) {
        if (ini.contains(QString::fromLatin1("Domain/Name"))) {
            rethash[QString::fromLatin1("vendor")] = ini.value(QString::fromLatin1("Domain/Name")).toString();
        }
        if (ini.contains(QString::fromLatin1("Domain/URL"))) {
            rethash[QString::fromLatin1("vendorurl")] = ini.value(QString::fromLatin1("Domain/URL")).toString();
        }
        if (ini.contains(QString::fromLatin1("Domain/Icon"))) {
            rethash[QString::fromLatin1("icon")] = ini.value(QString::fromLatin1("Domain/Icon")).toString();
        }
    }

    return rethash;
}
