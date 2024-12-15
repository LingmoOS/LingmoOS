// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEM_SERVICE_TABLE_HEADER_VIEW_H
#define SYSTEM_SERVICE_TABLE_HEADER_VIEW_H

#include <DHeaderView>

DWIDGET_USE_NAMESPACE
/**
 * @brief The LogViewHeaderView class
 * 表头
 */
class LogViewHeaderView : public DHeaderView
{
public:
    explicit LogViewHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~LogViewHeaderView() override;

    QSize sizeHint() const override;
    int sectionSizeHint(int logicalIndex) const;

    inline int getSpacing() const { return m_spacing; }
    inline void setSpacing(int spacing) { m_spacing = spacing; }

private slots:
    /**
     * @brief 根据布局模式(紧凑)变更更新界面布局
     */
    void updateSizeMode();
protected:
    void paintEvent(QPaintEvent *e) override;
    virtual void paintSection(QPainter *painter, const QRect &rect,
                              int logicalIndex) const override;
    void focusInEvent(QFocusEvent *event) override;

private:
    int m_spacing {1};
    Qt::FocusReason m_reson = Qt::MouseFocusReason;
    QStyleOptionHeader *m_option {nullptr};
};

#endif // SYSTEM_SERVICE_TABLE_HEADER_VIEW_H
