// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loginsafetymodel.h"
#include "window/modules/common/invokers/invokerfactory.h"
#include "window/modules/common/invokers/invokerinterface.h"

LoginSafetyModel::LoginSafetyModel(InvokerFactoryInterface *invokeFactory, QObject *parent)
    : QObject(parent)
    , m_gSettingsInvokerInter(nullptr)
    , m_defenderDataInvokerInter(nullptr)
    , m_controlCenterInvokerInter(nullptr)
{
    if (invokeFactory) {
        m_gSettingsInvokerInter = invokeFactory->CreateSettings(DEEPIN_DEFENDER_GSETTING_PATH, "", this);
        m_defenderDataInvokerInter = invokeFactory->CreateInvoker("com.deepin.defender.datainterface",
                                                                  "/com/deepin/defender/datainterface",
                                                                  "com.deepin.defender.datainterface",
                                                                  ConnectType::SESSION, this);
        m_controlCenterInvokerInter = invokeFactory->CreateInvoker("com.deepin.dde.ControlCenter",
                                                                   "/com/deepin/dde/ControlCenter",
                                                                   "com.deepin.dde.ControlCenter",
                                                                   ConnectType::SESSION, this);
    }

    m_defenderDataInvokerInter->Connect("SetPwdLimitedLevelFinished", this, SLOT(setPwdLimitedLevelFinished(int)));
}

LoginSafetyModel::~LoginSafetyModel()
{
}

// 开启/关闭密码限制策略
void LoginSafetyModel::setPwdLimitPolicyEnable(const bool &enable)
{
    m_defenderDataInvokerInter->Invoke("SetPwdLimitPolicyEnable", QVariantList() << enable, BlockMode::NOBLOCK);
}

// 获取登录安全等级
int LoginSafetyModel::getPwdLimitLevel() const
{
    return m_gSettingsInvokerInter->GetValue(PWD_LIMIT_LEVEL).toInt();
}

// 设置登录安全等级
void LoginSafetyModel::setPwdLimitLevel(const int level)
{
    m_defenderDataInvokerInter->Invoke("SetPwdLimitedLevel", QVariantList() << level, BlockMode::NOBLOCK);
}

// 获取密码到期提醒截止天数类型
int LoginSafetyModel::getPwdChangeDeadlineType() const
{
    return m_gSettingsInvokerInter->GetValue(PWD_CHANGE_DEADLINE_TYPE).toInt();
}

// 设置密码到期提醒截止天数类型
void LoginSafetyModel::setPwdChangeDeadlineType(const int &type)
{
    return m_gSettingsInvokerInter->SetValue(PWD_CHANGE_DEADLINE_TYPE, type);
}

// 打开控制中心账号页
void LoginSafetyModel::jumpToAccountSettingPage()
{
    m_controlCenterInvokerInter->Invoke("ShowPage", {"accounts", ""}, BlockMode::NOBLOCK);
}
