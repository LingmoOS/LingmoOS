/*
    SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QProcess>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class Timer : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE void runCommand(const QString &command)
    {
        if (!command.isEmpty()) {
            QStringList split = QProcess::splitCommand(command);
            const QString program = split.takeFirst();
            QProcess::startDetached(program, split);
        }
    }
    /*!
     * Represent \a seconds as s string representing time duration with
     * given format based on QTime.
     */
    Q_INVOKABLE QString secondsToString(int seconds, const QString &format)
    {
        return QTime::fromMSecsSinceStartOfDay(seconds * 1000).toString(format);
    }
};

class TimerPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterSingletonType<Timer>(uri, 0, 1, "Timer", [](QQmlEngine *, QJSEngine *) {
            return new Timer();
        });
    }
};

#include "timerplugin.moc"
