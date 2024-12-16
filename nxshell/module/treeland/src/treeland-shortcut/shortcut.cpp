// Copyright (C) 2023 Dingyuan Zhang <zhangdingyuan@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "shortcut.h"

#include <systemd/sd-daemon.h>

#include <QApplication>
#include <QDBusInterface>
#include <QDir>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QStandardPaths>

static const QMap<QString, QString> SpecialKeyMap = {
    { "minus", "-" },      { "equal", "=" },     { "brackertleft", "[" }, { "breckertright", "]" },
    { "backslash", "\\" }, { "semicolon", ";" }, { "apostrophe", "'" },   { "comma", "," },
    { "period", "." },     { "slash", "/" },     { "grave", "`" },
};

static const QMap<QString, QString> SpecialRequireShiftKeyMap = {
    { "exclam", "!" },    { "at", "@" },          { "numbersign", "#" }, { "dollar", "$" },
    { "percent", "%" },   { "asciicircum", "^" }, { "ampersand", "&" },  { "asterisk", "*" },
    { "parenleft", "(" }, { "parenright", ")" },  { "underscore", "_" }, { "plus", "+" },
    { "braceleft", "{" }, { "braceright", "}" },  { "bar", "|" },        { "colon", ":" },
    { "quotedbl", "\"" }, { "less", "<" },        { "greater", ">" },    { "question", "?" },
    { "asciitilde", "~" }
};

QString transFromDaemonAccelStr(const QString &accelStr)
{
    if (accelStr.isEmpty()) {
        return accelStr;
    }

    QString str(accelStr);

    str.remove("<").replace(">", "+").replace("Control", "Ctrl").replace("Super", "Meta");

    for (auto it = SpecialKeyMap.constBegin(); it != SpecialKeyMap.constEnd(); ++it) {
        QString origin(str);
        str.replace(it.key(), it.value());
        if (str != origin) {
            return str;
        }
    }

    for (auto it = SpecialRequireShiftKeyMap.constBegin();
         it != SpecialRequireShiftKeyMap.constEnd();
         ++it) {
        QString origin(str);
        str.replace(it.key(), it.value());
        if (str != origin) {
            return str.remove("Shift+");
        }
    }

    return str;
}

ShortcutV1::ShortcutV1()
    : QWaylandClientExtensionTemplate<ShortcutV1>(1)
{
    connect(this, &ShortcutV1::activeChanged, this, [this] {
        qDebug() << isActive();

        if (isActive()) {
            // TODO: Use a converter
            const QString configDir =
                QString("%1/lingmo/ocean-daemon/keybinding/")
                    .arg(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first());
            const QString customIni = QString("%1/custom.ini").arg(configDir);

            auto updateShortcuts = [this, customIni] {
                for (auto *context : m_customShortcuts) {
                    delete context;
                }

                m_customShortcuts.clear();

                QSettings custom(customIni, QSettings::IniFormat);
                for (auto group : custom.childGroups()) {
                    const QString &action =
                        custom.value(QString("%1/Action").arg(group)).toString();
                    const QString &accels = transFromDaemonAccelStr(
                        custom.value(QString("%1/Accels").arg(group)).toString());
                    ShortcutContext *context =
                        new ShortcutContext(register_shortcut_context(accels));
                    m_customShortcuts.push_back(context);
                    connect(context, &ShortcutContext::shortcutHappended, this, [action] {
                        QProcess::startDetached(action);
                    });
                }
            };

            QFileSystemWatcher *watcher = new QFileSystemWatcher({ customIni }, this);
            connect(watcher, &QFileSystemWatcher::fileChanged, this, [updateShortcuts] {
                updateShortcuts();
            });

            updateShortcuts();

            QDir dir(TREELAND_DATA_DIR "/shortcuts");
            for (auto d : dir.entryInfoList(QDir::Filter::Files)) {
                auto shortcut = new Shortcut(d.filePath());
                ShortcutContext *context =
                    new ShortcutContext(register_shortcut_context(shortcut->shortcut()));
                connect(context, &ShortcutContext::shortcutHappended, this, [shortcut] {
                    shortcut->exec();
                });
            }
        }
    });
}

ShortcutContext::ShortcutContext(struct ::treeland_shortcut_context_v1 *object)
    : QWaylandClientExtensionTemplate<ShortcutContext>(1)
    , QtWayland::treeland_shortcut_context_v1(object)
{
}

ShortcutContext::~ShortcutContext()
{
    destroy();
}

void ShortcutContext::treeland_shortcut_context_v1_shortcut()
{
    emit shortcutHappended();
}

Shortcut::Shortcut(const QString &path)
    : m_settings(QSettings(path, QSettings::IniFormat))
{
}

void Shortcut::exec()
{
    const QString &type = m_settings.value("Shortcut/Type").toString();

    if (type == "Exec") {
        const QString &exec = m_settings.value("Type.Exec/Exec").toString();
        const QString &args = m_settings.value("Type.Exec/Args").toString();
        QProcess::startDetached(exec, args.split(" "));
    }

    if (type == "DBus") {
        const QString &service = m_settings.value("Type.DBus/Service").toString();
        const QString &path = m_settings.value("Type.DBus/Path").toString();
        const QString &interface = m_settings.value("Type.DBus/Interface").toString();
        const QString &method = m_settings.value("Type.DBus/Method").toString();

        QDBusInterface dbus(service, path, interface);
        dbus.asyncCall(method);
    }

    if (type == "Action") { }
}

QString Shortcut::shortcut()
{
    return m_settings.value("Shortcut/Shortcut").toString();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    new ShortcutV1;

    sd_notify(0, "READY=1");

    return app.exec();
}
