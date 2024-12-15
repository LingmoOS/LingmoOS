// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROUNDEDANGLESTRYLEWIDGET_H
#define ROUNDEDANGLESTRYLEWIDGET_H
#include "attributewidget.h"
#include <DLabel>
#include <DWidget>
#include <DSpinBox>
#include <DSlider>
#include <DToolButton>
DWIDGET_USE_NAMESPACE

class CSpinBox;
class NumberSlider;
class BoxLayoutWidget;
class  DrawBoard;

class BackgroundWidget : public QWidget
{
public:
    using QWidget::QWidget;
    void setBackgroudColor(QColor c);
    virtual void paintEvent(QPaintEvent *event) override;
private:
    QColor m_bkColor = Qt::transparent;
};

class RectRadiusStyleWidget : public AttributeWgt
{
    Q_OBJECT
public:
    explicit RectRadiusStyleWidget(DrawBoard *drawBoard, QWidget *parent = nullptr);
    void setVar(const QVariant &var) override;
private:
    void initUi();
    void initConnect();
    void emitValueChange(QVariant left, QVariant right, QVariant leftBottom, QVariant rightBottom, EChangedPhase phase = EChanged);
    void setSpinBoxValue(CSpinBox *s, int value);
    void showByChecked(bool bDiffMode = false);
signals:
    void valueChanged(QVariant value, EChangedPhase phase);
private:
    QButtonGroup *m_checkgroup = nullptr;
    CSpinBox    *m_left = nullptr;
    CSpinBox    *m_right = nullptr;
    CSpinBox    *m_leftBottom = nullptr;
    CSpinBox    *m_rightBottom = nullptr;
    DSlider    *m_radiusSlider = nullptr;
    CSpinBox        *m_radius = nullptr;
    DToolButton *m_sameRadiusButton = nullptr;
    DToolButton *m_diffRadiusButton = nullptr;
    BackgroundWidget     *m_diffRadiusWidget = nullptr;
    BoxLayoutWidget     *m_sameRadiusWidget = nullptr;
    DrawBoard           *m_drawboard = nullptr;
};

#endif // ROUNDEDANGLESTRYLEWIDGET_H
