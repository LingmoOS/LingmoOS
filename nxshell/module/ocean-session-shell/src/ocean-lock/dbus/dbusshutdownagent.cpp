// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusshutdownagent.h"
#include "sessionbasemodel.h"

DBusShutdownAgent::DBusShutdownAgent(QObject *parent)
    : QObject(parent)
    , m_model(nullptr)
{

}

void DBusShutdownAgent::setModel(SessionBaseModel * const model)
{
    m_model = model;
}

void DBusShutdownAgent::show()
{
    if (!canShowShutDown()) {
        return;
    }

    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    m_model->setVisible(true);
}

void DBusShutdownAgent::Shutdown()
{
    qInfo() << "Shutdown";
    if (!canShowShutDown()) {
         //锁屏时允许关机
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireShutdown, false);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
        m_model->setVisible(true);
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireShutdown, true);
    }
}

void DBusShutdownAgent::Restart()
{
    qInfo() << "Restart";
    if (!canShowShutDown()) {
        //锁屏时允许重启
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireRestart, false);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
        m_model->setVisible(true);
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireRestart, true);
    }
}

void DBusShutdownAgent::Logout()
{
    qInfo() << "Logout";
    if (!canShowShutDown()) {
        return;
    }

    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    m_model->setVisible(true);
    emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireLogout, true);
}

void DBusShutdownAgent::Suspend()
{
    qInfo() << "Suspend";
    if (!canShowShutDown()) {
        //锁屏时允许待机
        m_model->setPowerAction(SessionBaseModel::RequireSuspend);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
        m_model->setVisible(true);
        emit m_model->onRequirePowerAction(SessionBaseModel::RequireSuspend, true);
    }
}

void DBusShutdownAgent::Hibernate()
{
    qInfo() << "Hibernate";
    if (!canShowShutDown()) {
        //锁屏时允许休眠
        m_model->setPowerAction(SessionBaseModel::RequireHibernate);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
        m_model->setVisible(true);
        emit m_model->onRequirePowerAction(SessionBaseModel::RequireHibernate, true);
    }
}

void DBusShutdownAgent::SwitchUser()
{
    qInfo() << "SwitchUser";
    if (!canShowShutDown()) {
        return;
    }

    emit m_model->showUserList();
}

void DBusShutdownAgent::Lock()
{
    qInfo() << "Lock";
    if (!canShowShutDown()) {
        return;
    }
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    m_model->setVisible(true);
    m_model->setPowerAction(SessionBaseModel::RequireLock);
}

bool DBusShutdownAgent::canShowShutDown()
{
    //如果当前界面已显示，而且不是关机模式，则当前已锁屏，因此不允许调用,以免在锁屏时被远程调用而进入桌面
    if (m_model->visible() && m_model->currentModeState() != SessionBaseModel::ModeStatus::ShutDownMode) {
        return false;
    }

    return true;
}
