/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTSERVICEADAPTOR_H
#define BLUEZQT_GATTSERVICEADAPTOR_H

#include <QDBusAbstractAdaptor>

namespace BluezQt
{
class GattService;

class GattServiceAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.GattService1")
    Q_PROPERTY(QString UUID READ uuid)
    Q_PROPERTY(bool Primary READ primary)

public:
    explicit GattServiceAdaptor(GattService *parent);

    QString uuid() const;

    bool primary() const;

private:
    GattService *m_gattService;
};

} // namespace BluezQt

#endif
