#ifndef DBUSPROPERTIESINTERFACE_H
#define DBUSPROPERTIESINTERFACE_H

#include "kerror.h"

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QtDBus>

class DBusPropertiesInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:
    static inline const char *staticInterfaceName() {
        return "org.freedesktop.DBus.Properties";
    }

public:
    /**
     * @brief DBusPropertiesInterface 构造
     * @param service   服务名称
     * @param path  服务路径
     * @param connection    dbus连接
     * @param parent    父对象指针
     */
    DBusPropertiesInterface(const QString &service, const QString &path,
                                const QDBusConnection &connection, QObject *parent = nullptr);
    ~DBusPropertiesInterface();

public Q_SLOTS:
    /**
     * @brief Get 获取指定dbus的指定属性值
     * @param in0 dbus名称
     * @param in1 属性名称
     * @return 属性值
     */
    inline QPair<KError, QDBusMessage> Get(const QString &in0, const QString &in1)
    {
        QList<QVariant> args;
        KError ec;

        args << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        // make dbus call
        QDBusMessage msg = callWithArgumentList(QDBus::Block, QStringLiteral("Get"), args);
        if (lastError().isValid()) {
            ec.setCode(KError::kErrorDBus);
            ec.setSubCode(lastError().type());
            ec.setErrorName(lastError().name());
            ec.setErrorMessage(lastError().message());
        }

        return {ec, msg};
    }
};

#endif // DBUSPROPERTIESINTERFACE_H
