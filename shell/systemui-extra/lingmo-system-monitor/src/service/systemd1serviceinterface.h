#ifndef SYSTEMD1SERVICEINTERFACE_H
#define SYSTEMD1SERVICEINTERFACE_H

#include "dbuspropertiesinterface.h"

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

class KError;

#define SYSTEMD1_UNIT_PATH "/org/freedesktop/systemd1/unit/"
#define SYSTEMD1_UNIT_NAME "org.freedesktop.systemd1.Unit"
#define SYSTEMD1_SERVICE_NAME "org.freedesktop.systemd1.Service"

class Systemd1ServiceInterface : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Systemd1ServiceInterface 构造
     * @param service 服务名称
     * @param path  服务路径
     * @param connection dbus连接对象
     * @param parent 父对象指针
     */
    Systemd1ServiceInterface(const QString &service,
                             const QString &path,
                             const QDBusConnection &connection,
                             QObject *parent = nullptr);

    virtual ~Systemd1ServiceInterface();

    /**
     * @brief normalizeUnitPath 规范化Service路径, 用转义格式替换特殊字符
     * @param id Service id
     * @return 规范化的Service对象路径
     */
    static inline QDBusObjectPath normalizeUnitPath(const QString &id)
    {
        QString buf {};
        for (auto u : id) {
            if (u.isLetterOrNumber()) {
                buf.append(u);
            } else {
                // special character, needs encode
                buf.append("_").append(QString("%1").arg(uint(u.toLatin1()), 0, 16));
            }
        }
        return QDBusObjectPath {QString("%1%2").arg(SYSTEMD1_UNIT_PATH).arg(buf)};
    }

    /**
     * @brief getMainPID 获取服务主进程id
     * @return 错误信息，主进程id
     */
    QPair<KError, quint32> getMainPID() const;

    /**
     * @brief getId 获取服务的id
     * @return 服务的id
     */
    QPair<KError, QString> getId() const;

    /**
     * @brief getLoadState 获取服务的加载状态
     * @return 服务的加载状态
     */
    QPair<KError, QString> getLoadState() const;

    /**
     * @brief getActiveState 获取服务的活动状态
     * @return 服务的活动状态
     */
    QPair<KError, QString> getActiveState() const;

    /**
     * @brief getSubState 获取服务的子状态
     * @return 服务的子状态
     */
    QPair<KError, QString> getSubState() const;

    /**
     * @brief getDescription 获取服务的描述
     * @return 服务的描述
     */
    QPair<KError, QString> getDescription() const;

    /**
     * @brief canReload 获取服务是否可重加载属性
     * @return 服务是否可重加载属性
     */
    QPair<KError, bool> canReload() const;

    /**
     * @brief canStart 获取服务是否可启动属性
     * @return 服务是否可启动属性
     */
    QPair<KError, bool> canStart() const;

    /**
     * @brief canStop 获取服务是否可停止属性
     * @return 服务是否可停止属性
     */
    QPair<KError, bool> canStop() const;

private:
    // 服务属性接口对象
    DBusPropertiesInterface *m_dbusPropIFService;
};

#endif // SYSTEMD1SERVICEINTERFACE_H
