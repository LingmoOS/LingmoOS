// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <QListView>
#include <QStyledItemDelegate>
#include "clock.h"

class QVBoxLayout;
namespace dwclock {
class ClockDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    ClockDelegate(QObject *parent = Q_NULLPTR);

public:
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QString utcOffsetText(const int offset) const;
    QString promptOfUtcOffsetText(const int offset) const;

    mutable Clock m_clock;
};

class ClockView : public QListView
{
    Q_OBJECT
public:
    explicit ClockView (QWidget *parent = nullptr);
    virtual ~ClockView() override;

    bool isSmallType() const;
    void setSmallType(const bool small);

private:
    bool m_isSmallType = false;
};

class ClockPanel : public QWidget {
    Q_OBJECT

    Q_PROPERTY(int roundedCornerRadius READ roundedCornerRadius WRITE setRoundedCornerRadius NOTIFY roundedCornerRadiusChanged FINAL)
public:
    explicit ClockPanel(QWidget *parent = nullptr);
    ClockView *view() const;

    void setSmallType(const bool isSmallType);
    int roundedCornerRadius() const;
    void setRoundedCornerRadius(int newRoundedCornerRadius);

signals:
    void roundedCornerRadiusChanged();

protected:
    virtual void paintEvent(QPaintEvent *event) override;
private:
    ClockView *m_view = nullptr;
    QVBoxLayout *m_layout = nullptr;
    int m_roundedCornerRadius;
};
}
