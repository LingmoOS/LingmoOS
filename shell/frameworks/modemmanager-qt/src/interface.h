/*
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGERQT_INTERFACE_H
#define MODEMMANAGERQT_INTERFACE_H

#include <modemmanagerqt_export.h>

#include <QObject>
#include <QSharedPointer>

#include "generictypes.h"

namespace ModemManager
{
class InterfacePrivate;

class MODEMMANAGERQT_EXPORT Interface : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Interface)

    Q_PROPERTY(QString uni READ uni)
public:
    typedef QSharedPointer<Interface> Ptr;
    typedef QList<Ptr> List;

    explicit Interface(const QString &path, QObject *parent = nullptr);
    ~Interface() override;

    QString uni() const;

protected:
    MODEMMANAGERQT_NO_EXPORT explicit Interface(InterfacePrivate &dd, QObject *parent = nullptr);

protected:
    InterfacePrivate *const d_ptr;
};
} // namespace ModemManager

#endif
