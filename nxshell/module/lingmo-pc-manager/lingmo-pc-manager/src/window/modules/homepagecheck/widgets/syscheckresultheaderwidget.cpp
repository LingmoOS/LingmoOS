// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "syscheckresultheaderwidget.h"

#include <DFontSizeManager>
#include <DPalette>
#include <DPaletteHelper>

#include <QGridLayout>

DWIDGET_USE_NAMESPACE

#define SET_ACC_NAME(control, name) SET_ACCESS_NAME_T(control, sysCheckResultHeaderWidget, name)

SysCheckResultHeaderWidget::SysCheckResultHeaderWidget(QWidget *parent)
    : DFrame(parent)
    , m_pointValue(nullptr)
    , m_pointText(nullptr)
    , m_title(nullptr)
    , m_logo(nullptr)
    , m_tips(nullptr)
    , m_fixingTitle(nullptr)
    , m_fixingTips(nullptr)
{
    this->setFixedSize(750, 146);
    this->setLineWidth(0);
    this->setBackgroundRole(DPalette::Window);
    this->setContentsMargins(0, 0, 0, 0);
    DPalette dpa = DPaletteHelper::instance()->palette(this, this->palette());
    dpa.setColor(DPalette::Window, QColor::fromRgb(163, 211, 255, int(0.13 * 255)));
    DPaletteHelper::instance()->setPalette(this, dpa);
    initUI();

    // 唯一实例
    this->setAccessibleName("sysCheckResultHeaderWidget");
}

void SysCheckResultHeaderWidget::onPointChanged(int value, int issueCount)
{
    hideAll();

    m_pointValue->show();
    m_pointText->show();
    m_title->show();
    m_tips->show();
    m_logo->show();

    m_pointValue->setText(SystemCheckHelper::getColoredText(value, QString("%1").arg(value)));

    if (CHECK_MAX_POINT == value) {
        m_title->setText(kNormalResultTitle);
        m_tips->setText(kNormalResultTip);
        m_logo->setPixmap(QIcon::fromTheme(k100PointResultLogoPath).pixmap(190, 146));
    } else {
        m_title->setText(
            QString(kAbnormalResultTitle)
                .arg(SystemCheckHelper::getColoredText(value, QString("%1").arg(issueCount))));
        m_tips->setText(kAbnormalResultTip);
        m_logo->setPixmap(QIcon::fromTheme(kAbnormalResultLogoPath).pixmap(190, 146));
    }
}

// 修复完成全部换绿色，不管是否真的修完了
void SysCheckResultHeaderWidget::onFastFixFinished()
{
    hideAll();
    m_pointValue->show();
    m_pointText->show();
    m_pointValue->setText(SystemCheckHelper::getColoredText(100, QString("%1").arg(100)));
    m_title->show();
    m_title->setText(kNormalResultTitle);
    m_tips->show();
    m_tips->setText(kNormalResultTip);

    m_logo->setPixmap(QIcon::fromTheme(k100PointResultLogoPath).pixmap(190, 146));
    m_logo->show();
}

void SysCheckResultHeaderWidget::onFastFixStarted()
{
    hideAll();
    m_fixingTips->show();
    m_fixingTitle->show();
    m_fixingTips->setText(kFixingTitle);
    m_tips->setText(kFixingTip);
    m_logo->setPixmap(QIcon::fromTheme(kRepairingResultLogoPath).pixmap(190, 146));
    m_logo->show();
}

void SysCheckResultHeaderWidget::initUI()
{
    m_pointValue = new DLabel(this);
    SET_ACC_NAME(m_pointValue, pointValue);
    m_pointValue->setFixedHeight(87);
    QFont pvf = m_pointValue->font();
    pvf.setPixelSize(60);
    m_pointValue->setFont(pvf);

    m_pointText = new DLabel(this);
    SET_ACC_NAME(m_pointText, pointText);
    m_pointText->setFixedHeight(20);
    FIXED_FONT_PIXEL_SIZE(m_pointText, 14);
    m_pointText->setText(kPointText);

    m_title = new DLabel(this);
    SET_ACC_NAME(m_title, title);
    m_title->setFixedHeight(32);
    FIXED_FONT_PIXEL_SIZE(m_title, 22);
    m_title->setAlignment(Qt::AlignBottom);

    m_tips = new DTipLabel("", this);
    SET_ACC_NAME(m_tips, tips);
    m_tips->setFixedHeight(17);
    DFontSizeManager::instance()->unbind(m_tips);
    FIXED_FONT_PIXEL_SIZE(m_tips, 12);
    m_tips->setAlignment(Qt::AlignBottom);

    m_logo = new DLabel(this);
    SET_ACC_NAME(m_logo, logo);
    m_logo->setFixedSize(190, 146);

    m_fixingTitle = new DLabel(this);
    SET_ACC_NAME(m_fixingTitle, fixingTitle);
    m_fixingTitle->setFixedHeight(32);
    FIXED_FONT_PIXEL_SIZE(m_fixingTitle, 22);

    m_fixingTitle->setAlignment(Qt::AlignBottom);

    m_fixingTips = new DTipLabel("", this);
    SET_ACC_NAME(m_fixingTips, fixingTips);
    m_fixingTips->setFixedHeight(17);
    DFontSizeManager::instance()->unbind(m_fixingTips);
    FIXED_FONT_PIXEL_SIZE(m_fixingTips, 12);

    m_fixingTips->setAlignment(Qt::AlignBottom);

    QGridLayout *mainLayout = new QGridLayout;
    // 7列 1空白*26 2分数*103 3文本*14 4空白*20 5文本*375 6图标*190 7空白*20
    // 3行 1空白*46 2文本*32 3空白*3 4文本*20 5空白*45
    FIXED_COL(0, 20);
    FIXED_COL(1, 38);
    FIXED_COL(2, 14);
    FIXED_COL(3, 20);
    FIXED_COL(4, 375);
    FIXED_COL(5, 190);
    FIXED_COL(6, 20);

    FIXED_ROW(0, 46);
    FIXED_ROW(1, 32);
    FIXED_ROW(2, 3);
    FIXED_ROW(3, 20);
    FIXED_ROW(4, 45);

    mainLayout->addWidget(m_pointValue, 0, 1, -1, 1, Qt::AlignRight | Qt::AlignHCenter);
    mainLayout->addWidget(m_pointText, 3, 2, Qt::AlignCenter);
    mainLayout->addWidget(m_title, 1, 4, Qt::AlignLeft | Qt::AlignHCenter);
    mainLayout->addWidget(m_tips, 3, 4, Qt::AlignLeft | Qt::AlignHCenter);
    mainLayout->addWidget(m_logo, 0, 5, 1, -1, Qt::AlignCenter);
    mainLayout->addWidget(m_fixingTitle, 1, 1, Qt::AlignLeft | Qt::AlignHCenter);
    mainLayout->addWidget(m_fixingTips, 3, 1, Qt::AlignLeft | Qt::AlignHCenter);
    this->setLayout(mainLayout);
}

// 隐藏UI内容，按状态显示
void SysCheckResultHeaderWidget::hideAll()
{
    m_pointValue->hide();
    m_pointText->hide();
    m_title->hide();
    m_tips->hide();
    m_logo->hide();
    m_fixingTips->hide();
    m_fixingTitle->hide();
}
