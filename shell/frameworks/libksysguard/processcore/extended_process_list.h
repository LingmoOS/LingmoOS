/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <processcore/processes.h>

#include <QObject>
#include <QSharedPointer>

#include "processcore_export.h"

namespace KSysGuard
{
class ProcessAttribute;

class PROCESSCORE_EXPORT ExtendedProcesses : public KSysGuard::Processes
{
    Q_OBJECT
public:
    QList<ProcessAttribute *> attributes() const;
    QList<ProcessAttribute *> extendedAttributes() const;

    /**
     * Returns a single shared instance of the process list for when used in multiple views
     */
    static QSharedPointer<ExtendedProcesses> instance();

private:
    ExtendedProcesses(QObject *parent = nullptr);
    ~ExtendedProcesses() override;
    class Private;
    QScopedPointer<Private> d;
};
}
