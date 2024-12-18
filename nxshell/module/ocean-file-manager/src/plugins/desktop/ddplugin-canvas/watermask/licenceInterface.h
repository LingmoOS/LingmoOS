// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LICENCEINTERFACE_H
#define LICENCEINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.lingmo.license
 */
class ComLingmoLicenseInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.lingmo.license.Info"; }

public:
    ComLingmoLicenseInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~ComLingmoLicenseInterface();

    Q_PROPERTY(int AuthorizationState READ authorizationState)
    inline int authorizationState() const
    { return qvariant_cast< int >(property("AuthorizationState")); }

    Q_PROPERTY(uint AuthorizationProperty READ authorizationProperty)
    inline uint authorizationProperty() const
    { return qvariant_cast< uint >(property("AuthorizationProperty")); }

    Q_PROPERTY(uint ServiceProperty READ serviceProperty)
    inline uint serviceProperty() const
    { return qvariant_cast< uint >(property("ServiceProperty")); }

Q_SIGNALS: // SIGNALS
    void LicenseStateChange();

};

namespace com {
  namespace lingmo {
    namespace license {
      typedef ::ComLingmoLicenseInterface Info;
    }
  }
}

#endif // LICENCEINTERFACE_H
