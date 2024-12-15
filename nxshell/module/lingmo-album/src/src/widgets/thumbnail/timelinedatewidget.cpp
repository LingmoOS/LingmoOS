// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timelinedatewidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <DFontSizeManager>
#include <dpalettehelper.h>

TimeLineDateWidget::TimeLineDateWidget(QStandardItem *item, const QString &time, const QString &num)
    :  m_chooseBtn(nullptr), m_pDate(nullptr), m_pNumCheckBox(nullptr), m_currentItem(item)
{
    this->setContentsMargins(0, 10, 0, 0);
    this->setFixedHeight(90);
    //时间线日期
    m_pDate = new DLabel(this);
    DFontSizeManager::instance()->bind(m_pDate, DFontSizeManager::T3, QFont::DemiBold);
    QFont ft1 = DFontSizeManager::instance()->get(DFontSizeManager::T3);
    ft1.setFamily("Noto Sans CJK SC");
    m_pDate->setFont(ft1);
    m_pDate->setText(time);

    //数量
    m_pNumCheckBox = new DCheckBox(this);
    connect(m_pNumCheckBox, &DCheckBox::clicked, this, &TimeLineDateWidget::onCheckBoxCliked);
    DFontSizeManager::instance()->bind(m_pNumCheckBox, DFontSizeManager::T6, QFont::Normal);
    QFont ft2 = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    ft2.setFamily("Noto Sans CJK SC");

    m_pNumCheckBox->setFont(ft2);
    m_pNumCheckBox->setText(num);

    m_pNum = new DLabel(this);
    DFontSizeManager::instance()->bind(m_pNum, DFontSizeManager::T6, QFont::Normal);
    m_pNum->setFont(ft2);
    m_pNum->setText(num);

    onShowCheckBox(false);

    //选择按钮
    m_chooseBtn = new DCommandLinkButton(QObject::tr("Select"));
    DFontSizeManager::instance()->bind(m_chooseBtn, DFontSizeManager::T5);
    m_chooseBtn->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T5));
    m_chooseBtn->setFocusPolicy(Qt::NoFocus);
    m_chooseBtn->setVisible(false);
    //占位btn，防止显影选择按钮时，ui变化
    m_pbtn = new DCommandLinkButton(" ");
    m_pbtn->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T5));
    m_pbtn->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *NumandBtnLayout = new QHBoxLayout();
    NumandBtnLayout->setContentsMargins(0, 0, 0, 0);
    NumandBtnLayout->addWidget(m_pNumCheckBox);
    NumandBtnLayout->addWidget(m_pNum);
    NumandBtnLayout->addStretch();
    NumandBtnLayout->addWidget(m_pbtn);
    NumandBtnLayout->addWidget(m_chooseBtn);

    QVBoxLayout *TitleViewLayout = new QVBoxLayout(this);
    TitleViewLayout->setContentsMargins(6, 0, 23, 0);
    TitleViewLayout->addWidget(m_pDate);
    TitleViewLayout->addStretch();
    TitleViewLayout->addLayout(NumandBtnLayout);
    this->setLayout(TitleViewLayout);

    onThemeChanged(DGuiApplicationHelper::instance()->themeType());

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &TimeLineDateWidget::onThemeChanged);
}

void TimeLineDateWidget::onThemeChanged(DGuiApplicationHelper::ColorType themeType)
{
    DPalette pal = DPaletteHelper::instance()->palette(m_pNumCheckBox);
    QColor color_BT = pal.color(DPalette::BrightText);
    if (themeType == DGuiApplicationHelper::LightType) {
        pal.setBrush(DPalette::Text, lightTextColor);
        m_pNumCheckBox->setForegroundRole(DPalette::Text);
        m_pNumCheckBox->setPalette(pal);
        m_pNum->setForegroundRole(DPalette::Text);
        m_pNum->setPalette(pal);
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        color_BT.setAlphaF(0.75);
        pal.setBrush(DPalette::Text, darkTextColor);
        m_pNumCheckBox->setForegroundRole(DPalette::Text);
        m_pNumCheckBox->setPalette(pal);
        m_pNum->setForegroundRole(DPalette::Text);
        m_pNum->setPalette(pal);
    }

    DPalette color = DPaletteHelper::instance()->palette(m_pDate);
    color.setBrush(DPalette::Text, themeType == DGuiApplicationHelper::LightType ? lightTextColor : darkTextColor);
    m_pDate->setPalette(color);
}

void TimeLineDateWidget::onShowCheckBox(bool bShow)
{
    m_pNumCheckBox->setVisible(bShow);
    m_pNum->setVisible(!bShow);
}

void TimeLineDateWidget::onCheckBoxCliked()
{
    if (m_pNumCheckBox->isChecked()) {
        emit sigIsSelectCurrentDatePic(true, m_currentItem);
    } else {
        emit sigIsSelectCurrentDatePic(false, m_currentItem);
    }
}

void TimeLineDateWidget::onChangeChooseBtnVisible(bool visible)
{
    m_chooseBtn->setVisible(visible);
}

void TimeLineDateWidget::onTimeLinePicSelectAll(bool selectall)
{
    m_pNumCheckBox->setChecked(selectall);
}

QString TimeLineDateWidget::onGetBtnStatus()
{
    return m_chooseBtn->text();
}

importTimeLineDateWidget::importTimeLineDateWidget(QStandardItem *item, const QString &time, const QString &num)
    : m_chooseBtn(nullptr), m_pDateandNumCheckBox(nullptr), m_currentItem(item)
{
    this->setContentsMargins(6, 0, 0, 0);
    this->setFixedHeight(35);

    //时间+照片数量
    m_pDateandNumCheckBox = new DCheckBox(this);
    connect(m_pDateandNumCheckBox, &DCheckBox::clicked, this, &importTimeLineDateWidget::onCheckBoxCliked);
    DFontSizeManager::instance()->bind(m_pDateandNumCheckBox, DFontSizeManager::T6, QFont::Normal);
    QFont ft1 = DFontSizeManager::instance()->get(DFontSizeManager::T6);
    ft1.setFamily("Noto Sans CJK SC");
    m_pDateandNumCheckBox->setFont(ft1);
    QString tempTimeAndNumber = time + " " + num;
    m_pDateandNumCheckBox->setText(tempTimeAndNumber);

    m_pDateandNum = new DLabel(this);
    DFontSizeManager::instance()->bind(m_pDateandNum, DFontSizeManager::T6, QFont::Normal);
    m_pDateandNum->setFont(ft1);
    m_pDateandNum->setText(tempTimeAndNumber);

    onShowCheckBox(false);

    //选择按钮
    m_chooseBtn = new DCommandLinkButton(QObject::tr("Select"));
    //connect(m_chooseBtn, &DCommandLinkButton::clicked, this, &importTimeLineDateWidget::onChooseBtnCliked);
    DFontSizeManager::instance()->bind(m_chooseBtn, DFontSizeManager::T5);
    m_chooseBtn->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T5));
    m_chooseBtn->setFocusPolicy(Qt::NoFocus);
    m_chooseBtn->setVisible(false);

    //占位btn，防止显影选择按钮时，ui变化
    m_pbtn = new DCommandLinkButton(" ");
    m_pbtn->setFont(DFontSizeManager::instance()->get(DFontSizeManager::T5));
    m_pbtn->setFocusPolicy(Qt::NoFocus);

    //开始布局
    QHBoxLayout *TitleViewLayout = new QHBoxLayout(this);
    TitleViewLayout->setContentsMargins(0, 0, 25, 0);
    TitleViewLayout->addWidget(m_pDateandNumCheckBox);
    TitleViewLayout->addWidget(m_pDateandNum);
    TitleViewLayout->addStretch();
    TitleViewLayout->addWidget(m_pbtn);
    TitleViewLayout->addWidget(m_chooseBtn);
    this->setLayout(TitleViewLayout);

    onThemeChanged(DGuiApplicationHelper::instance()->themeType());

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &importTimeLineDateWidget::onThemeChanged);
}

void importTimeLineDateWidget::onCheckBoxCliked()
{
    if (m_pDateandNumCheckBox->isChecked()) {
        emit sigIsSelectCurrentDatePic(true, m_currentItem);
    } else {
        emit sigIsSelectCurrentDatePic(false, m_currentItem);
    }
}

void importTimeLineDateWidget::onChangeChooseBtnVisible(bool visible)
{
    m_chooseBtn->setVisible(visible);
}

void importTimeLineDateWidget::onTimeLinePicSelectAll(bool selectall)
{
    m_pDateandNumCheckBox->setChecked(selectall);
}

QString importTimeLineDateWidget::onGetBtnStatus()
{
    return m_chooseBtn->text();
}

void importTimeLineDateWidget::onThemeChanged(DGuiApplicationHelper::ColorType themeType)
{
    DPalette pal = DPaletteHelper::instance()->palette(m_pDateandNumCheckBox);
    if (themeType == DGuiApplicationHelper::LightType) {
        pal.setBrush(DPalette::Text, lightTextColor);
        m_pDateandNumCheckBox->setForegroundRole(DPalette::Text);
        m_pDateandNumCheckBox->setPalette(pal);
        m_pDateandNum->setForegroundRole(DPalette::Text);
        m_pDateandNum->setPalette(pal);
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        pal.setBrush(DPalette::Text, darkTextColor);
        m_pDateandNumCheckBox->setForegroundRole(DPalette::Text);
        m_pDateandNumCheckBox->setPalette(pal);
        m_pDateandNum->setForegroundRole(DPalette::Text);
        m_pDateandNum->setPalette(pal);
    }
}

void importTimeLineDateWidget::onShowCheckBox(bool bShow)
{
    m_pDateandNumCheckBox->setVisible(bShow);
    m_pDateandNum->setVisible(!bShow);
}
