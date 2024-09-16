// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHUTDOWNWIDGET
#define SHUTDOWNWIDGET

#include <QFrame>

#include <functional>
#include <dtkcore_global.h>

#include "util_updateui.h"
#include "rounditembutton.h"
#include "sessionbasemodel.h"
#include "framedatabind.h"
#include "dbuslogin1manager.h"
#include "systemmonitor.h"
#include "public_func.h"

#include "huaweiswitchos_interface.h"

DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

using HuaWeiSwitchOSInterface = com::huawei::switchos;

class ShutdownWidget: public QFrame
{
    Q_OBJECT
public:
    ShutdownWidget(QWidget* parent = nullptr);
    ~ShutdownWidget() override;
    void setModel(SessionBaseModel * const model);
    void onStatusChanged(SessionBaseModel::ModeStatus status);
    bool enableState(const QString &gsettingsValue);

public slots:
    void leftKeySwitch();
    void rightKeySwitch();
    void runSystemMonitor();
    void recoveryLayout();
    void onRequirePowerAction(SessionBaseModel::PowerAction powerAction, bool needConfirm);
    void setUserSwitchEnable(bool enable);
    void onEnable(const QString &gsettingsName, bool enable);
    void updateLocale(std::shared_ptr<User> user);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    bool event(QEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnect();
    void updateTr(RoundItemButton * widget, const QString &tr);
    void onOtherPageChanged(const QVariant &value);
    void hideToplevelWindow();
    void enterKeyPushed();
    void enableHibernateBtn(bool enable);
    void enableSleepBtn(bool enable);

private:
    int m_index;
    bool m_switchUserEnable= false;
    QList<RoundItemButton *> m_btnList;
    QList<std::pair<std::function<void (QString)>, QString>> m_trList;
    SessionBaseModel* m_model;
    FrameDataBind *m_frameDataBind;
    QFrame* m_shutdownFrame = nullptr;
    SystemMonitor* m_systemMonitor = nullptr;
    QFrame* m_actionFrame = nullptr;
    QStackedLayout* m_mainLayout;
    QHBoxLayout* m_shutdownLayout;
    QVBoxLayout* m_actionLayout;
    RoundItemButton* m_currentSelectedBtn = nullptr;
    RoundItemButton* m_requireShutdownButton;
    RoundItemButton* m_requireRestartButton;
    RoundItemButton* m_requireSuspendButton;
    RoundItemButton* m_requireHibernateButton;
    RoundItemButton* m_requireLockButton;
    RoundItemButton* m_requireLogoutButton;
    RoundItemButton* m_requireSwitchUserBtn;
    RoundItemButton* m_requireSwitchSystemBtn = nullptr;
    HuaWeiSwitchOSInterface *m_switchosInterface = nullptr;
    DTK_CORE_NAMESPACE::DConfig *m_dconfig;
};

#endif // SHUTDOWNWIDGET
