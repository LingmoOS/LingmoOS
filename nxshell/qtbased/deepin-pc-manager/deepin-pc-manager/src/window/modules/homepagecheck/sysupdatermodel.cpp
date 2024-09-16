// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sysupdatermodel.h"

#include "src/window/modules/common/invokers/invokerfactory.h"

#include <QDebug>
#include <QTimer>

SysUpdaterModel::SysUpdaterModel(QObject *parent)
    : QObject(parent)
    , m_managerInvokerInter(nullptr)
    , m_updaterInvokerInter(nullptr)
    , m_checkUpdateJobInvokerInter(nullptr)
    , m_checkUpdateJobTimer(nullptr)
{
    // 系统更新管理器dbus服务
    m_managerInvokerInter =
        InvokerFactory::GetInstance().CreateInvoker("org.deepin.lastore1",
                                                    "/org/deepin/lastore1",
                                                    "org.deepin.lastore1.Manager",
                                                    ConnectType::SYSTEM,
                                                    this);
    // 系统更新dbus服务
    m_updaterInvokerInter =
        InvokerFactory::GetInstance().CreateInvoker("org.deepin.lastore1",
                                                    "/org/deepin/lastore1",
                                                    "org.deepin.lastore1.Updater",
                                                    ConnectType::SYSTEM,
                                                    this);

    m_checkUpdateJobTimer = new QTimer(this);
    m_checkUpdateJobTimer->setInterval(500);
    connect(m_checkUpdateJobTimer, &QTimer::timeout, this, [this] {
        if (!m_checkUpdateJobInvokerInter) {
            return;
        }

        QString status;
        QDBusMessage msg =
            DBUS_BLOCK_INVOKE(m_checkUpdateJobInvokerInter, DBUS_PROPERTY_INVOKER_NAME, "Status");
        if (QDBusMessage::MessageType::ErrorMessage == msg.type()) {
            // 存在系统更新检测进程完成后,已经被删除的可能
            qWarning()
                << "[SysUpdaterModel] [m_checkUpdateJobInvokerInter] update checking job error";
        } else {
            QVariant ret = msg.arguments().first();
            // 将QDbusVariant转成QVariant
            ret = Utils::checkAndTransQDbusVarIntoQVar(ret);
            status = ret.toString();
            qInfo() << "[SysUpdaterModel] [m_checkUpdateJobInvokerInter] status " << status;
        }
        this->onCheckJobStatusChanged(status);
    });
}

SysUpdaterModel::~SysUpdaterModel() { }

// 检查是否有更新
void SysUpdaterModel::CheckForUpdates()
{
    // 如果任务正在进行，则不继续创建任务
    if (m_checkUpdateJobInvokerInter) {
        qInfo() << "[SysUpdaterModel] [CheckForUpdates] update task existed";
        return;
    }

    // 更新apt源
    QDBusMessage msg = DBUS_BLOCK_INVOKE(m_managerInvokerInter, "UpdateSource");
    if (QDBusMessage::MessageType::ErrorMessage == msg.type()) {
        // 发送没有系统更新信号
        Q_EMIT this->SendHaveUpdates(false);
        stopCheckingSysVer();
        qCritical() << "[SysUpdaterModel] [CheckForUpdates] UpdateSource error "
                    << msg.errorMessage();
        return;
    }

    QVariantList args = msg.arguments();
    QDBusObjectPath jobPath = args.takeFirst().value<QDBusObjectPath>();
    const QString jobPathStr = jobPath.path();
    // 设置检查系统更新任务
    SetCheckUpdatesJob(jobPathStr);
}

// 设置检查系统更新任务
void SysUpdaterModel::SetCheckUpdatesJob(const QString &jobPath)
{
    m_checkUpdateJobInvokerInter =
        InvokerFactory::GetInstance().CreateInvoker("org.deepin.lastore1",
                                                    jobPath,
                                                    "org.deepin.lastore1.Job",
                                                    ConnectType::SYSTEM,
                                                    this);
    m_checkUpdateJobTimer->start();
}

// 停止检查系统版本
void SysUpdaterModel::stopCheckingSysVer()
{
    if (m_checkUpdateJobInvokerInter) {
        // 先将QDbusVariant转成QVariant

        QVariant retArg = Utils::checkAndTransQDbusVarIntoQVar(
            DBUS_BLOCK_INVOKE(m_checkUpdateJobInvokerInter, DBUS_PROPERTY_INVOKER_NAME, "Id")
                .arguments()
                .first());
        QString jobId = retArg.toString();
        DBUS_BLOCK_INVOKE(m_managerInvokerInter, "CleanJob", jobId);
        delete m_checkUpdateJobInvokerInter;
        m_checkUpdateJobInvokerInter = nullptr;
        m_checkUpdateJobTimer->stop();
    }
}

void SysUpdaterModel::onCheckJobStatusChanged(const QString &status)
{
    qInfo() << "[SysUpdaterModel] [onCheckJobStatusChanged] status " << status;
    if (status == "failed") {
        stopCheckingSysVer();
        Q_EMIT SendHaveUpdates(false);
    } else if (status == "end" || status.isEmpty()) {
        stopCheckingSysVer();

        // apt update之后可以查看最新更新信息
        bool getFailed = false;
        int updatableAppsCount = 0;
        int updatablePackagesCount = 0;

        QDBusMessage msg =
            DBUS_BLOCK_INVOKE(m_updaterInvokerInter, DBUS_PROPERTY_INVOKER_NAME, "UpdatableApps");
        if (QDBusMessage::MessageType::ErrorMessage == msg.type()) {
            getFailed = true;
        }

        if (!getFailed) {
            // 先将QDbusVariant转成QVariant
            QVariant retArg = Utils::checkAndTransQDbusVarIntoQVar(msg.arguments().first());
            updatableAppsCount = retArg.toStringList().count();
            msg = DBUS_BLOCK_INVOKE(m_updaterInvokerInter,
                                    DBUS_PROPERTY_INVOKER_NAME,
                                    "UpdatablePackages");
            if (QDBusMessage::MessageType::ErrorMessage == msg.type()) {
                getFailed = true;
            }
        }

        if (!getFailed) {
            // 先将QDbusVariant转成QVariant
            QVariant retArg = Utils::checkAndTransQDbusVarIntoQVar(msg.arguments().first());
            updatablePackagesCount = retArg.toStringList().count();
        } else {
            // 获取失败
            // 发送没有系统更新信号
            Q_EMIT this->SendHaveUpdates(false);
            qCritical() << "[MainWindow] [onCheckJobStatusChanged] UpdateFailed, check for updates "
                           "error: = "
                        << msg;
            return;
        }

        if (0 < (updatableAppsCount + updatablePackagesCount)) {
            // 有更新
            Q_EMIT this->SendHaveUpdates(true);
        } else {
            Q_EMIT this->SendHaveUpdates(false);
        }
    }
}
