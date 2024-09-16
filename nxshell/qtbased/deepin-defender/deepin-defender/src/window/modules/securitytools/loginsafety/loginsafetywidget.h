// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "loginsafetymodel.h"
#include "../src/window/namespace.h"
#include "../src/window/modules/common/common.h"
#include "../src/window/modules/common/compixmap.h"

#include <dtkwidget_global.h>

#include <QWidget>

namespace def {
namespace widgets {
class TitleButtonItem;
class TitledSliderItem;
class DCCSlider;
} // namespace widgets
} // namespace def

DEF_NAMESPACE_BEGIN
class PwdLimitLevelItem;
class PwdLevelChangedDlg;
DEF_NAMESPACE_END

class QVBoxLayout;

// 使用dtk命名空间
DWIDGET_USE_NAMESPACE
// 使用安全中心命名空间
DEF_USING_NAMESPACE
// 使用安全中心控件命名空间
using namespace def::widgets;
class LoginSafetyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoginSafetyWidget(LoginSafetyModel *model, QWidget *parent = nullptr);
    ~LoginSafetyWidget();
    void initUI();

private:
    // 密码限制等级设置（显示）
    void setPwdLimitLevelDisplay(PwdLimitLevel level);

private:
    LoginSafetyModel *m_model; // 登陆安全数据对象
    QVBoxLayout *m_layout; // 主布局器
    PwdLimitLevelItem *m_highLevelItem; // 高密码安全等级项
    PwdLimitLevelItem *m_mediumLevelItem; // 中密码安全等级项
    PwdLimitLevelItem *m_lowLevelItem; // 低密码安全等级项
    PwdLimitLevel m_lastPwdLimitLevel; // 上次设置的密码安全等级
    TitleButtonItem *m_goToSettingsItem; // 跳转到设置界面按钮项
    TitledSliderItem *m_pwdChangeDeadlineItem; // 密码修改到期提醒设置条
};
