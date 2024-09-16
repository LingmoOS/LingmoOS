// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef ICONLABEL_H
#define ICONLABEL_H

#include <QWidget>
#include <QBoxLayout>

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class IconLabel : public QWidget
{
public:
    explicit IconLabel(QWidget *parent = 0, bool wordWrap = false);

    QString title() const;
    void setTitle(const QString &title);
    void setIcon(const QIcon &icon, int extent = 0);
    void setDirection(QBoxLayout::Direction direction);

    void setChecked(bool checked);

    void setHoverBackground(const QColor &color);

private:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;

    QLabel *m_icon;
    QLabel *m_label;

    bool m_isChecked = false;
    QColor m_background;
};

#endif // ICONLABEL_H
