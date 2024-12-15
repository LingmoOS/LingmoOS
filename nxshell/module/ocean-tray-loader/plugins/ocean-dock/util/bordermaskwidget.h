// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BORDERMASKWIDGET_H
#define BORDERMASKWIDGET_H

#include <DBlurEffectWidget>

#include <QWidget>

DWIDGET_USE_NAMESPACE

class BorderMaskWidget : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit BorderMaskWidget(QWidget *parent = nullptr);

    void setRadius(int radius);
    void setBackgroundColor(const QColor &backColor);
    void setOuterBorderColor(const QColor &borderColor);
    void setOuterBorderWidth(int borderWidth);
    void setInnerBorderColor(const QColor &borderColor);
    void setInnerBorderWidth(int borderWidth);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    QColor m_backColor;
    int m_radius;
    QColor m_outerBorderColor;
    int m_outerBorderWidth;
    QColor m_innerBorderColor;
    int m_innerBorderWidth;
};

#endif // BORDERMASKWIDGET_H
