// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dtkcore_global.h"

#include <QDBusVirtualObject>
#include <QDBusAbstractInterface>
#include <DDBusExtendedAbstractInterface>
#include <QThread>
#include <QDBusVirtualObject>

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

DCORE_USE_NAMESPACE

typedef struct DBusProxySubPathInfoDef {
    QString proxyPathPrefix;
    QString proxyInterface;
    QString interface;
}DBusProxySubPathInfo;

// TODO : DBusProxyBase越来越大了，不适合用hpp，且最好拆分成多个类解耦下
class DBusProxyBase : public QDBusVirtualObject {
public:
    DBusProxyBase(QString dbusName, QString dbusPath, QString dbusInterface,
        QString proxyDbusName, QString proxyDbusPath, QString proxyDbusInterface,
        QDBusConnection::BusType dbusType, QObject *parent = nullptr) : QDBusVirtualObject(parent)
        , m_dbusName(dbusName)
        , m_dbusPath(dbusPath)
        , m_dbusInterface(dbusInterface)
        , m_proxyDbusName(proxyDbusName)
        , m_proxyDbusPath(proxyDbusPath)
        , m_proxyDbusInterface(proxyDbusInterface)
        , m_dbusType(dbusType)
        , m_filterProperiesEnable(false)
        , m_filterMethodsEnable(false)
    {
    }
    virtual	~DBusProxyBase() {
        QDBusConnection::connectToBus(QDBusConnection::SessionBus, m_proxyDbusName).unregisterObject(m_proxyDbusPath);
        if (m_proxy) {
            delete m_proxy;
            m_proxy = nullptr;
        }
    }
    // handleMessageCustom:可选，无定义方法处理可不实现
    virtual	bool handleMessageCustom(const QDBusMessage &message, const QDBusConnection &connection) {return false;}

    // signalMonitorCustom:可选，无定义信号可不实现
    virtual void signalMonitorCustom() {};

    virtual DDBusExtendedAbstractInterface * initConnect() = 0;
    virtual QString introspectCustom(QString result) const {return result;}

    void ServiceStart()
    {
        qInfo() << "proxy:" << m_proxyDbusInterface << "to" << m_dbusInterface;
        m_proxy = initConnect();
        if (!QDBusConnection::connectToBus(m_dbusType, m_proxyDbusName).registerVirtualObject(m_proxyDbusPath, this)) {
            qWarning() << m_proxyDbusInterface << "failed to register object:" << QDBusConnection::connectToBus(m_dbusType, m_proxyDbusName).lastError().message();
            return;
        }
        if (!QDBusConnection::connectToBus(m_dbusType, m_proxyDbusName).registerService(m_proxyDbusName)) {
            qWarning() << m_proxyDbusInterface << "failed to register service:" << QDBusConnection::connectToBus(m_dbusType, m_proxyDbusName).lastError().message();
            return;
        }

        signalMonitor();
    }

    // 不设置默认不检验，全部有权限；设置了后list中指定的才有权限
    void InitFilterProperies(QStringList list) {
        m_filterProperiesEnable = true;
        m_filterProperies = list;
    }
    // 不设置默认不检验，全部有权限；设置了后list中指定的才有权限
    void InitFilterMethods(QStringList list) {
        m_filterMethodsEnable = true;
        m_filterMethods = list;
    }

    virtual bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection)
    {
        qInfo() << "[statistics]";
        qInfo() << " - proxy sender:" << getSenderProcess(message.service());
        qInfo() << " - proxy service:" << m_proxyDbusName;
        qInfo() << " - proxy path:" << m_proxyDbusPath;
        qInfo() << " - proxy interface:" << m_proxyDbusInterface;
        qInfo() << " - proxy method:" << message.member();
        qInfo() << " - proxy arguments:" << message.arguments();

        if (handleMessageCustom(message, connection)) {
            // 自定义处理成功则直接返回，否则执行通用处理
            return true;
        }
        if (message.interface() == m_proxyDbusInterface) {
            // qInfo() << "message.interface()=" << message.interface();
            if (m_filterMethodsEnable && !m_filterMethods.contains(message.member())) {
                qInfo() << m_proxyDbusInterface << "method-filter:" << message.member() << "is not allowed.";
                connection.send(message.createErrorReply("com.deepin.dde.error.NotAllowed", "is not allowed"));
                return true;
            }
            QDBusPendingCall call = m_proxy->asyncCallWithArgumentList(message.member(), message.arguments());
            call.waitForFinished();
            connection.send(message.createReply(call.reply().arguments()));
            return true;
        }
        if (message.interface() == "org.freedesktop.DBus.Properties") {
            QString calledInterface = message.arguments().at(0).toString();
            if (calledInterface != m_proxyDbusInterface) {
                qInfo() << m_proxyDbusInterface << "Properties-filter:" << calledInterface << "is not allowed.";
                connection.send(message.createErrorReply("com.deepin.dde.error.NotAllowed", "is not allowed"));
                return true;
            }

            if (message.member() == "GetAll") {
                qInfo() << m_proxyDbusInterface << "Properties-GetAll:";
                QDBusMessage msg = QDBusMessage::createMethodCall(m_proxy->service(), m_proxy->path(), "org.freedesktop.DBus.Properties", "GetAll");
                msg.setArguments({m_proxy->interface()});
                QDBusMessage reply = connection.call(msg);

                // TODO:

                connection.send(message.createReply(reply.arguments()));
                return true;
            }

            QString prop = message.arguments().at(1).toString();
            if (prop.isEmpty() || (m_filterProperiesEnable && !m_filterProperies.contains(prop))) {
                qInfo() << m_proxyDbusInterface << "Properties-filter:" << prop << "is not allowed.";
                connection.send(message.createErrorReply("com.deepin.dde.error.NotAllowed", "is not allowed"));
                return true;
            }
            if (message.member() == "Get") {
                qInfo() << m_proxyDbusInterface << "Properties-Get:" << prop;
                QVariant var = m_proxy->property(prop.toStdString().c_str());
                if (!var.isValid()) {
                    qWarning() << m_proxyDbusInterface << "Properties-Get error: Unknow";
                    connection.send(message.createErrorReply("com.deepin.dde.error.Unknow", "unknow error"));
                } else {
                    auto iterFind = m_pathInfoMap.find(prop);
                    if (iterFind != m_pathInfoMap.end()) {
                        // 如果属性是subPath的属性，则需要转换
                        const DBusProxySubPathInfo &pathInfo = iterFind.value();
                        QVariant varFix;
                        if (subPathVariantCast(var, pathInfo.proxyPathPrefix, varFix)) {
                            QVariant dbusVar;
                            dbusVar.setValue(QDBusVariant(varFix));
                            connection.send(message.createReply(dbusVar));
                        } else {
                            qWarning() << m_proxyDbusInterface << "Properties-Get error, get is not allowed." << prop;
                            connection.send(message.createErrorReply("com.deepin.dde.error.NotAllowed", "get is not allowed"));
                        }
                        return true;
                    }
                    auto iterFindProp = m_pathPropMap.find(prop);
                    if (iterFindProp != m_pathPropMap.end()) {
                        // 如果属性是subPath的属性，则需要转换
                        const QString &prefix = iterFindProp.value();
                        QVariant varFix;
                        if (subPathVariantCast(var, prefix, varFix)) {
                            QVariant dbusVar;
                            dbusVar.setValue(QDBusVariant(varFix));
                            connection.send(message.createReply(dbusVar));
                        } else {
                            qWarning() << m_proxyDbusInterface << "Properties-Get error, get is not allowed." << prop;
                            connection.send(message.createErrorReply("com.deepin.dde.error.NotAllowed", "get is not allowed"));
                        }
                        return true;
                    }
                    {
                        QVariant dbusVar;
                        dbusVar.setValue(QDBusVariant(var));
                        connection.send(message.createReply(dbusVar));
                    }
                }
                return true;
            } else if (message.member() == "Set") {
                qInfo() << m_proxyDbusInterface << "Properties-Set:" << prop;
                // 如果属性是subPath的属性，则不允许Set
                auto iterFind = m_pathInfoMap.find(prop);
                if (iterFind != m_pathInfoMap.end()) {
                    qWarning() << m_proxyDbusInterface << "Properties-Set error: not allowed to set";
                    connection.send(message.createErrorReply("com.deepin.dde.error.NotAllowed", "set is not allowed"));
                    return true;
                }
                m_proxy->setProperty(prop.toStdString().c_str(), message.arguments().at(2).value<QDBusVariant>().variant());
                connection.send(message.createReply());
                return true;
            }
        }
        if (message.interface() == "org.freedesktop.DBus.Introspectable" && message.member() == "Introspect") {
            QDBusMessage msg = QDBusMessage::createMethodCall(m_proxy->service(), m_proxy->path(), "org.freedesktop.DBus.Introspectable", QStringLiteral("Introspect"));
            QDBusMessage reply = connection.call(msg);
            QString introspectData = reply.arguments().at(0).toString();
            // qInfo() << m_proxyDbusInterface << "introspect:" << introspectData;
            QString proxyIntrospectData = introspectData.replace(m_dbusInterface, m_proxyDbusInterface);
            // qInfo() << m_proxyDbusInterface << "proxy introspect:" << proxyIntrospectData;
            connection.send(message.createReply(introspectCustom(proxyIntrospectData)));
            return true;
        }
        return false;
    }
    virtual QString	introspect(const QString &path) const
    {
        return "";
    }

    void signalMonitor()
    {
        signalAutoConnect();
        signalMonitorCustom();
        connect(m_proxy, &DDBusExtendedAbstractInterface::propertyChanged, this, [this](const QString &propName, const QVariant &value){
            // qInfo() << "propertyChanged:" << propName << value;
            if (m_filterProperiesEnable && !m_filterProperies.contains(propName)) {
                qWarning() << m_proxyDbusInterface << "propertyChanged-filter:" << propName << "is not allowed.";
                return;
            }

            QVariant varFix(value);
            auto iterFind = m_pathInfoMap.find(propName);
            if (iterFind != m_pathInfoMap.end()) {
                // 如果属性是subPath的属性，则需要转换
                const DBusProxySubPathInfo &prefix = iterFind.value();
                QVariant varTemp;
                if (subPathVariantCast(varFix, prefix.proxyPathPrefix, varTemp)) {
                    varFix = varTemp;
                }
            }
            auto iterFindProp = m_pathPropMap.find(propName);
            if (iterFindProp != m_pathPropMap.end()) {
                // 如果属性是subPath的属性，则需要转换
                const QString &prefix = iterFindProp.value();
                QVariant varTemp;
                if (subPathVariantCast(varFix, prefix, varTemp)) {
                    varFix = varTemp;
                }
            }
            QDBusMessage msg = QDBusMessage::createSignal(m_proxyDbusPath, "org.freedesktop.DBus.Properties", "PropertiesChanged");
            QList<QVariant> arguments;
            arguments.push_back(m_proxyDbusInterface);
            QVariantMap changedProps;
            changedProps.insert(propName, varFix);
            arguments.push_back(changedProps);
            arguments.push_back(QStringList());
            msg.setArguments(arguments);
            QDBusConnection::connectToBus(m_dbusType, m_proxyDbusName).send(msg);
        });
    }

    // 已知固定信号的自动连接
    void signalAutoConnect()
    {
        for(auto i = m_proxy->metaObject()->methodOffset(); i != m_proxy->metaObject()->methodCount(); ++i)
        {
            auto m = m_proxy->metaObject()->method(i);
            // 只需要处理信号
            if(m.methodType() != QMetaMethod::Signal)
                continue;
            QMetaObject::connect(m_proxy, i, this, i);
        }
    }

    // 元对象调用
    int qt_metacall(QMetaObject::Call c, int id, void **arguments)
    {
        if (id < 0)
            return id;

        if (c == QMetaObject::InvokeMetaMethod)
        {
            signalHandle(m_proxy->metaObject()->method(id), arguments);
        }

        return id;
    }

    // 信号处理
    void signalHandle(const QMetaMethod &met, void **arguments)
    {
        //创建信号
        QDBusMessage msg = QDBusMessage::createSignal(m_proxyDbusPath, m_proxyDbusInterface, met.name());

        //分析信号参数
        QList<QVariant> argumentsList;
        QVariant var;
        for (int i = 0; i != met.parameterCount(); ++i)
        {
            int type = met.parameterType(i);
            void *arg = arguments[i + 1];

            if(type == QMetaType::UnknownType)
            {
                qInfo() << "未知类型，不作处理 !";
                return;
            }

            var = QVariant(type, arg);
            argumentsList << var;
        }
        msg.setArguments(argumentsList);

        //发送信息
        QDBusConnection::connectToBus(m_dbusType, m_proxyDbusName).send(msg);
    }

    // SubPathPropInit 有一些属性是子PATH时，用SubPathPropInit注册下，用于PATH前缀自动转换，该方法不会自动动态创建子PATH
    void SubPathPropInit(QString propName, QString proxyPathPrefix) {
        if (propName.isEmpty() || proxyPathPrefix.isEmpty()) {
            return;
        }
        m_pathPropMap.insert(propName, proxyPathPrefix);
    }
    // SubPathInit 存在一种属性，这个属性是子PATH的集合时，适用SubPathInit自动动态创建子PATH
    template<typename Functor>
    void SubPathInit(QString propName, DBusProxySubPathInfo subInfo, Functor func) {
        SubPathInit(propName, subInfo, m_pathMap, func);
    }
    // 额外提供这个SubPathInit可以指定pathMap，以此处理一个proxy包含多种子PATH的情况
    template<typename Functor>
    void SubPathInit(QString propName, DBusProxySubPathInfo subInfo, QMap<QString, DBusProxyBase *> &pathMap, Functor func) {
        if (propName.isEmpty() || subInfo.proxyPathPrefix.isEmpty() || subInfo.proxyInterface.isEmpty() || subInfo.interface.isEmpty()) {
            return;
        }
        m_pathInfoMap.insert(propName, subInfo);
        QVariant var = m_proxy->property(propName.toStdString().c_str());
        if (var.isValid()) {
            subPathHandler(propName, var, pathMap, func);
        }
        connect(m_proxy, &DDBusExtendedAbstractInterface::propertyChanged, this, [=, &pathMap](const QString &name, const QVariant &value){
            if (name != propName || !value.isValid()) {
                return;
            }
            qInfo() << m_proxyDbusInterface << "propertyChanged-subPathChanged:" << name << value;
            subPathHandler(name, value, pathMap, func);
        });
    }

    // 创建和删除子PATH
    template<typename Functor>
    void subPathHandler(const QString &propName, const QVariant &pathsVar, QMap<QString, DBusProxyBase *> &pathMap, Functor func) {
        QString varType(pathsVar.typeName());
        QStringList paths;
        if (varType == "QList<QDBusObjectPath>") {
            QList<QDBusObjectPath> pathsDbus = qvariant_cast<QList<QDBusObjectPath>>(pathsVar);
            for (auto pathDbus : pathsDbus) {
                paths.append(pathDbus.path());
            }
        } else if (varType == "QStringList") {
            paths = pathsVar.value<QStringList>();
        } else {
            qWarning() << m_proxyDbusName << "sub path handler, error variant type:" << pathsVar.typeName();
            return;
        }
        QVector<QString> newPaths;
        QVector<QString> deletePaths;
        for (auto path : paths) {
            auto iter = pathMap.find(path);
            if (iter == pathMap.end()) {
                newPaths.push_back(path);
            }
        }
        for (auto existPath : pathMap.toStdMap()) {
            bool isFind = false;
            for (auto path : paths) {
                if (existPath.first == path) {
                    isFind = true;
                    break;
                }
            }
            if (!isFind) {
                deletePaths.push_back(existPath.first);
            }
        }

        qInfo() << m_proxyDbusName << "prop-paths:" << paths;
        qInfo() << m_proxyDbusName << "new-paths:" << newPaths;
        qInfo() << m_proxyDbusName << "delete-paths:" << deletePaths;

        const DBusProxySubPathInfo &pathInfo = m_pathInfoMap.find(propName).value();
        for (auto addPath : newPaths) {
            QString suffix = addPath.right(addPath.size() - (addPath.lastIndexOf("/") + 1));
            QString proxyPath = pathInfo.proxyPathPrefix + suffix;
            qInfo() << m_proxyDbusName << "create sub-path proxy:" << proxyPath << "to" << addPath;
            pathMap.insert(addPath, func(addPath, pathInfo.interface, proxyPath, pathInfo.proxyInterface));
        }
        for (auto deletePath : deletePaths) {
            auto iter = pathMap.find(deletePath);
            if (iter != pathMap.end()) {
                DBusProxyBase *proxy = iter.value();
                if (proxy) {
                    qInfo() << m_proxyDbusName << "delete sub path:" << deletePath;
                    delete proxy;
                    pathMap.erase(iter);
                }
            }
        }
    }

    // path 的转换统一处理吧
    bool subPathVariantCast(QVariant var, const QString &proxyPathPrefix, QVariant &varFix)
    {
        QString varType(var.typeName());
        if (varType == "QDBusObjectPath") {
            QDBusObjectPath pathOrg = qvariant_cast<QDBusObjectPath>(var);
            QString pathOrgStr = pathOrg.path();
            QString suffix = pathOrgStr.right(pathOrgStr.size() - (pathOrgStr.lastIndexOf("/") + 1));
            QString proxyPath = proxyPathPrefix + suffix;
            varFix.setValue(QDBusObjectPath(proxyPath));
            return true;
        } else if (varType == "QString") {
            QString pathOrg = var.value<QString>();
            QString suffix = pathOrg.right(pathOrg.size() - (pathOrg.lastIndexOf("/") + 1));
            QString proxyPath = proxyPathPrefix + suffix;
            varFix.setValue(proxyPath);
            return true;
        } else if (varType == "QList<QDBusObjectPath>") {
            QList<QDBusObjectPath> pathsFix;
            QList<QDBusObjectPath> pathsOrg = qvariant_cast<QList<QDBusObjectPath>>(var);
            for (auto pathOrg : pathsOrg) {
                QString pathOrgStr = pathOrg.path();
                QString suffix = pathOrgStr.right(pathOrgStr.size() - (pathOrgStr.lastIndexOf("/") + 1));
                QString proxyPath = proxyPathPrefix + suffix;
                pathsFix.append(QDBusObjectPath(proxyPath));
            }
            varFix.setValue(pathsFix);
            return true;
        } else if (varType == "QStringList") {
            QStringList pathsFix;
            QStringList pathsOrg = var.value<QStringList>();
            for (auto pathOrg : pathsOrg) {
                QString suffix = pathOrg.right(pathOrg.size() - (pathOrg.lastIndexOf("/") + 1));
                QString proxyPath = proxyPathPrefix + suffix;
                pathsFix.append(proxyPath);
            }
            varFix.setValue(pathsFix);
            return true;
        }
        return false;
    }

    QString getSenderProcess(QString dbusService)
    {
        const unsigned int &pid = QDBusConnection::connectToBus(m_dbusType, m_proxyDbusName).interface()->servicePid(dbusService).value();
        QFile procCmd("/proc/" + QString::number(pid) + "/cmdline");
        QString cmd;
        if(procCmd.open(QIODevice::ReadOnly))
        {
            QList<QByteArray> cmds = procCmd.readAll().split('\0');
            cmd = QString(cmds.first());
        }
        return cmd;
    }

protected:
    QString m_dbusName;
    QString m_dbusPath;
    QString m_dbusInterface;
    QString m_proxyDbusName;
    QString m_proxyDbusPath;
    QString m_proxyDbusInterface;
    QDBusConnection::BusType m_dbusType;
private:
    DDBusExtendedAbstractInterface *m_proxy;
    bool m_filterProperiesEnable;
    QStringList m_filterProperies;
    bool m_filterMethodsEnable;
    QStringList m_filterMethods;
    // subpath
    QMap<QString, DBusProxyBase *> m_pathMap;
    QMap<QString, DBusProxySubPathInfo> m_pathInfoMap;
    QMap<QString, QString> m_pathPropMap;
};


