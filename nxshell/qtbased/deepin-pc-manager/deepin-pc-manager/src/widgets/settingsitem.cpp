// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingsitem.h"

#include <DPalette>

#include <QResizeEvent>
#include <QStyle>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace def {
namespace widgets {

SettingsItem::SettingsItem(QWidget *parent)
    : QFrame(parent)
    , m_isErr(false)
{
}

bool SettingsItem::isErr() const
{
    return m_isErr;
}

void SettingsItem::setIsErr(const bool err)
{
    if (m_isErr == err)
        return;
    m_isErr = err;

    style()->unpolish(this);
    style()->polish(this);
}

void SettingsItem::addBackground()
{
    // 加入一个 DFrame 作为圆角背景
    if (m_bgGroup)
        m_bgGroup->deleteLater();
    m_bgGroup = new DFrame(this);
    //    m_bgGroup->setContentsMargins(0, 0, 0, 0);
    m_bgGroup->setBackgroundRole(DPalette::ItemBackground);
    m_bgGroup->setLineWidth(0);

    // 将 m_bgGroup 沉底
    m_bgGroup->lower();
    // 设置m_bgGroup 的大小
    m_bgGroup->setFixedSize(size());
}

void SettingsItem::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    // 设置m_bgGroup 的大小
    if (m_bgGroup)
        m_bgGroup->setFixedSize(size());
}
} // namespace widgets
} // namespace def
