#include "systemd1managerinterface.h"

Systemd1ManagerInterface::Systemd1ManagerInterface(const QString &service,
                                                   const QString &path,
                                                   const QDBusConnection &connection,
                                                   QObject *parent)
    : QDBusAbstractInterface(service, path, SYSTEMD1_MANAGER_NAME, connection, parent)
{
    setTimeout(10000);
}

Systemd1ManagerInterface::~Systemd1ManagerInterface()
{}

// 获取服务文件列表
QPair<KError, ServiceFileInfoList> Systemd1ManagerInterface::ListServiceFiles()
{
    // 已安装的服务文件列表
    ServiceFileInfoList list;
    QList<QVariant> args;
    KError ke;

    // 调用dbus接口方法：ListUnitFiles
    QDBusMessage reply = callWithArgumentList(QDBus::Block, "ListUnitFiles", args);

    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(lastError().type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "a(ss)") {
            qvariant_cast<QDBusArgument>(reply.arguments()[0]) >> list;
        }
    }

    return {ke, list};
}

// 获取服务列表
QPair<KError, ServiceInfoList> Systemd1ManagerInterface::ListServices()
{
    ServiceInfoList list;
    QList<QVariant> args;
    KError ke;

    // 调用dbus接口方法：ListUnits
    QDBusMessage reply = callWithArgumentList(QDBus::Block, "ListUnits", args);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(lastError().type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "a(ssssssouso)") {
            qvariant_cast<QDBusArgument>(reply.arguments()[0]) >> list;
        }
    }

    return {ke, list};
}

// 获取服务dbus对象路径
QPair<KError, QDBusObjectPath> Systemd1ManagerInterface::GetService(const QString &path)
{
    KError ke {};
    QDBusObjectPath o;
    QList<QVariant> args;
    args << path;

    // 调用dbus接口方法：GetUnit
    QDBusMessage reply = callWithArgumentList(QDBus::Block, "GetUnit", args);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(lastError().type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "o") {
            o = qvariant_cast<QDBusObjectPath>(reply.arguments()[0]);
        }
    }

    return {ke, o};
}

// 启动服务
QPair<KError, QDBusObjectPath> Systemd1ManagerInterface::StartService(const QString &name, const QString &mode)
{
    KError ke;
    QDBusObjectPath o;
    QList<QVariant> args;
    args << name << mode;

    // 调用dbus接口方法: StartUnit
    QDBusMessage reply = callWithArgumentList(QDBus::Block, "StartUnit", args);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(lastError().type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "o") {
            o = qvariant_cast<QDBusObjectPath>(reply.arguments()[0]);
        }
    }

    return {ke, o};
}

// 停止服务
QPair<KError, QDBusObjectPath> Systemd1ManagerInterface::StopService(const QString &name, const QString &mode)
{
    KError ke;
    QDBusObjectPath o;
    QList<QVariant> args;
    args << name << mode;

    // 调用dbus接口方法: StopUnit
    QDBusMessage reply = callWithArgumentList(QDBus::Block, "StopUnit", args);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(lastError().type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "o") {
            o = qvariant_cast<QDBusObjectPath>(reply.arguments()[0]);
        }
    }

    return {ke, o};
}

// 停止服务
QPair<KError, QDBusObjectPath> Systemd1ManagerInterface::RestartService(const QString &name,
                                                        const QString &mode)
{
    KError ke;
    QDBusObjectPath o;
    QList<QVariant> args;
    args << name << mode;

    // 调用dbus接口方法: RestartUnit
    QDBusMessage reply = callWithArgumentList(QDBus::Block, "RestartUnit", args);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(lastError().type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "o") {
            o = qvariant_cast<QDBusObjectPath>(reply.arguments()[0]);
        }
    }

    return {ke, o};
}

// 获取服务文件状态
QPair<KError, QString> Systemd1ManagerInterface::GetServiceFileState(const QString &unit)
{
    KError ke;
    QString state;
    QList<QVariant> args;
    args.clear();

    args << unit;

    // 调用dbus接口方法: GetUnitFileState
    QDBusMessage reply = callWithArgumentList(QDBus::Block, "GetUnitFileState", args);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ke.setCode(KError::kErrorDBus);
        ke.setSubCode(lastError().type());
        ke.setErrorName(reply.errorName());
        ke.setErrorMessage(reply.errorMessage());
    } else {
        Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
        if (reply.signature() == "s") {
            state = qvariant_cast<QString>(reply.arguments()[0]);
        }
    }

    return {ke, state};
}
