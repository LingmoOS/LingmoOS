// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loginsafetywidget.h"
#include "../src/widgets/titlebuttonitem.h"
#include "../src/widgets/titledslideritem.h"
#include "../src/widgets/dccslider.h"
#include "pwdlimitlevelitem.h"
#include "pwdlevelchangeddlg.h"

#include <DTipLabel>
#include <DFrame>
#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>

LoginSafetyWidget::LoginSafetyWidget(LoginSafetyModel *model, QWidget *parent)
    : QWidget(parent)
    , m_model(model)
    , m_layout(new QVBoxLayout)
    , m_highLevelItem(nullptr)
    , m_mediumLevelItem(nullptr)
    , m_lowLevelItem(nullptr)
    , m_lastPwdLimitLevel(PwdLimitLevel::Low)
    , m_goToSettingsItem(nullptr)
    , m_pwdChangeDeadlineItem(nullptr)
{
    this->setAccessibleName("rightWidget_loginSafetyWidget");

    initUI();
}

LoginSafetyWidget::~LoginSafetyWidget()
{
}

// 初始化界面
void LoginSafetyWidget::initUI()
{
    setLayout(m_layout);

    // 添加滑动条区域
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->viewport()->setAccessibleName("scrollArea_viewWidget");
    scrollArea->setAccessibleName("loginSafetyWidget_scrollArea");
    scrollArea->setFrameShape(QFrame::Shape::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    m_layout->addWidget(scrollArea);

    // 将mainWidget设置为滑动区域的主窗口
    QWidget *scrollAreaWidget = new QWidget(scrollArea);
    scrollAreaWidget->setAccessibleName("viewWidget_contentWidget");
    scrollArea->setWidget(scrollAreaWidget);

    // 设置滑动条区域布局器
    QVBoxLayout *scrollAreaLayout = new QVBoxLayout;
    scrollAreaLayout->setContentsMargins(0, 0, 0, 0);
    scrollAreaWidget->setLayout(scrollAreaLayout);

    QLabel *title = new QLabel(tr("Login Safety"), this);
    title->setAccessibleName("contentWidget_titleLable");
    QFont ft = title->font();
    ft.setBold(true);
    title->setFont(ft);
    DFontSizeManager::instance()->bind(title, DFontSizeManager::T5);
    scrollAreaLayout->setSpacing(0);
    scrollAreaLayout->addWidget(title, 0, Qt::AlignLeft | Qt::AlignTop);
    scrollAreaLayout->setAlignment(Qt::AlignTop);
    scrollAreaLayout->addSpacing(15);

    // 密码限制等级
    scrollAreaLayout->addSpacing(10);
    m_highLevelItem = new PwdLimitLevelItem(this);
    m_highLevelItem->setAccessibleName("contentWidget_highLevelFrame");
    m_highLevelItem->setAccessibleParentText("highLevelFrame");
    m_highLevelItem->setIcon(QIcon::fromTheme("dcc_security_level_high"));
    m_highLevelItem->setSelectedIcon(QIcon::fromTheme(LOGIN_SAFETY_LEVEL_SELECTED));
    m_highLevelItem->setLevelText(tr("High"));
    m_highLevelItem->setTipText(tr("The password must have at least 8 characters, and contain at least 3 of the four available character types: lowercase letters, uppercase letters, numbers, and symbols"));
    scrollAreaLayout->addWidget(m_highLevelItem);

    scrollAreaLayout->addSpacing(15);
    m_mediumLevelItem = new PwdLimitLevelItem(this);
    m_mediumLevelItem->setAccessibleName("contentWidget_mediumLevelFrame");
    m_mediumLevelItem->setAccessibleParentText("mediumLevelFrame");
    m_mediumLevelItem->setIcon(QIcon::fromTheme("dcc_security_level_medium"));
    m_mediumLevelItem->setSelectedIcon(QIcon::fromTheme(LOGIN_SAFETY_LEVEL_SELECTED));
    m_mediumLevelItem->setLevelText(tr("Medium"));
    m_mediumLevelItem->setTipText(tr("The password must have at least 6 characters, and contain at least 2 of the four available character types: lowercase letters, uppercase letters, numbers, and symbols"));
    scrollAreaLayout->addWidget(m_mediumLevelItem);

    scrollAreaLayout->addSpacing(15);
    m_lowLevelItem = new PwdLimitLevelItem(this);
    m_lowLevelItem->setAccessibleName("contentWidget_lowLevelFrame");
    m_lowLevelItem->setAccessibleParentText("lowLevelFrame");
    m_lowLevelItem->setIcon(QIcon::fromTheme("dcc_security_level_low"));
    m_lowLevelItem->setSelectedIcon(QIcon::fromTheme(LOGIN_SAFETY_LEVEL_SELECTED));
    m_lowLevelItem->setLevelText(tr("Low"));
    m_lowLevelItem->setTipText(tr("No restrictions"));
    scrollAreaLayout->addWidget(m_lowLevelItem);

    // 跳转到控制中心-密码修改页按钮
    scrollAreaLayout->addSpacing(30);
    m_goToSettingsItem = new TitleButtonItem(this);
    m_goToSettingsItem->setAccessibleName("contentWidget_settingFrame");
    m_goToSettingsItem->setAccessibleParentText("contentWidget_settingFrame");
    m_goToSettingsItem->setTitle(tr("Change password"));
    m_goToSettingsItem->setButtonText(tr("Go to Settings"));
    scrollAreaLayout->addWidget(m_goToSettingsItem, 0, Qt::AlignTop);

    // 密码有效期设置滚动条
    scrollAreaLayout->addSpacing(15);
    QStringList annos;
    annos << tr("%1 days").arg(30) << tr("%1 days").arg(60) << tr("%1 days").arg(90) << tr("%1 days").arg(120) << tr("%1 days").arg(150) << tr("%1 days").arg(180) << tr("Never");

    m_pwdChangeDeadlineItem = new TitledSliderItem(tr("Notify me before password expired"), this);
    m_pwdChangeDeadlineItem->setAccessibleName("contentWidget_pwdChangeLimitFrame");
    m_pwdChangeDeadlineItem->slider()->setType(DCCSlider::Vernier);
    m_pwdChangeDeadlineItem->slider()->setRange(0, 6);
    m_pwdChangeDeadlineItem->slider()->setTickPosition(QSlider::TicksBelow);
    m_pwdChangeDeadlineItem->slider()->setTickInterval(0);
    m_pwdChangeDeadlineItem->slider()->setPageStep(0);
    m_pwdChangeDeadlineItem->setAnnotations(annos);
    scrollAreaLayout->addWidget(m_pwdChangeDeadlineItem);
    scrollAreaLayout->addSpacing(15);
    //设置初始值
    m_pwdChangeDeadlineItem->slider()->setValue(m_model->getPwdChangeDeadlineType());

    DTipLabel *tiplable = new DTipLabel(tr("The notification will pop up, asking you to change the password before expiration"), this);
    tiplable->setWordWrap(true);
    tiplable->setAlignment(Qt::AlignLeft);
    scrollAreaLayout->addWidget(tiplable);

    //// 连接信号槽
    // 密码限制等级设置
    connect(m_lowLevelItem, &PwdLimitLevelItem::clicked, this, [this] {
        this->m_model->setPwdLimitLevel(PwdLimitLevel::Low);
    });
    connect(m_mediumLevelItem, &PwdLimitLevelItem::clicked, this, [this] {
        this->m_model->setPwdLimitLevel(PwdLimitLevel::Medium);
    });
    connect(m_highLevelItem, &PwdLimitLevelItem::clicked, this, [this] {
        this->m_model->setPwdLimitLevel(PwdLimitLevel::High);
    });
    // 密码限制等级设置完成
    connect(m_model, &LoginSafetyModel::setPwdLimitedLevelFinished, this, [this](int level) {
        PwdLimitLevel limitLevel = PwdLimitLevel(level);
        setPwdLimitLevelDisplay(limitLevel);
        if (m_lastPwdLimitLevel < level) {
            PwdLevelChangedDlg *dlg = new PwdLevelChangedDlg(this);
            dlg->show();
            // 更新上次密码限制等级
            m_lastPwdLimitLevel = limitLevel;
            dlg->exec();
        }
        m_lastPwdLimitLevel = limitLevel;
    });

    //跳转到控制中心修改密码界面
    connect(m_goToSettingsItem->button(), &QPushButton::clicked, m_model, &LoginSafetyModel::jumpToAccountSettingPage);

    //链接，密码有效期设置滚动条 数值改变时
    connect(m_pwdChangeDeadlineItem->slider(), &DCCSlider::valueChanged, m_model, &LoginSafetyModel::setPwdChangeDeadlineType);

    // 设置密码限制等级（显示）
    PwdLimitLevel limitLevel = PwdLimitLevel(m_model->getPwdLimitLevel());
    setPwdLimitLevelDisplay(limitLevel);
    m_lastPwdLimitLevel = limitLevel;
}

// 密码限制等级设置
void LoginSafetyWidget::setPwdLimitLevelDisplay(PwdLimitLevel level)
{
    m_highLevelItem->setSelected(false);
    m_mediumLevelItem->setSelected(false);
    m_lowLevelItem->setSelected(false);
    switch (level) {
    case PwdLimitLevel::High:
        m_highLevelItem->setSelected(true);
        break;
    case PwdLimitLevel::Medium:
        m_mediumLevelItem->setSelected(true);
        break;
    case PwdLimitLevel::Low:
        m_lowLevelItem->setSelected(true);
        break;
    }
}
