// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLURATTRIBUTIONWIDGET_H
#define BLURATTRIBUTIONWIDGET_H
#include <QAbstractItemDelegate>
#include "iattributionregister.h"
#include <DToolButton>
#include "attributewidget.h"

class BoxLayoutWidget;
class DrawBoard;
class SliderSpinBoxWidget;
class BlurAttributionWidget: public AttributeWgt
{
    Q_OBJECT
public:
    explicit BlurAttributionWidget(DrawBoard *drawBoard, QWidget *parent = nullptr);
protected:
    void initUi();
private:
    DrawBoard *m_drawBoard = nullptr;
    QToolButton *m_blurEffect = nullptr;           //模糊
    QToolButton *m_masicoEffect = nullptr;           //高斯模糊
    BoxLayoutWidget  *m_blurStyleWidget = nullptr;
};
#endif // BLURATTRIBUTIONWIDGET_H
