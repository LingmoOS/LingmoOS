/*
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_INTERFACE_P_H
#define MODEMMANAGERQT_INTERFACE_P_H

#include "interface.h"

namespace ModemManager
{
class InterfacePrivate : public QObject
{
    Q_OBJECT
public:
    explicit InterfacePrivate(const QString &path, Interface *q);
    ~InterfacePrivate() override;
    QString uni;

    Q_DECLARE_PUBLIC(Interface)
    Interface *q_ptr;
public Q_SLOTS:
    virtual void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps);
};

} // namespace ModemManager

#endif
