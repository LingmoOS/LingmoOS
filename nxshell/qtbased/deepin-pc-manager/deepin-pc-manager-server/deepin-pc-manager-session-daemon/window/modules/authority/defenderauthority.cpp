// Copyright (C) 2022 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderauthority.h"

#include <QDebug>
#include <QtConcurrent>

const QString kActionName = "com.deepin.pc.sesseion.daemon.checkAuthentication";

DefenderAuthority::DefenderAuthority(QObject *parent)
    : QObject(parent)
{
}

DefenderAuthority::~DefenderAuthority()
{
    // 检查是否授权过程中，如果是，取消授权
    if (!m_mutex.tryLock()) {
        Authority::instance()->checkAuthorizationCancel();
    } else {
        // 非授权状态
        m_mutex.unlock();
    }
}

void DefenderAuthority::checkAuthorization(const QString &name, int id)
{
    // 加锁以处理意外的连接授权
    QMutexLocker locker(&m_mutex);

    preAuthorize();

    processAuthorize(name, id);

    postAuthorize();
}

// 新的授权窗口启动时，需要知道当前的授权状态
// 仅需要这一时刻的状态，之后的变化由信号通知
void DefenderAuthority::onModulesActived()
{
    if (m_mutex.tryLock()) {
        Q_EMIT notifyAuthFinished();
        m_mutex.unlock();
        return;
    }

    Q_EMIT notifyAuthStarted();
}

// 收到通知的界面需要处理授权状态
void DefenderAuthority::preAuthorize()
{
    Q_EMIT notifyAuthStarted();
}

// 收到通知的界面需要处理授权状态
void DefenderAuthority::postAuthorize()
{
    Q_EMIT notifyAuthFinished();
}

// 基于QT的信号队列与锁机制
// 对请求进行序列化处理
void DefenderAuthority::processAuthorize(const QString &moduldName, int id)
{
    // 需要关注授权错误，在本次处理前，消除旧的错误信息
    Authority::instance()->clearError();

    Authority::Result result;

    result = Authority::instance()->checkAuthorizationSync(kActionName,
                                                           UnixProcessSubject(getpid()), /// 第一个参数是需要验证的action，和规则文件写的保持一致
                                                           Authority::AllowUserInteraction);

    // 根据库建议，添加错误码处理过程
    Authority::ErrorCode err = checkAuthorizeStatus();
    if (Authority::ErrorCode::E_None != err) {
        Q_EMIT notifyAuthStatusAbnormal(err);
        Q_EMIT notifyAuthResult(moduldName, id, false);

        Authority::instance()->clearError();
        return;
    }

    Q_EMIT notifyAuthResult(moduldName, id, Authority::Result::Yes == result);
}

// 获取状态，如果有异常写入日志
Authority::ErrorCode DefenderAuthority::checkAuthorizeStatus()
{
    Authority::ErrorCode err = Authority::ErrorCode::E_None;

    if (Authority::instance()->hasError()) {
        err = Authority::instance()->lastError();
        qCritical() << Q_FUNC_INFO << "status abnormal : " << Authority::instance()->errorDetails();
    }

    return err;
}
