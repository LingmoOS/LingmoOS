/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMOIDATTACHED_P_H
#define PLASMOIDATTACHED_P_H

#include <QAction>
#include <QObject>
#include <QQmlEngine>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public Lingmo API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace Lingmo
{
class Applet;
class Containment;
}

namespace LingmoQuick
{
// Class used exclusively to generate the Plasmoid.* attached proeprties, which are Applet instances
class PlasmoidAttached : public QObject
{
    Q_OBJECT

public:
    /** TODO: When the migration to the new action api is done, remove this enum
     */
    enum ActionPriority {
        LowPriorityAction = QAction::LowPriority,
        NormalPriorityAction = QAction::NormalPriority,
        HighPriorityAction = QAction::HighPriority,
    };
    Q_ENUM(ActionPriority)

    PlasmoidAttached(QObject *parent = nullptr);
    ~PlasmoidAttached() override;

    ////NEEDED BY QML TO CREATE ATTACHED PROPERTIES
    static Lingmo::Applet *qmlAttachedProperties(QObject *object);
};

// Class used exclusively to generate the Plasmoid.* attached proeprties, which are Applet instances
class ContainmentAttached : public QObject
{
    Q_OBJECT

public:
    ContainmentAttached(QObject *parent = nullptr);
    ~ContainmentAttached() override;

    ////NEEDED BY QML TO CREATE ATTACHED PROPERTIES
    static Lingmo::Containment *qmlAttachedProperties(QObject *object);
};

}

QML_DECLARE_TYPEINFO(LingmoQuick::PlasmoidAttached, QML_HAS_ATTACHED_PROPERTIES)
QML_DECLARE_TYPEINFO(LingmoQuick::ContainmentAttached, QML_HAS_ATTACHED_PROPERTIES)

#endif
