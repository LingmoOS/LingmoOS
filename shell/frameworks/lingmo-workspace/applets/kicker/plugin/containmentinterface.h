/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QQuickItem>
#include <QUrl>

#include <KService>

namespace Lingmo
{
class Applet;
class Containment;
}

class ContainmentInterface : public QObject
{
    Q_OBJECT

public:
    enum Target {
        Desktop = 0,
        Panel,
        TaskManager,
    };

    Q_ENUM(Target)

    explicit ContainmentInterface(QObject *parent = nullptr);
    ~ContainmentInterface() override;

    static bool mayAddLauncher(QObject *appletInterface, Target target, const KService::Ptr &service = {});

    static Q_INVOKABLE bool hasLauncher(QObject *appletInterface, Target target, const KService::Ptr &service);

    static Q_INVOKABLE void addLauncher(QObject *appletInterface, Target target, const QString &entryPath);

    static Q_INVOKABLE QObject *screenContainment(QObject *appletInterface);
    static Q_INVOKABLE bool screenContainmentMutable(QObject *appletInterface);
    static Q_INVOKABLE void ensureMutable(Lingmo::Containment *containment);

private:
    template<class UnaryPredicate>
    static QQuickItem *findLingmoGraphicObjectChildIf(const Lingmo::Applet *applet, UnaryPredicate predicate);
    static QQuickItem *firstLingmoGraphicObjectChild(const Lingmo::Applet *applet);

    static Lingmo::Applet *findTaskManagerApplet(Lingmo::Containment *containment);
    static QStringList m_knownTaskManagers;
};
