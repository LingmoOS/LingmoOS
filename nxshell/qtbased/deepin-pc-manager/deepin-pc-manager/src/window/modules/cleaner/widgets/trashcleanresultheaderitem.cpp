// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcleanresultheaderitem.h"

#include <DFontSizeManager>
#include <DPalette>
#include <DPaletteHelper>

#include <QGridLayout>

DWIDGET_USE_NAMESPACE

TrashCleanResultHeaderItem::TrashCleanResultHeaderItem(QWidget *parent)
    : DFrame(parent)
    , m_infos(nullptr)
    , m_actionInfo(nullptr)
    , m_scanTimeInfo(nullptr)
    , m_scanIcon(nullptr)
{
    this->setFixedSize(750, 146);

    this->setLineWidth(0);
    this->setBackgroundRole(DPalette::Window);
    DPalette dpa = DPaletteHelper::instance()->palette(this, this->palette());
    dpa.setColor(DPalette::Window, QColor::fromRgb(163, 211, 255, int(0.13 * 255)));
    DPaletteHelper::instance()->setPalette(this, dpa);

    initUI();
}

void TrashCleanResultHeaderItem::setScanStarted()
{
    // 扫描计时
    // 扫描计时初始化
    m_scanTimeInfo->setVisible(true);
}

void TrashCleanResultHeaderItem::setScanStopped(const QString &totalFiles)
{
    m_actionInfo->setText(tr("Scanned: %1 files").arg(totalFiles));
}

void TrashCleanResultHeaderItem::setCleanFinished(const QString &titleInfo, const QString &tipInfo)
{
    m_scanTimeInfo->setVisible(false);
    m_infos->setText(titleInfo);
    m_actionInfo->setText(tipInfo);
}

void TrashCleanResultHeaderItem::updateScanPath(const QString &pash)
{
    m_actionInfo->setText(tr("Scanning: %1").arg(pash));
}

void TrashCleanResultHeaderItem::updateScanTitleInfo(const QString &total, const QString &selected)
{
    m_infos->setText(tr("%1 junk files, %2 selected").arg(total).arg(selected));
}

void TrashCleanResultHeaderItem::setTitle(const QString &info)
{
    m_infos->setText(info);
}

void TrashCleanResultHeaderItem::setTip(const QString &info)
{
    m_actionInfo->setText(info);
}

void TrashCleanResultHeaderItem::setTimeInfo(const QString &info)
{
    m_scanTimeInfo->setText(info);
}

void TrashCleanResultHeaderItem::setLogo(const QString &iconPath)
{
    m_scanIcon->setPixmap(QIcon::fromTheme(iconPath).pixmap(265, 146));
}

void TrashCleanResultHeaderItem::initUI()
{
    m_infos = new DLabel("", this);
    m_infos->setAccessibleName("headerInfos");
    m_infos->setFixedHeight(30);
    QFont infoFont = m_infos->font();
    infoFont.setPixelSize(20);
    m_infos->setFont(infoFont);

    m_scanIcon = new DLabel(this);
    m_scanIcon->setFixedHeight(146);
    m_scanIcon->setAccessibleName("headerIcon");
    m_scanIcon->setPixmap(
        QPixmap("/home/test/defender-parts/dcc_antiav_whitelist_16px.svg").scaledToHeight(116));

    m_actionInfo = new DTipLabel("", this);
    m_actionInfo->setAccessibleName("headerActionInfo");
    DFontSizeManager::instance()->unbind(m_actionInfo);
    m_actionInfo->setFixedHeight(17);
    m_actionInfo->setFixedWidth(500);
    QFont aiFont = m_actionInfo->font();
    aiFont.setPixelSize(12);
    m_actionInfo->setFont(aiFont);
    m_actionInfo->setElideMode(Qt::ElideMiddle);
    m_actionInfo->setAlignment(Qt::AlignLeft);

    m_scanTimeInfo = new DTipLabel("", this);
    m_scanTimeInfo->setAccessibleName("headerScanTimeInfo");
    DFontSizeManager::instance()->unbind(m_scanTimeInfo);
    m_scanTimeInfo->setFixedHeight(17);
    QFont stiFont = m_scanTimeInfo->font();
    aiFont.setPixelSize(12);
    m_scanTimeInfo->setFont(aiFont);

    // 高度146
    // 列分布 1空白 30 2标题 30 3空白 10 4标题 17 5空白 10 6标题 17 7空白 32
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setRowStretch(0, 30);
    mainLayout->setRowMinimumHeight(0, 30);
    mainLayout->setRowStretch(1, 30);
    mainLayout->setRowMinimumHeight(1, 30);
    mainLayout->setRowStretch(2, 37);
    mainLayout->setRowMinimumHeight(2, 37);
    mainLayout->setRowStretch(3, 17);
    mainLayout->setRowMinimumHeight(3, 14);
    mainLayout->setRowStretch(4, 32);
    mainLayout->setRowMinimumHeight(4, 32);

    // 行分布 1空白 30 2标题574   3图标146
    mainLayout->setColumnStretch(0, 30);
    mainLayout->setColumnMinimumWidth(0, 30);
    mainLayout->setColumnStretch(1, 565);
    mainLayout->setColumnMinimumWidth(1, 565);
    mainLayout->setColumnStretch(2, 205);
    mainLayout->setColumnMinimumWidth(2, 205);

    mainLayout->addWidget(m_infos, 1, 1, Qt::AlignLeft);
    mainLayout->addWidget(m_actionInfo, 2, 1, Qt::AlignLeft);
    mainLayout->addWidget(m_scanTimeInfo, 3, 1, Qt::AlignLeft);

    mainLayout->addWidget(m_scanIcon, 0, 2, -1, 1, Qt::AlignVCenter | Qt::AlignLeft);

    this->setLayout(mainLayout);
}
