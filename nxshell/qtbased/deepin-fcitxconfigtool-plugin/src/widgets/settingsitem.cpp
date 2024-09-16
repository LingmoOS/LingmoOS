// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingsitem.h"

#include <DPalette>
#include <DStyle>

#include <QStyle>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QLabel>
#include <QApplication>
#include <DStyle>
#include <DApplication>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace dcc_fcitx_configtool {
namespace widgets {

FcitxSettingsItem::FcitxSettingsItem(QWidget *parent)
    : QFrame(parent)
    , m_isErr(false)
{
}

bool FcitxSettingsItem::isErr() const
{
    return m_isErr;
}

void FcitxSettingsItem::setIsErr(const bool err)
{
    if (m_isErr == err)
        return;
    m_isErr = err;

    style()->unpolish(this);
    style()->polish(this);
}

void FcitxSettingsItem::addBackground()
{
    //加入一个 DFrame 作为圆角背景
    if (m_bgGroup)
        m_bgGroup->deleteLater();
    m_bgGroup = new DFrame(this);
    m_bgGroup->setBackgroundRole(DPalette::ItemBackground);
    m_bgGroup->setLineWidth(0);
    DStyle::setFrameRadius(m_bgGroup, 8);

    //将 m_bgGroup 沉底
    m_bgGroup->lower();
    //设置m_bgGroup 的大小
    m_bgGroup->setFixedSize(size());
}

void FcitxSettingsItem::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    //设置m_bgGroup 的大小
    if (m_bgGroup)
        m_bgGroup->setFixedSize(size());
}

} // namespace widgets
} // namespace dcc
