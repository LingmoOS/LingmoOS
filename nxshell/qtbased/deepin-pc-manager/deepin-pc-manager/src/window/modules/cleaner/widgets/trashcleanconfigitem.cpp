// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcleanconfigitem.h"

#include <DBackgroundGroup>
#include <DFontSizeManager>
#include <DFrame>
#include <DLabel>
#include <DTipLabel>

#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>

DWIDGET_USE_NAMESPACE

TrashCleanConfigItem::TrashCleanConfigItem(QWidget *parent)
    : DFrame(parent)
    , m_checkbox(nullptr)
    , m_icon(nullptr)
    , m_title(nullptr)
    , m_tips(nullptr)
    , m_textFrame(nullptr)
{
    this->setFixedSize(516, 34);
    this->setFrameShape(QFrame::NoFrame);
    this->setFocusPolicy(Qt::NoFocus);
    this->setLineWidth(0);
    this->setContentsMargins(0, 0, 0, 0);

    initUI();
    initConnection();
}

void TrashCleanConfigItem::setTitles(const QString &mainTitle, const QString &tipTitle)
{
    m_title->setText(mainTitle);
    m_tips->setText(tipTitle);
}

void TrashCleanConfigItem::setCheckBoxStatus(bool isChecked)
{
    m_checkbox->setChecked(isChecked);
}

void TrashCleanConfigItem::setIconPath(const QString &path)
{
    m_icon->setPixmap(QIcon::fromTheme(path).pixmap(16, 16));
}

void TrashCleanConfigItem::initUI()
{
    m_icon = new DLabel(this);
    m_icon->setAccessibleName("configItemIcon");

    m_title = new DLabel("title", this);
    m_title->setAccessibleName("configItemTitle");
    QFont titleFont = m_title->font();
    titleFont.setPixelSize(14);
    titleFont.setWeight(QFont::Medium);
    m_title->setFont(titleFont);

    m_tips = new DTipLabel("tip", this);
    m_tips->setAccessibleName("configItemTip");
    DFontSizeManager::instance()->unbind(m_tips);
    QFont tipFont = m_tips->font();
    tipFont.setPixelSize(11);
    m_tips->setFont(tipFont);

    m_checkbox = new QCheckBox(this);
    m_checkbox->setAccessibleName("configItemCheckbox");
    m_checkbox->setFixedSize(20, 20);
    m_checkbox->setTristate(false);
    m_checkbox->setFocusPolicy(Qt::NoFocus);

    m_textFrame = new DFrame(this);
    m_textFrame->setFixedSize(450, 34);
    m_textFrame->setBackgroundRole(DPalette::ItemBackground);
    m_textFrame->setLineWidth(0);
    m_textFrame->setContentsMargins(0, 0, 0, 0);
    QGridLayout *textLayout = new QGridLayout;
    textLayout->setContentsMargins(14, 0, 0, 0);
    textLayout->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
    textLayout->setColumnMinimumWidth(0, 70);
    textLayout->setColumnMinimumWidth(1, 20);
    textLayout->addWidget(m_title, 0, 0, Qt::AlignLeft);
    textLayout->addWidget(m_tips, 0, 2, Qt::AlignLeft);
    m_textFrame->setLayout(textLayout);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_checkbox, 0, 0, Qt::AlignLeft);
    mainLayout->addWidget(m_icon, 0, 1, Qt::AlignLeft);
    mainLayout->addWidget(m_textFrame, 0, 2, Qt::AlignLeft);

    this->setLayout(mainLayout);
}

void TrashCleanConfigItem::initConnection()
{
    connect(m_checkbox, &QCheckBox::stateChanged, this, [&](int stage) {
        qDebug() << "stagechanged";
        Q_EMIT this->stageChanged(m_configIndex, stage);
    });
}
