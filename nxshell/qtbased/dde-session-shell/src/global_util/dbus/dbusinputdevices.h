// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSINPUTDEVICES_H_1439802129
#define DBUSINPUTDEVICES_H_1439802129

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

struct InputDevice {
    QString interface;
    QString deviceType;
};

typedef QList<InputDevice> InputDeviceList;

Q_DECLARE_METATYPE(InputDevice)
Q_DECLARE_METATYPE(InputDeviceList)

QDBusArgument &operator<<(QDBusArgument &argument, const InputDevice &device);
const QDBusArgument &operator>>(const QDBusArgument &argument, InputDevice &device);


/*
 * Proxy class for interface com.deepin.dde.InputDevices1
 */
class DBusInputDevices: public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage& msg)
        {
            QList<QVariant> arguments = msg.arguments();
            if (3 != arguments.count())
                return;
            QString interfaceName = msg.arguments().at(0).toString();
            if (interfaceName !="org.deepin.dde.InputDevices1")
                return;
            QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
            QStringList keys = changedProps.keys();
            foreach(const QString &prop, keys) {
            const QMetaObject* self = metaObject();
                for (int i=self->propertyOffset(); i < self->propertyCount(); ++i) {
                    QMetaProperty p = self->property(i);
                    if (p.name() == prop) {
                    Q_EMIT p.notifySignal().invoke(this);
                    }
                }
            }
       }
public:
    static inline const char *staticInterfaceName()
    { return "org.deepin.dde.InputDevices1"; }
    static inline const char *staticServiceName()
    { return "org.deepin.dde.InputDevices1"; }
    static inline const char *staticObjectPath()
    { return "/org/deepin/dde/InputDevices1"; }

public:
    DBusInputDevices(QObject *parent = 0);

    ~DBusInputDevices();

    Q_PROPERTY(InputDeviceList Infos READ infos NOTIFY InfosChanged)
    inline InputDeviceList infos() const
    { return qvariant_cast< InputDeviceList >(property("Infos")); }

public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
    // begin property changed signals
    void InfosChanged();
};

namespace org {
  namespace deepin {
    namespace dde {
      typedef ::DBusInputDevices InputDevices1;
    }
  }
}
#endif
