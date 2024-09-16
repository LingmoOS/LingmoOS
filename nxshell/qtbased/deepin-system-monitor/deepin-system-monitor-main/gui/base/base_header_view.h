// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASE_HEADER_VIEW_H
#define BASE_HEADER_VIEW_H

#include <DHeaderView>

DWIDGET_USE_NAMESPACE

class QModelIndex;

/**
 * @brief Customized HeaderView
 */
class BaseHeaderView : public DHeaderView
{
    Q_OBJECT

public:
    /**
     * @brief BaseHeaderView constructor
     * @param orientation Header orientation
     * @param parent Parent Widget
     */
    explicit BaseHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    /**
     * @brief sizeHint Hint size of this widget
     * @return Hint size
     */
    QSize sizeHint() const override;

protected:
    /**
     * @brief paintEvent Paint event handler for header view
     * @param e Paint event object
     */
    void paintEvent(QPaintEvent *e) override;
    /**
     * @brief eventFilter Event filter for header view
     * @param obj Object being watched
     * @param ev Event to be filtered
     * @return true: Event being handled & no further processing; false: event should be propagated further
     */
    bool eventFilter(QObject *obj, QEvent *ev) override;

    /**
     * @brief focusInEvent
     */
    void focusInEvent(QFocusEvent *event) override;

private:
    int m_focusReason = Qt::TabFocusReason;
};

#endif  // BASE_HEADER_VIEW_H
