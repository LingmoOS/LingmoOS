// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/namespace.h"

#include <dtkwidget_global.h>

#include <DFrame>
#include <DLabel>
#include <DTipLabel>

class QIcon;
class QGraphicsOpacityEffect;

DWIDGET_USE_NAMESPACE

DEF_NAMESPACE_BEGIN
class PwdLimitLevelItem : public DFrame
{
    Q_OBJECT
public:
    PwdLimitLevelItem(QWidget *parent = nullptr);
    ~PwdLimitLevelItem();
    // 设置左侧图标
    void setIcon(const QIcon &icon);
    // 设置等级文字
    void setLevelText(const QString &text);
    // 设置提示文字
    void setTipText(const QString &text);
    // 设置选中图标
    inline void setSelectedIcon(const QIcon &icon)
    {
        m_selectedIcon = icon;
    }
    // 设置非选中图标
    inline void setNotSelectIcon(const QIcon &icon)
    {
        m_notSelectIcon = icon;
    }
    // 设置是否选中
    void setSelected(const bool &selected);
    // 设置是否可用
    void setEnable(const bool &enable);

    // 设置相关标签名称
    void setAccessibleParentText(QString sAccessibleName);

protected:
    void mousePressEvent(QMouseEvent *event);

Q_SIGNALS:
    void clicked();

private:
    QWidget *m_firstLayerWidgt;
    DLabel *m_icon; // 左侧图标
    DLabel *m_levelLabel; // 等级文字
    DTipLabel *m_tipLabel; // 提示文字
    QIcon m_selectedIcon; // 选中图标
    QIcon m_notSelectIcon; // 非选中图标
    DLabel *m_selectIconLabel; // 是否选中图标标签
    bool m_canClick; // 是否可点击
};
DEF_NAMESPACE_END
