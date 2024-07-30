/*
    SPDX-FileCopyrightText: 2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QJSEngine>
#include <QJSValue>

#include <QFontMetrics>

#include <lingmoactivities/controller.h>

#include "../shellcorona.h"

namespace Lingmo
{
class Applet;
class Containment;
} // namespace Lingmo

class KLocalizedContext;

namespace WorkspaceScripting
{
class AppInterface;
class Containment;
class V1;

class ScriptEngine : public QJSEngine
{
    Q_OBJECT

public:
    explicit ScriptEngine(Lingmo::Corona *corona, QObject *parent = nullptr);
    ~ScriptEngine() override;

    QString errorString() const;

    static QStringList pendingUpdateScripts(Lingmo::Corona *corona);

    Lingmo::Corona *corona() const;
    QJSValue wrap(Lingmo::Applet *w);
    QJSValue wrap(Lingmo::Containment *c);
    virtual int defaultPanelScreen() const;
    QJSValue newError(const QString &message);

    static bool isPanel(const Lingmo::Containment *c);

    Lingmo::Containment *createContainment(const QString &type, const QString &plugin);

public Q_SLOTS:
    bool evaluateScript(const QString &script, const QString &path = QString());

Q_SIGNALS:
    void print(const QString &string);
    void printError(const QString &string);

private:
    void setupEngine();
    static QString onlyExec(const QString &commandLine);

    // Script API versions
    class V1;

    // helpers
    QStringList availableActivities() const;
    QList<Containment *> desktopsForActivity(const QString &id);
    Containment *createContainmentWrapper(const QString &type, const QString &plugin);

private Q_SLOTS:
    void exception(const QJSValue &value);

private:
    Lingmo::Corona *m_corona;
    ScriptEngine::V1 *m_globalScriptEngineObject;
    KLocalizedContext *m_localizedContext;
    AppInterface *m_appInterface;
    QJSValue m_scriptSelf;
    QString m_errorString;
};

static const int LINGMO_DESKTOP_SCRIPTING_VERSION = 20;
}
