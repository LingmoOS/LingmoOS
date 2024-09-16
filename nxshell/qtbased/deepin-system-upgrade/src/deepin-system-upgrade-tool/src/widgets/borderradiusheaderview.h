// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DHeaderView>
#include <DLabel>

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QMouseEvent>


DWIDGET_USE_NAMESPACE

class BorderRadiusHeaderView : public DHeaderView
{
Q_OBJECT

public:
    explicit BorderRadiusHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

Q_SIGNALS:
    void selectAllButtonClicked(bool);

protected:
    virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void paintEvent(QPaintEvent *event) override;
};
