// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "homepage.h"

#include "widgets/systemcheckdefinition.h"

#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DLabel>
#include <DSuggestButton>
#include <DSysInfo>

#include <QButtonGroup>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

HomePage::HomePage(HomePageModel *mode, QWidget *parent)
    : QWidget(parent)
    , m_mode(mode)
    , m_guiHelper(nullptr)
    , m_scorePic(nullptr)
    , m_lbScoreTip(nullptr)
    , m_greetingInfo(nullptr)
    , m_examTip(nullptr)
    , m_examBtn(nullptr)
{
    this->setFixedSize(770, 590);
    this->setAccessibleName("rightWidget_homeWidget");
    this->setContentsMargins(0, 0, 0, 0);
    // 获取应用界面工具实例
    m_guiHelper = DGuiApplicationHelper::instance();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setAlignment(Qt::AlignHCenter);

    // 首页展现的图片
    m_scorePic = new QLabel(this);
    m_scorePic->setAccessibleName("homeWidget_scorePictureLable");
    // 水平居中对齐
    m_scorePic->setAlignment(Qt::AlignCenter);
    m_scorePic->setFixedHeight(320);
    m_scorePic->setPixmap(QIcon::fromTheme(k1stPageLogoPath).pixmap(320, 320));
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_scorePic, 0, Qt::AlignHCenter);

    // 分数提示项
    m_lbScoreTip = new DLabel(this);
    m_lbScoreTip->setFixedHeight(24);
    FIXED_FONT_PIXEL_SIZE(m_lbScoreTip, 14);
    m_lbScoreTip->setAccessibleName("homeWidget_scoreTipLable");
    m_lbScoreTip->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    m_lbScoreTip->setElideMode(Qt::ElideMiddle);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_lbScoreTip, 0, Qt::AlignHCenter);

    // 欢迎语
    m_greetingInfo = new DLabel(this);
    m_greetingInfo->setFixedHeight(44);
    m_greetingInfo->setText("########");
    FIXED_FONT_PIXEL_SIZE(m_greetingInfo, 30);
    m_greetingInfo->setAccessibleName("homeWidget_greetingLable");
    m_greetingInfo->setAlignment(Qt::AlignHCenter);
    m_greetingInfo->setElideMode(Qt::ElideMiddle);
    // 设置分数提示字体样式
    QFont ft = m_greetingInfo->font();
    ft.setBold(true);
    m_greetingInfo->setFont(ft);
    mainLayout->addWidget(m_greetingInfo, 0, Qt::AlignHCenter);

    // 体检提示标签
    m_examTip = new DTipLabel("", this);
    m_examTip->setFixedHeight(20);
    m_examTip->setAccessibleName("homeWidget_examTipLable");
    m_examTip->setAlignment(Qt::AlignBottom);
    m_examTip->setElideMode(Qt::ElideMiddle);
    DFontSizeManager::instance()->unbind(m_examTip);
    FIXED_FONT_PIXEL_SIZE(m_examTip, 14);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_examTip, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(25);

    // 体检按钮
    m_examBtn = new DSuggestButton(this);
    m_examBtn->setAccessibleName("homeWidget_examButton");
    m_examBtn->setFixedSize(170, 36);
    FIXED_FONT_PIXEL_SIZE(m_examBtn, 14);
    mainLayout->addWidget(m_examBtn, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(60);

    setLayout(mainLayout);

    // 更新界面
    updateUI();
    // 连接信号
    connect(m_examBtn, &DSuggestButton::clicked, this, &HomePage::notifyShowCheckPage);
    // 主题变换
    connect(m_guiHelper,
            &DGuiApplicationHelper::themeTypeChanged,
            this,
            &HomePage::changePicWithTheme);
}

HomePage::~HomePage()
{
}

// 设置数据处理对象
void HomePage::setModel(HomePageModel *mode)
{
    m_mode = mode;
}

// 更新界面
void HomePage::updateUI()
{
    // 根据体检分数，改变显示
    int score = m_mode->GetSafetyScore();
    // 若分数为“-1”，则表示没有体检过
    if (-1 == score) {
        // 分数提示项
        QString coloredTip = kOutOfDateGreetingStatus;
        coloredTip = SystemCheckHelper::getColoredText(-1, coloredTip);
        m_lbScoreTip->setText(coloredTip);
        m_examTip->setText(kOutOfDateGreetingInfo);
        m_examBtn->setText(tr("Check Now"));
        m_greetingInfo->setText(kOutOfDateGreetingStatus);
        return;
    }
    // 非常安全分数阶段;
    else if (PERFECTLY_SAFE_SCORE_ABOVE <= score) {
        // 分数提示项
        m_lbScoreTip->setText(
            tr("Last Result: <font color=%1>%2</font> points").arg(k90_100Color).arg(score));
        m_examBtn->setText(tr("Check Now"));
        m_greetingInfo->setText(k90_100GreetingStatus);
        m_examTip->setText(k90_100GreetingInfo);
    }
    // 安全分数阶段
    else if (SAFE_SCORE_ABOVE <= score) {
        // 分数提示项
        m_lbScoreTip->setText(
            tr("Last Result: <font color=%1>%2</font> points").arg(k75_90Color).arg(score));
        m_examTip->setText(k75_90GreetingInfo);
        m_examBtn->setText(tr("Check Now"));
        m_greetingInfo->setText(k75_90GreetingStatus);
    }
    // 危险分数阶段
    else if (DANGEROUS_SCORE_ABOVE <= score) {
        // 分数提示项
        m_lbScoreTip->setText(
            tr("Last Result: <font color=%1>%2</font> points").arg(k60_75Color).arg(score));
        m_examTip->setText(k60_75GreetingInfo);
        m_examBtn->setText(tr("Check Now"));

        QString coloredGreeting = kyouComputer + SystemCheckHelper::getColoredText(score, katRist);
        m_greetingInfo->setText(coloredGreeting);
    }
    // 非常危险分数阶段
    else {
        // 分数提示项
        m_lbScoreTip->setText(
            tr("Last Result: <font color=%1>%2</font> points").arg(k0_60Color).arg(score));
        m_examTip->setText(k0_60GreetingInfo);
        m_examBtn->setText(tr("Check Now"));

        QString coloredGreeting =
            kyouComputer + SystemCheckHelper::getColoredText(score, katHighRist);
        m_greetingInfo->setText(coloredGreeting);
    }

    // 根据上次体检时间，调整体检提示标签文字
    QDateTime lastCheckTime = m_mode->GetLastCheckTime();
    qint64 days = lastCheckTime.daysTo(QDateTime::currentDateTime());
    if (days > 5) {
        QString coloredTip = tr("Checked %1 days ago").arg(int(days));
        coloredTip = SystemCheckHelper::getColoredText(-1, coloredTip);
        m_lbScoreTip->setText(coloredTip);

        m_examTip->setText(k5DaysGreetingInfo);

        m_greetingInfo->setText(k5DaysGreetingStatus);
    }

    // 随主题类型改变首页图
    changePicWithTheme(m_guiHelper->themeType());
}

void HomePage::changePicWithTheme(DGuiApplicationHelper::ColorType themeType)
{
    Q_UNUSED(themeType);

    //    QIcon icon;
    //    // 只有是社区版才是社区  其他的都是专业
    //    if (DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosCommunity) {
    //        // 社区版本
    //        if (themeType == DGuiApplicationHelper::ColorType::LightType) {
    //            icon = QIcon::fromTheme(HOMEPAGE_DEEPIN_LIGHT);
    //        } else {
    //            icon = QIcon::fromTheme(HOMEPAGE_DEEPIN_DARK);
    //        }
    //    } else {
    //        // 专业版本
    //        if (themeType == DGuiApplicationHelper::ColorType::LightType) {
    //            icon = QIcon::fromTheme(HOMEPAGE_UOS_LIGHT);
    //        } else {
    //            icon = QIcon::fromTheme(HOMEPAGE_UOS_DARK);
    //        }
    //    }
    //    m_scorePic->setPixmap(icon.pixmap(300, 300));
}
