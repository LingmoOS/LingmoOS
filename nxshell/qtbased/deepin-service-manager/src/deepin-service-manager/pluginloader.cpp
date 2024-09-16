// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pluginloader.h"

#include "graph.h"
#include "policy/policy.h"
#include "service/serviceqtdbus.h"
#include "service/servicesdbus.h"
#include "utils.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QLoggingCategory>
#include <QTimer>

Q_LOGGING_CATEGORY(dsm_PluginLoader, "[PluginLoader]")

PluginLoader::PluginLoader(QObject *parent)
    : QObject(parent)
{
}

PluginLoader::~PluginLoader()
{
    foreach (auto srv, m_pluginMap.values()) {
        delete srv;
        srv = nullptr;
    }
    m_pluginMap.clear();
}

ServiceBase *PluginLoader::createService(Policy *policy)
{
    ServiceBase *srv = nullptr;
    if (policy->sdkType == SDKType::QT)
        srv = new ServiceQtDBus();
    if (policy->sdkType == SDKType::SD)
        srv = new ServiceSDBus();
    if (srv) {
        srv->init(m_type, policy);
    }

    return srv;
}

void PluginLoader::init(const QDBusConnection::BusType &type, const bool isResident)
{
    m_type = type;
    m_isResident = isResident;
}

void PluginLoader::loadByGroup(const QString &group)
{
    const QString &path = QString("%1/%2/").arg(SERVICE_CONFIG_DIR).arg(typeMap[m_type]);
    qCInfo(dsm_PluginLoader) << "init plugins in path:" << path;
    QList<Policy *> policys;
    QFileInfoList list = QDir(path).entryInfoList();
    for (auto file : list) {
        if (!file.isFile() || (file.suffix().compare("json", Qt::CaseInsensitive) != 0)) {
            continue;
        }
        Policy *policy = new Policy(this);
        policy->parseConfig(file.absoluteFilePath());
        if (policy->group != group) {
            policy->deleteLater();
            continue;
        }
        if (m_isResident != policy->isResident()) {
            continue;
        }
        policys.append(policy);
    }
    // sort policy
    const QList<Policy *> &sortedPolicys = sortPolicy(policys);
    for (auto policy : sortedPolicys) {
        // start delay
        const int delay = policy->startDelay * 1000;
        QEventLoop *loop = new QEventLoop(this);
        QTimer::singleShot(delay, this, [this, policy, loop] {
            ServiceBase *srv = createService(policy);
            if (srv == nullptr) {
                loop->quit();
                return;
            }
            if (!policy->pluginPath.isEmpty()) {
                addPlugin(srv);
            }
            loop->quit();
        });
        loop->exec();
    }
    qCInfo(dsm_PluginLoader) << "added plugins: " << m_pluginMap.keys();
}

void PluginLoader::loadByName(const QString &name)
{
    const QString &path = QString("%1/%2/").arg(SERVICE_CONFIG_DIR).arg(typeMap[m_type]);
    QFileInfoList list = QDir(path).entryInfoList();
    for (auto file : list) {
        if (!file.isFile() || (file.suffix().compare("json", Qt::CaseInsensitive) != 0)) {
            continue;
        }
        Policy *policy = new Policy(this);
        policy->parseConfig(file.absoluteFilePath());
        if (policy->name != name) {
            policy->deleteLater();
            continue;
        }

        ServiceBase *srv = createService(policy);
        if (srv == nullptr) {
            qCWarning(dsm_PluginLoader) << "create service failed!";
            return;
        }
        if (!policy->pluginPath.isEmpty()) {
            qCInfo(dsm_PluginLoader) << "init plugin:" << file;
            addPlugin(srv);
        }
        break;
    }
}

QString PluginLoader::getGroup(const QString &name)
{
    // 要在load插件之前获取group，所以不能通过map获取，只能通过Policy获取
    const QString &path = QString("%1/%2/").arg(SERVICE_CONFIG_DIR).arg(typeMap[m_type]);
    QFileInfoList list = QDir(path).entryInfoList();
    for (auto file : list) {
        if (!file.isFile() || (file.suffix().compare("json", Qt::CaseInsensitive) != 0)) {
            continue;
        }
        Policy *policy = new Policy(this);
        policy->parseConfig(file.absoluteFilePath());
        if (policy->name == name) {
            const QString &group = policy->group;
            policy->deleteLater();
            return group;
        } else {
            policy->deleteLater();
        }
    }

    return QString();
}

void PluginLoader::addPlugin(ServiceBase *obj)
{
    m_pluginMap[obj->policy->name] = obj;
    connect(obj, &ServiceBase::idleSignal, this, [this] {
        ServiceBase *srv = qobject_cast<ServiceBase *>(sender());
        Q_EMIT PluginRemoved(srv->policy->name);
        qApp->quit();
    });
    Q_EMIT PluginAdded(obj->policy->name);
}

QList<Policy *> PluginLoader::sortPolicy(QList<Policy *> policys)
{
    // 使用拓扑排序，先确定依赖关系
    auto containsDependency = [policys](const QString &name) -> Policy * {
        for (auto &&policy : policys) {
            if (policy->name == name)
                return policy;
        }
        return nullptr;
    };
    QList<QPair<Policy *, Policy *>> edges;
    for (auto &&policy : policys) {
        for (auto &&dependency : policy->dependencies) {
            if (Policy *dependencyPolicy = containsDependency(dependency)) {
                edges.append(QPair<Policy *, Policy *>{ dependencyPolicy, policy });
            } else {
                qCWarning(dsm_PluginLoader) << QString("service: %1 cannot found "
                                                       "dependency: %2!")
                                                   .arg(policy->name)
                                                   .arg(dependency);
            }
        }
    }
    // 拓扑排序
    QScopedPointer<Graph<Policy *>> graph(new Graph<Policy *>(policys, edges));
    QList<Policy *> result;
    graph->topologicalSort(result);
    qCDebug(dsm_PluginLoader) << "sort result name:";
    for (auto policy : result) {
        qDebug() << "  " << policy->name;
    }
    return result;
}

QStringList PluginLoader::plugins() const
{
    return m_pluginMap.keys();
}
