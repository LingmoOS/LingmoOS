/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ABSTRACT_INHIBITION_H
#define ABSTRACT_INHIBITION_H

#include <QObject>

#include <solid/power/inhibition.h>

namespace Solid
{
/**
 * Represents an inhibition, allows to stop and start it
 *
 * When implementing this class take into account that the
 * inhibition should be stopped upon object deletion, so
 * stateChanged should be emitted on it
 */
class AbstractInhibition : public QObject
{
    Q_OBJECT
public:
    explicit AbstractInhibition(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    virtual ~AbstractInhibition()
    {
    }

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual Inhibition::State state() const = 0;

Q_SIGNALS:
    void stateChanged(Inhibition::State state);
};
} // Solid nanmespace
#endif // ABSTRACT_INHIBITION_H
