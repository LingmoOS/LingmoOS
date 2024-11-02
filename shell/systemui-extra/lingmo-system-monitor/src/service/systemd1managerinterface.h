#ifndef SYSTEMD1MANAGERINTERFACE_H
#define SYSTEMD1MANAGERINTERFACE_H

#include "kerror.h"
#include "servicefileinfo.h"
#include "serviceinfo.h"

#include <QtDBus>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#define SYSTEMD1_MANAGER_NAME "org.freedesktop.systemd1.Manager"

class Systemd1ManagerInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    Systemd1ManagerInterface(const QString &service, const QString &path,
                             const QDBusConnection &connection, QObject *parent = nullptr);
    virtual ~Systemd1ManagerInterface();

public slots:
    /**
     * @brief ListServiceFiles 获取服务文件列表
     * @return 错误信息，服务文件列表
     */
    QPair<KError, ServiceFileInfoList> ListServiceFiles();

    /**
     * @brief ListServices 获取服务列表
     * @return 错误信息，服务列表
     */
    QPair<KError, ServiceInfoList> ListServices();

    /**
     * @brief GetService 获取服务dbus对象路径
     * @param path 服务路径
     * @return 错误信息，服务dbus对象路径
     */
    QPair<KError, QDBusObjectPath> GetService(const QString &path);

    /**
     * @brief StartService 启动服务
     * @param name  服务名称
     * @param mode 模式
     * @return 错误信息，服务dbus对象路径
     */
    QPair<KError, QDBusObjectPath> StartService(const QString &name, const QString &mode);

    /**
     * @brief StopService 停止服务
     * @param name 服务名称
     * @param mode 模式
     * @return 错误信息，服务dbus对象路径
     */
    QPair<KError, QDBusObjectPath> StopService(const QString &name, const QString &mode);

    /**
     * @brief RestartService 停止服务
     * @param name 服务名称
     * @param mode 模式
     * @return 错误信息，服务dbus对象路径
     */
    QPair<KError, QDBusObjectPath> RestartService(const QString &name, const QString &mode);

    /**
     * @brief GetServiceFileState 获取服务文件状态
     * @param service 服务文件名称
     * @return 错误信息，服务文件名称
     */
    QPair<KError, QString> GetServiceFileState(const QString &service);

};

#endif // SYSTEMD1MANAGERINTERFACE_H
