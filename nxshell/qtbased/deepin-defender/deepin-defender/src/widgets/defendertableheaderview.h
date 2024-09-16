// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFENDERTABLEHEADERVIEW_H
#define DEFENDERTABLEHEADERVIEW_H

#include <DHeaderView>

DWIDGET_USE_NAMESPACE

// DHeaderView重载类
class DefenderTableHeaderView : public DHeaderView
{
public:
    DefenderTableHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    // 表头seize
    QSize sizeHint() const override;
    int sectionSizeHint(int logicalIndex) const;

    // 表头间隔
    inline int getSpacing() const { return m_spacing; }
    inline void setSpacing(int spacing) { m_spacing = spacing; }

    // 掩藏某列表头下标
    void setHeadViewSortHide(int nIndex);

protected:
    // 表头类绘制事件
    void paintEvent(QPaintEvent *e) override;
    virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

    // 焦点事件
    void focusInEvent(QFocusEvent *event) override;

private:
    int m_spacing;
    int m_nHideSortIndex;
    Qt::FocusReason m_reson;
};

#endif  // DEFENDERTABLEHEADERVIEW_H
