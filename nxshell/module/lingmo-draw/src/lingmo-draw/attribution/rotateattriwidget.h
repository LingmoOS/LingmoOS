// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROTATEATTRIWIDGET_H
#define ROTATEATTRIWIDGET_H
#include <QPushButton>
#include <QLineEdit>
#include <DDoubleSpinBox>
#include <DToolButton>

#include "attributewidget.h"

DWIDGET_USE_NAMESPACE
class DrawBoard;
class PageItem;
class PageScene;
class CDoubleSpinBox;
class QTimer;

class RotateAttriWidget: public AttributeWgt
{
    Q_OBJECT
public:
    RotateAttriWidget(DrawBoard *drawBoard, QWidget *parent = nullptr);
    void setVar(const QVariant &var) override;
    void setAngle(double angle);
    bool eventFilter(QObject *o, QEvent *e) override;
private:
    void initUi();
    void initConnect();
    void showTooltip();
    double checkValue(double value);
protected:
    void showEvent(QShowEvent *event) override;
signals:
    void buttonClicked(bool doHorFlip, bool doVerFlip);

protected:
    Q_SLOT void onSceneSelectionChanged(const QList<PageItem * > &selectedItems);

private:
    QLabel *m_label;
    CDoubleSpinBox *m_angle;
    DToolButton *m_horFlipBtn;
    DToolButton *m_verFlipBtn;
    DIconButton *m_clockwiseBtn;
    DIconButton *m_anticlockwiseBtn;
    DrawBoard   *m_drawBoard = nullptr;
    PageScene   *m_currentScene = nullptr;
    QTimer      *m_delayHideTimer = nullptr;        // 延迟隐藏提示信息的定时器
};
#endif // ROTATEATTRIWIDGET_H
