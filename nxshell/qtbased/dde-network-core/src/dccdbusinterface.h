// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCCDBUSINTERFACE_H
#define DCCDBUSINTERFACE_H

#include <QDBusAbstractInterface>

namespace dde {
namespace network {

class DCCDBusInterfacePrivate;

class DCCDBusInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:
    explicit DCCDBusInterface(const QString &service, const QString &path, const QString &interface = QString(),
                              const QDBusConnection &connection = QDBusConnection::sessionBus(), QObject *parent = nullptr);
    virtual ~DCCDBusInterface() override;

    QString suffix() const;
    void setSuffix(const QString &suffix);

    QVariant property(const char *propname);
    void setProperty(const char *propname, const QVariant &value);

private:
    QScopedPointer<DCCDBusInterfacePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DCCDBusInterface)
    Q_DISABLE_COPY(DCCDBusInterface)
};

}
}

#endif // DCCDBUSINTERFACE_H
