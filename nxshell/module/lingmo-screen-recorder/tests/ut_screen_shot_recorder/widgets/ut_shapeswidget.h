// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QMap>

#include <QList>

#define private public
#include "stub.h"
#include "addr_pri.h"
#include "../../src/main_window.h"
#include "../../src/widgets/shapeswidget.h"

ACCESS_PRIVATE_FUN(ShapesWidget, void(QMouseEvent *event), mousePressEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QMouseEvent *event), mouseReleaseEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QMouseEvent *event), mouseMoveEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPaintEvent *e), paintEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QEvent *e), enterEvent);
ACCESS_PRIVATE_FUN(ShapesWidget, bool(QPointF pos), clickedShapes);
ACCESS_PRIVATE_FUN(ShapesWidget, void(TextEdit *edit, QRectF newRect), updateTextRect);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, QPointF, QPixmap, bool), paintImgPoint);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, FourPoints, int, ShapesWidget::ShapeBlurStatus, bool, bool), paintRect);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, FourPoints, int, ShapesWidget::ShapeBlurStatus, bool, bool), paintEllipse);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, QList<QPointF>, int, bool), paintArrow);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, QList<QPointF>), paintLine);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPainter &, FourPoints), paintText);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QPinchGesture *pinch), pinchTriggered);
ACCESS_PRIVATE_FUN(ShapesWidget, void(QTapGesture *tap), tapTriggered);

ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isSelectedText)
ACCESS_PRIVATE_FIELD(ShapesWidget, Toolshape, m_selectedShape)
ACCESS_PRIVATE_FIELD(ShapesWidget, int, m_selectedIndex)
ACCESS_PRIVATE_FIELD(ShapesWidget, int, m_selectedOrder)
ACCESS_PRIVATE_FIELD(ShapesWidget, Toolshapes, m_shapes)
ACCESS_PRIVATE_FIELD(ShapesWidget, QString, m_currentType)
ACCESS_PRIVATE_FIELD(ShapesWidget, QPointF, m_pos1)
ACCESS_PRIVATE_FIELD(ShapesWidget, Toolshape, m_currentShape)
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isArrowRotated)
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isRecording)
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isPressed)
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isShiftPressed)
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isRotated)
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isResize)
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isSelected)
ACCESS_PRIVATE_FIELD(ShapesWidget, ResizeDirection, m_resizeDirection)
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isMoving)
ACCESS_PRIVATE_FIELD(ShapesWidget, bool, m_isHovered)

using namespace testing;
bool toBool_stub()
{
    return true;
}

bool clickedOnRect_stub(FourPoints rectPoints, QPointF pos, bool isBlurMosaic = false)
{
    Q_UNUSED(rectPoints)
    Q_UNUSED(pos)
    Q_UNUSED(isBlurMosaic)
    return true;
}

bool pointInRect_stub(FourPoints fourPoints, QPointF pos)
{
    Q_UNUSED(fourPoints)
    Q_UNUSED(pos)
    return true;
}


bool clickedOnEllipse_stub(FourPoints mainPoints, QPointF pos, bool isBlurMosaic = false)
{
    Q_UNUSED(mainPoints)
    Q_UNUSED(pos)
    Q_UNUSED(isBlurMosaic)
    return true;
}

bool clickedOnArrow_stub(QList<QPointF> points, QPointF pos)
{
    Q_UNUSED(points)
    Q_UNUSED(pos)
    return true;
}

bool clickedOnLine_stub(FourPoints mainPoints, QList<QPointF> points, QPointF pos)
{
    Q_UNUSED(mainPoints)
    Q_UNUSED(points)
    Q_UNUSED(pos)
    return true;
}

bool clickedOnText_stub(FourPoints mainPoints, QPointF pos)
{
    Q_UNUSED(mainPoints)
    Q_UNUSED(pos)
    return true;
}

class ShapesWidgetTest: public testing::Test
{

public:
    Stub stub;
    ShapesWidget *shapesWidget;
    virtual void SetUp() override
    {
        shapesWidget = new ShapesWidget();
    }

    virtual void TearDown() override
    {
        if (nullptr != shapesWidget)
            delete shapesWidget;
    }
};

TEST_F(ShapesWidgetTest, enterEvent)
{
    QEvent *e = new QEvent(QEvent::Enter);
    call_private_fun::ShapesWidgetenterEvent(*shapesWidget, e);

    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("line");
    call_private_fun::ShapesWidgetenterEvent(*shapesWidget, e);

    delete e;
}

TEST_F(ShapesWidgetTest, paintEvent)
{
    QPaintEvent *e = new QPaintEvent(QRect());
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    shapesWidget->m_currentShape.type = QString("text");
    toolShape1.type = QString("rectangle");
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    Toolshape toolShape3;
    toolShape3.type = QString("arrow");
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    toolShape3.points = points;
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    shapesWidget->m_pos1 = QPointF(2, 2);
    shapesWidget->m_pos2 = QPointF(2, 2);

    shapesWidget->m_currentType = QString("oval");
    call_private_fun::ShapesWidgetpaintEvent(*shapesWidget, e);

    shapesWidget->m_currentType = QString("arrow");
    call_private_fun::ShapesWidgetpaintEvent(*shapesWidget, e);

    shapesWidget->m_currentType = QString("line");
    call_private_fun::ShapesWidgetpaintEvent(*shapesWidget, e);

    shapesWidget->m_currentType = QString("text");
    call_private_fun::ShapesWidgetpaintEvent(*shapesWidget, e);

    delete e;
}

TEST_F(ShapesWidgetTest, pinchTriggered)
{
    QPinchGesture *pinch = new QPinchGesture();
    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = -1;
    call_private_fun::ShapesWidgetpinchTriggered(*shapesWidget, pinch);

    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    call_private_fun::ShapesWidgetpinchTriggered(*shapesWidget, pinch);

    delete pinch;
}

TEST_F(ShapesWidgetTest, mousePressEvent)
{
    QMap<int, TextEdit *> editMap;
    TextEdit *textEdit = new TextEdit(-1, nullptr);
    TextEdit *textEdit0 = new TextEdit(0, nullptr);
    TextEdit *textEdit1 = new TextEdit(1, nullptr);
    TextEdit *textEdit2 = new TextEdit(2, nullptr);
    editMap.insert(-1, textEdit);
    editMap.insert(0, textEdit0);
    editMap.insert(1, textEdit1);
    editMap.insert(2, textEdit2);
    shapesWidget->m_editMap = editMap;
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    Toolshape toolShape3;
    toolShape3.type = QString("arrow");
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;

    QMouseEvent *ev0 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget, ev0);
    delete ev0;
    //1220
    QMouseEvent *ev1 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("line");
    access_private_field::ShapesWidgetm_pos1(*shapesWidget) = QPointF(0, 0);
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget, ev1);
    delete ev1;

    QMouseEvent *ev2 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_pos1(*shapesWidget) = QPointF(0, 0);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("arrow");
    access_private_field::ShapesWidgetm_currentShape(*shapesWidget).isStraight = true;
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget, ev2);
    delete ev2;

    QMouseEvent *ev21 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_pos1(*shapesWidget) = QPointF(0, 0);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("arrow");
    access_private_field::ShapesWidgetm_currentShape(*shapesWidget).isStraight = false;
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget, ev21);
    delete ev21;

    QMouseEvent *ev3 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_pos1(*shapesWidget) = QPointF(0, 0);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("rectangle");
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget, ev3);
    delete ev3;

    QMouseEvent *ev4 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_pos1(*shapesWidget) = QPointF(0, 0);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("oval");
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget, ev4);
    delete ev4;

    QMouseEvent *ev5 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_pos1(*shapesWidget) = QPointF(0, 0);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("text");
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget, ev5);
    delete ev5;

    //1162
    QMouseEvent *ev6 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget, ev6);
    delete ev6;

    //1178
    QMouseEvent *ev7 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = -1;
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget, ev7);
    delete ev7;

    //1220
    QMouseEvent *ev8 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = -1;
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("line");
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget, ev8);
    delete ev8;

    QMouseEvent *ev9 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("arrow");
    call_private_fun::ShapesWidgetmousePressEvent(*shapesWidget, ev9);
    delete ev9;

    delete textEdit;
    delete textEdit0;
    delete textEdit1;
    delete textEdit2;
}

TEST_F(ShapesWidgetTest, mouseMoveEvent)
{
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("line");
    shapesWidget->m_clickedKey = ShapesWidget::ClickedKey::Fifth;
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    Toolshape toolShape3;
    toolShape3.type = QString("arrow");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    shapesWidget->m_selectedOrder = 0;
    shapesWidget->m_isResize = true;
    shapesWidget->m_isPressed = true;
    shapesWidget->m_isSelected = true;
    shapesWidget->m_selectedIndex = 0;
    shapesWidget->m_currentType = QString("arrow");
    QMouseEvent *ev = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    access_private_field::ShapesWidgetm_isRecording(*shapesWidget) = true;
    access_private_field::ShapesWidgetm_isPressed(*shapesWidget) = true;
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    access_private_field::ShapesWidgetm_currentShape(*shapesWidget).type = QString("arrow");
    access_private_field::ShapesWidgetm_currentShape(*shapesWidget).points = points;
    access_private_field::ShapesWidgetm_isShiftPressed(*shapesWidget) = true;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_isShiftPressed(*shapesWidget) = false;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);

    QList<QPointF> points1;
    points1 << QPointF(96, 235);
    access_private_field::ShapesWidgetm_currentShape(*shapesWidget).type = QString("arrow");
    access_private_field::ShapesWidgetm_currentShape(*shapesWidget).points = points1;
    access_private_field::ShapesWidgetm_isShiftPressed(*shapesWidget) = true;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_isShiftPressed(*shapesWidget) = false;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_currentShape(*shapesWidget).type = QString("line");
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);

    access_private_field::ShapesWidgetm_isRecording(*shapesWidget) = false;
    access_private_field::ShapesWidgetm_isPressed(*shapesWidget) = true;
    access_private_field::ShapesWidgetm_isRotated(*shapesWidget) = true;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);

    access_private_field::ShapesWidgetm_isResize(*shapesWidget) = true;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);

    access_private_field::ShapesWidgetm_isPressed(*shapesWidget) = false;
    access_private_field::ShapesWidgetm_isRecording(*shapesWidget) = false;
//    QList <Toolshape> toolShapes;
//    Toolshape toolShape1;
//    toolShape1.type = QString("rectangle");
//    Toolshape toolShape2;
//    toolShape2.type = QString("oval");
//    Toolshape toolShape3;
//    toolShape3.type = QString("arrow");
//    toolShapes << toolShape1;
//    toolShapes << toolShape2;
//    toolShapes << toolShape3;
//    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    access_private_field::ShapesWidgetm_isRotated(*shapesWidget) = true;
    access_private_field::ShapesWidgetm_isSelected(*shapesWidget) = true;
    access_private_field::ShapesWidgetm_resizeDirection(*shapesWidget) = Left;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_resizeDirection(*shapesWidget) = Top;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_resizeDirection(*shapesWidget) = Right;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_resizeDirection(*shapesWidget) = Bottom;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_resizeDirection(*shapesWidget) = TopLeft;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_resizeDirection(*shapesWidget) = BottomLeft;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_resizeDirection(*shapesWidget) = TopRight;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_resizeDirection(*shapesWidget) = BottomRight;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_resizeDirection(*shapesWidget) = Rotate;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_resizeDirection(*shapesWidget) = Moving;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);
    access_private_field::ShapesWidgetm_resizeDirection(*shapesWidget) = Outting;
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);

    access_private_field::ShapesWidgetm_isHovered(*shapesWidget) = false;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget).clear();
    call_private_fun::ShapesWidgetmouseMoveEvent(*shapesWidget, ev);

    delete ev;
}

TEST_F(ShapesWidgetTest, mouseReleaseEvent)
{
    shapesWidget->m_isRecording = true;
    shapesWidget->m_isSelected = false;
    shapesWidget->m_pos2 = QPointF(10, 10);
    shapesWidget->m_isRotated = false;
    shapesWidget->m_selectedIndex = 0;
    shapesWidget->m_currentType = QString("arrow");
    QMouseEvent *ev = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::ShapesWidgetmouseReleaseEvent(*shapesWidget, ev);

    delete ev;
}

TEST_F(ShapesWidgetTest, updateTextRect)
{
    TextEdit *edit = new TextEdit(2, nullptr);
    QRectF *newRect = new QRectF(0, 0, 100, 100);
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    toolShape1.index = 0;
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    toolShape2.index = 1;
    Toolshape toolShape3;
    toolShape3.type = QString("text");
    toolShape3.index = 2;
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    toolShape3.points = points;
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    call_private_fun::ShapesWidgetupdateTextRect(*shapesWidget, edit, *newRect);

    delete edit;
    delete newRect;
}


TEST_F(ShapesWidgetTest, updateSelectedShape)
{
    shapesWidget->m_isSelectedText = true;
    QMap<int, TextEdit *> editMap;
    TextEdit *textEdit0 = new TextEdit(0, nullptr);
    TextEdit *textEdit1 = new TextEdit(1, nullptr);
    TextEdit *textEdit2 = new TextEdit(2, nullptr);
    editMap.insert(0, textEdit0);
    editMap.insert(1, textEdit1);
    editMap.insert(2, textEdit2);
    shapesWidget->m_editMap = editMap;
    QString group = QString("line");
    QString key = QString("linewidth_index");
    int index = 0;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 0;
    shapesWidget->updateSelectedShape(group, key, index);

    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 0;
//    QList <Toolshape> toolShapes;
//    toolShapes << Toolshape();
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    toolShape1.index = 0;
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    toolShape2.index = 1;
    Toolshape toolShape3;
    toolShape3.type = QString("arrow");
    toolShape3.index = 2;
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    toolShape3.points = points;
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = QString("arrow");
    key = QString("arrow_linewidth_index");
    shapesWidget->m_isSelectedText = false;
    shapesWidget->updateSelectedShape(group, key, index);

    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).isStraight = true;
    key = QString("straightline_linewidth_index");
    shapesWidget->updateSelectedShape(group, key, index);

    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = QString("line");
    key = QString("linewidth_index");
    shapesWidget->updateSelectedShape(group, key, index);

    group = QString("text");
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = group;
    key = QString("color_index");
    shapesWidget->updateSelectedShape(group, key, index);

    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    group = QString("text");
    key = QString("fontsize");
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = group;
    shapesWidget->updateSelectedShape(group, key, index);

    group = QString("line");
    key = QString("color_index");
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = group;
    shapesWidget->updateSelectedShape(group, key, index);

    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 0;
//    QList <Toolshape> toolShapes;
    toolShapes << Toolshape();
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = QString("arrow");
    key = QString("arrow_linewidth_index");
    shapesWidget->updateSelectedShape(group, key, index);

    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).isStraight = true;
    key = QString("straightline_linewidth_index");
    shapesWidget->updateSelectedShape(group, key, index);

    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = QString("line");
    key = QString("linewidth_index");
    shapesWidget->updateSelectedShape(group, key, index);

    group = QString("text");
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = group;
    key = QString("color_index");
    shapesWidget->updateSelectedShape(group, key, index);

    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    group = QString("text");
    key = QString("fontsize");
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = group;
    shapesWidget->updateSelectedShape(group, key, index);

    group = QString("line");
    key = QString("color_index");
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = group;
    shapesWidget->updateSelectedShape(group, key, index);

    group = QString("rectangle");
    key = QString("linewidth_index");
    shapesWidget->updateSelectedShape(group, key, index);
    group = QString("oval");
    key = QString("linewidth_index");
    shapesWidget->updateSelectedShape(group, key, index);
    group = QString("arrow");
    key = QString("arrow_linewidth_index");
    shapesWidget->updateSelectedShape(group, key, index);

    access_private_field::ShapesWidgetm_isSelectedText(*shapesWidget) = true;
    shapesWidget->updateSelectedShape(group, key, index);

    group = QString("common");
    key = QString("color_index");
    shapesWidget->updateSelectedShape(group, key, index);

    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = QString("arrow");

    delete textEdit0;
    delete textEdit1;
    delete textEdit2;
}

TEST_F(ShapesWidgetTest, setCurrentShape)
{
    QString shapeType = QString("arrow");
    shapesWidget->setCurrentShape(shapeType);
    shapeType = QString("shapeType");
    shapesWidget->setCurrentShape(shapeType);
}

TEST_F(ShapesWidgetTest, clearSelected)
{
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("line");
    shapesWidget->clearSelected();
}

TEST_F(ShapesWidgetTest, paintImgPoint)
{
    QPainter painter;
    QPointF *pos =  new QPointF();
    QPixmap *img = new QPixmap();
    call_private_fun::ShapesWidgetpaintImgPoint(*shapesWidget, painter, *pos, *img, true);
    call_private_fun::ShapesWidgetpaintImgPoint(*shapesWidget, painter, *pos, *img, false);
    delete pos;
    delete img;
}

TEST_F(ShapesWidgetTest, paintRect)
{
    QPainter painter;
    FourPoints mainPoints;
    mainPoints << QPointF(96, 235);
    mainPoints << QPointF(96, 335);
    mainPoints << QPointF(265, 235);
    mainPoints << QPointF(265, 335);
    int index = 0;
    ShapesWidget::ShapeBlurStatus status = ShapesWidget::Hovered;
    call_private_fun::ShapesWidgetpaintRect(*shapesWidget, painter, mainPoints, index, status, false, false);
    ShapesWidget::ShapeBlurStatus status1 = ShapesWidget::Drawing;
    call_private_fun::ShapesWidgetpaintRect(*shapesWidget, painter, mainPoints, index, status1, false, false);
    call_private_fun::ShapesWidgetpaintRect(*shapesWidget, painter, mainPoints, index, status, true, true);
}

TEST_F(ShapesWidgetTest, paintEllipse)
{
    QPainter painter;
    FourPoints ellipseFPoints;
    ellipseFPoints << QPointF(96, 235);
    ellipseFPoints << QPointF(96, 335);
    ellipseFPoints << QPointF(265, 235);
    ellipseFPoints << QPointF(265, 335);
    int index = 0;
    ShapesWidget::ShapeBlurStatus status = ShapesWidget::Hovered;
    call_private_fun::ShapesWidgetpaintEllipse(*shapesWidget, painter, ellipseFPoints, index, status, false, false);
    ShapesWidget::ShapeBlurStatus status1 = ShapesWidget::Drawing;
    call_private_fun::ShapesWidgetpaintEllipse(*shapesWidget, painter, ellipseFPoints, index, status1, true, true);

    FourPoints ellipseFPoints1;
    ellipseFPoints1 << QPointF(0, 10);
    ellipseFPoints1 << QPointF(0, 0);
    ellipseFPoints1 << QPointF(0, 0);
    ellipseFPoints1 << QPointF(0, 0);
    call_private_fun::ShapesWidgetpaintEllipse(*shapesWidget, painter, ellipseFPoints1, index, status1, true, true);
}

TEST_F(ShapesWidgetTest, paintArrow)
{
    QPainter painter;
    QList<QPointF> lineFPoints;
    lineFPoints << QPointF(96, 235);
    lineFPoints << QPointF(96, 335);
    int lineWidth = 10;
    call_private_fun::ShapesWidgetpaintArrow(*shapesWidget, painter, lineFPoints, lineWidth, true);
    call_private_fun::ShapesWidgetpaintArrow(*shapesWidget, painter, lineFPoints, lineWidth, false);
}

TEST_F(ShapesWidgetTest, paintLine)
{
    QPainter painter;
    QList<QPointF> lineFPoints;
    lineFPoints << QPointF(96, 235);
    lineFPoints << QPointF(96, 335);
    call_private_fun::ShapesWidgetpaintLine(*shapesWidget, painter, lineFPoints);

    QList<QPointF> lineFPoints1;
    call_private_fun::ShapesWidgetpaintLine(*shapesWidget, painter, lineFPoints1);

    QList<QPointF> lineFPoints2;
    lineFPoints2 << QPointF(96, 235);
    lineFPoints2 << QPointF(96, 335);
    lineFPoints2 << QPointF(265, 235);
    lineFPoints2 << QPointF(265, 335);
    call_private_fun::ShapesWidgetpaintLine(*shapesWidget, painter, lineFPoints2);
}

TEST_F(ShapesWidgetTest, paintText)
{
    QPainter painter;
    FourPoints rectFPoints;
    rectFPoints << QPointF(96, 235);
    rectFPoints << QPointF(96, 335);
    rectFPoints << QPointF(265, 235);
    rectFPoints << QPointF(265, 335);
    call_private_fun::ShapesWidgetpaintText(*shapesWidget, painter, rectFPoints);
}

TEST_F(ShapesWidgetTest, setAllTextEditReadOnly)
{
    QMap<int, TextEdit *> editMap;
    TextEdit *textEdit = new TextEdit(-1, nullptr);
    TextEdit *textEdit0 = new TextEdit(0, nullptr);
    TextEdit *textEdit1 = new TextEdit(1, nullptr);
    TextEdit *textEdit2 = new TextEdit(2, nullptr);
    editMap.insert(-1, textEdit);
    editMap.insert(0, textEdit0);
    editMap.insert(1, textEdit1);
    editMap.insert(2, textEdit2);
    shapesWidget->m_editMap = editMap;
    shapesWidget->setAllTextEditReadOnly();

    delete  textEdit;
    delete  textEdit0;
    delete  textEdit1;
    delete  textEdit2;
}

TEST_F(ShapesWidgetTest, setNoChangedTextEditRemove)
{
    QMap<int, TextEdit *> editMap;
    TextEdit *textEdit = new TextEdit(-1, nullptr);
    TextEdit *textEdit0 = new TextEdit(0, nullptr);
    TextEdit *textEdit1 = new TextEdit(1, nullptr);
    TextEdit *textEdit2 = new TextEdit(2, nullptr);
    editMap.insert(-1, textEdit);
    editMap.insert(0, textEdit0);
    editMap.insert(1, textEdit1);
    editMap.insert(2, textEdit2);
    shapesWidget->m_editMap = editMap;
    shapesWidget->setNoChangedTextEditRemove();

    delete  textEdit;
    delete  textEdit0;
    delete  textEdit1;
    delete  textEdit2;
}

TEST_F(ShapesWidgetTest, saveActionTriggered)
{
    QMap<int, TextEdit *> editMap;
    TextEdit *textEdit = new TextEdit(-1, nullptr);
    TextEdit *textEdit0 = new TextEdit(0, nullptr);
    TextEdit *textEdit1 = new TextEdit(1, nullptr);
    TextEdit *textEdit2 = new TextEdit(2, nullptr);
    editMap.insert(-1, textEdit);
    editMap.insert(0, textEdit0);
    editMap.insert(1, textEdit1);
    editMap.insert(2, textEdit2);
    shapesWidget->m_editMap = editMap;
    shapesWidget->saveActionTriggered();

    delete  textEdit;
    delete  textEdit0;
    delete  textEdit1;
    delete  textEdit2;
}

TEST_F(ShapesWidgetTest, handleDrag)
{
    QPointF oldPoint1(123, 95);
    QPointF newPoint1(121, 93);
    shapesWidget->handleDrag(oldPoint1, newPoint1);

    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 2;
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    Toolshape toolShape3;
    toolShape3.type = QString("arrow");
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    toolShape3.points = points;
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    QPointF oldPoint11(123, 95);
    QPointF newPoint11(121, 93);
    shapesWidget->handleDrag(oldPoint11, newPoint11);

    //
    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 0;
    QList <Toolshape> shapes;
    Toolshape toolShape6;
    FourPoints mainPoints1;
    mainPoints1 << QPointF(96, 235);
    mainPoints1 << QPointF(96, 335);
    mainPoints1 << QPointF(265, 235);
    mainPoints1 << QPointF(265, 335);
    toolShape6.mainPoints = mainPoints1;
    QPointF oldPoint2(213, 231);
    QPointF newPoint2(213, 231);
    QList<QPointF> points1;
    points1 << QPointF(96, 235);
    points1 << QPointF(96, 335);
    points1 << QPointF(265, 235);
    points1 << QPointF(265, 335);
    toolShape6.points = points1;
    shapes << toolShape6;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = shapes;
    shapesWidget->handleDrag(oldPoint2, newPoint2);
}

TEST_F(ShapesWidgetTest, handleRotate)
{
    QPointF pos(500, 500);
    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = -1;
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = QString("text");
    shapesWidget->handleRotate(pos);

    //    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    //    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = QString("arrow");
    //    access_private_field::ShapesWidgetm_isArrowRotated(*shapesWidget) = true;
    //    shapesWidget->handleRotate(pos);

    //    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    //    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = QString("oval");
    //    shapesWidget->handleRotate(pos);



}

TEST_F(ShapesWidgetTest, clickedOnShapes)
{
    QPointF pos(500, 500);
    //shapesWidget->clickedOnShapes(pos);

    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    Toolshape toolShape3;
    toolShape3.type = QString("arrow");
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;

    stub.set(ADDR(ShapesWidget, clickedOnRect), clickedOnRect_stub);
    shapesWidget->clickedOnShapes(pos);
    stub.reset(ADDR(ShapesWidget, clickedOnRect));

    stub.set(ADDR(ShapesWidget, clickedOnEllipse), clickedOnEllipse_stub);
    shapesWidget->clickedOnShapes(pos);
    stub.reset(ADDR(ShapesWidget, clickedOnEllipse));

    stub.set(ADDR(ShapesWidget, clickedOnArrow), clickedOnArrow_stub);
    shapesWidget->clickedOnShapes(pos);
    stub.reset(ADDR(ShapesWidget, clickedOnArrow));

    stub.set(ADDR(ShapesWidget, clickedOnLine), clickedOnLine_stub);
    shapesWidget->clickedOnShapes(pos);
    stub.reset(ADDR(ShapesWidget, clickedOnLine));

    stub.set(ADDR(ShapesWidget, clickedOnText), clickedOnText_stub);
    shapesWidget->clickedOnShapes(pos);
    stub.reset(ADDR(ShapesWidget, clickedOnText));
}

TEST_F(ShapesWidgetTest, clickedOnRect)
{
    FourPoints rectPoints0;
    rectPoints0 << QPointF(189, 199);
    rectPoints0 << QPointF(189, 396);
    rectPoints0 << QPointF(488, 199);
    rectPoints0 << QPointF(488, 396);
    QPointF pos0 = QPointF(484, 398);
    shapesWidget->clickedOnRect(rectPoints0, pos0, false);

    FourPoints rectPoints1;
    rectPoints1 << QPointF(446.545, 154.746);
    rectPoints1 << QPointF(249.965, 141.891);
    rectPoints1 << QPointF(427.035, 453.109);
    rectPoints1 << QPointF(230.455, 440.254);
    QPointF pos1 = QPointF(372, 338);
    shapesWidget->clickedOnRect(rectPoints1, pos1, false);

    FourPoints rectPoints2;
    rectPoints2 << QPointF(446.545, 154.746);
    rectPoints2 << QPointF(249.965, 141.891);
    rectPoints2 << QPointF(427.035, 453.109);
    rectPoints2 << QPointF(230.455, 440.254);
    QPointF pos2 = QPointF(236, 291);
    shapesWidget->clickedOnRect(rectPoints2, pos2, false);

    FourPoints rectPoints3;
    rectPoints3 << QPointF(446.545, 154.746);
    rectPoints3 << QPointF(249.965, 141.891);
    rectPoints3 << QPointF(427.035, 453.109);
    rectPoints3 << QPointF(230.455, 440.254);
    QPointF pos3 = QPointF(425, 455);
    shapesWidget->clickedOnRect(rectPoints3, pos3, false);

    FourPoints rectPoints4;
    rectPoints4 << QPointF(421.027, 153.077);
    rectPoints4 << QPointF(249.965, 141.891);
    rectPoints4 << QPointF(409, 337);
    rectPoints4 << QPointF(237.938, 325.814);
    QPointF pos4 = QPointF(413, 250);
    shapesWidget->clickedOnRect(rectPoints4, pos4, false);

    FourPoints rectPoints5;
    rectPoints5 << QPointF(421.027, 153.077);
    rectPoints5 << QPointF(249.965, 141.891);
    rectPoints5 << QPointF(409, 337);
    rectPoints5 << QPointF(237.938, 325.814);
    QPointF pos5 = QPointF(436, 249);
    shapesWidget->clickedOnRect(rectPoints5, pos5, false);

    FourPoints rectPoints6;
    rectPoints6 << QPointF(189, 199);
    rectPoints6 << QPointF(189, 396);
    rectPoints6 << QPointF(488, 199);
    rectPoints6 << QPointF(488, 396);
    QPointF pos6 = QPointF(484, 398);
    shapesWidget->clickedOnRect(rectPoints6, pos6, false);

    FourPoints rectPoints7;
    rectPoints7 << QPointF(296.346, 360.862);
    rectPoints7 << QPointF(448.182, 281.281);
    rectPoints7 << QPointF(210.783, 197.61);
    rectPoints7 << QPointF(362.619, 118.03);
    QPointF pos7 = QPointF(524, 239);
    shapesWidget->clickedOnRect(rectPoints7, pos7, false);

    FourPoints rectPoints8;
    rectPoints8 << QPointF(216, 105);
    rectPoints8 << QPointF(216, 192);
    rectPoints8 << QPointF(334, 105);
    rectPoints8 << QPointF(334, 192);
    QPointF pos8 = QPointF(213, 106);
    shapesWidget->clickedOnRect(rectPoints8, pos8, false);

    FourPoints rectPoints9;
    rectPoints9 << QPointF(80, 131);
    rectPoints9 << QPointF(80, 288);
    rectPoints9 << QPointF(351, 131);
    rectPoints9 << QPointF(351, 288);
    QPointF pos9 = QPointF(219, 131);
    shapesWidget->clickedOnRect(rectPoints9, pos9, false);

    FourPoints rectPoints10;
    rectPoints10 << QPointF(252, 42);
    rectPoints10 << QPointF(252, 192);
    rectPoints10 << QPointF(334, 42);
    rectPoints10 << QPointF(334, 192);
    QPointF pos10 = QPointF(331, 116);
    shapesWidget->clickedOnRect(rectPoints10, pos10, false);

    FourPoints rectPoints11;
    rectPoints11 << QPointF(420, 85);
    rectPoints11 << QPointF(420, 330);
    rectPoints11 << QPointF(730, 85);
    rectPoints11 << QPointF(730, 330);
    QPointF pos11 = QPointF(651, 292);
    shapesWidget->clickedOnRect(rectPoints11, pos11, true);

    FourPoints rectPoints12;
    rectPoints12 << QPointF(182.329, 146.634);
    rectPoints12 << QPointF(126.683, 177.773);
    rectPoints12 << QPointF(201, 180);
    rectPoints12 << QPointF(145.353, 211.139);
    QPointF pos12 = QPointF(151, 166);
    shapesWidget->clickedOnRect(rectPoints12, pos12, false);

    FourPoints rectPoints13;
    rectPoints13 << QPointF(89, 86);
    rectPoints13 << QPointF(89, 321);
    rectPoints13 << QPointF(355, 86);
    rectPoints13 << QPointF(355, 321);
    QPointF pos13 = QPointF(90, 322);
    shapesWidget->clickedOnRect(rectPoints13, pos13, false);
}

TEST_F(ShapesWidgetTest, hoverOnText)
{
    FourPoints rectPoints;
    rectPoints << QPointF(96, 235);
    rectPoints << QPointF(96, 335);
    rectPoints << QPointF(265, 235);
    rectPoints << QPointF(265, 335);
    QPointF pos = QPointF(220, 233);
    EXPECT_TRUE(shapesWidget->hoverOnText(rectPoints, pos));
}

TEST_F(ShapesWidgetTest, hoverOnRotatePoint)
{
    FourPoints rectPoints;
    rectPoints << QPointF(96, 235);
    rectPoints << QPointF(96, 335);
    rectPoints << QPointF(265, 235);
    rectPoints << QPointF(265, 335);
    QPointF pos = QPointF(220, 233);
    EXPECT_FALSE(shapesWidget->hoverOnRotatePoint(rectPoints, pos));
}

TEST_F(ShapesWidgetTest, clickedOnEllipse)
{
    FourPoints mainPoints;
    mainPoints << QPointF(96, 235);
    mainPoints << QPointF(96, 335);
    mainPoints << QPointF(265, 235);
    mainPoints << QPointF(265, 335);
    QPointF pos = QPointF(220, 233);
    bool isBlurMosaic = false;
    EXPECT_TRUE(shapesWidget->clickedOnEllipse(mainPoints, pos, isBlurMosaic));
    pos = QPointF(500, 500);
    EXPECT_FALSE(shapesWidget->clickedOnEllipse(mainPoints, pos, isBlurMosaic));

    FourPoints rectPoints0;
    rectPoints0 << QPointF(189, 199);
    rectPoints0 << QPointF(189, 396);
    rectPoints0 << QPointF(488, 199);
    rectPoints0 << QPointF(488, 396);
    QPointF pos0 = QPointF(484, 398);
    shapesWidget->clickedOnEllipse(rectPoints0, pos0, false);

    FourPoints rectPoints1;
    rectPoints1 << QPointF(446.545, 154.746);
    rectPoints1 << QPointF(249.965, 141.891);
    rectPoints1 << QPointF(427.035, 453.109);
    rectPoints1 << QPointF(230.455, 440.254);
    QPointF pos1 = QPointF(372, 338);
    shapesWidget->clickedOnEllipse(rectPoints1, pos1, false);

    FourPoints rectPoints2;
    rectPoints2 << QPointF(446.545, 154.746);
    rectPoints2 << QPointF(249.965, 141.891);
    rectPoints2 << QPointF(427.035, 453.109);
    rectPoints2 << QPointF(230.455, 440.254);
    QPointF pos2 = QPointF(236, 291);
    shapesWidget->clickedOnEllipse(rectPoints2, pos2, false);

    FourPoints rectPoints3;
    rectPoints3 << QPointF(446.545, 154.746);
    rectPoints3 << QPointF(249.965, 141.891);
    rectPoints3 << QPointF(427.035, 453.109);
    rectPoints3 << QPointF(230.455, 440.254);
    QPointF pos3 = QPointF(425, 455);
    shapesWidget->clickedOnEllipse(rectPoints3, pos3, false);

    FourPoints rectPoints4;
    rectPoints4 << QPointF(421.027, 153.077);
    rectPoints4 << QPointF(249.965, 141.891);
    rectPoints4 << QPointF(409, 337);
    rectPoints4 << QPointF(237.938, 325.814);
    QPointF pos4 = QPointF(413, 250);
    shapesWidget->clickedOnEllipse(rectPoints4, pos4, false);

    FourPoints rectPoints5;
    rectPoints5 << QPointF(421.027, 153.077);
    rectPoints5 << QPointF(249.965, 141.891);
    rectPoints5 << QPointF(409, 337);
    rectPoints5 << QPointF(237.938, 325.814);
    QPointF pos5 = QPointF(436, 249);
    shapesWidget->clickedOnEllipse(rectPoints5, pos5, false);

    FourPoints rectPoints6;
    rectPoints6 << QPointF(189, 199);
    rectPoints6 << QPointF(189, 396);
    rectPoints6 << QPointF(488, 199);
    rectPoints6 << QPointF(488, 396);
    QPointF pos6 = QPointF(484, 398);
    shapesWidget->clickedOnEllipse(rectPoints6, pos6, false);

    FourPoints rectPoints7;
    rectPoints7 << QPointF(296.346, 360.862);
    rectPoints7 << QPointF(448.182, 281.281);
    rectPoints7 << QPointF(210.783, 197.61);
    rectPoints7 << QPointF(362.619, 118.03);
    QPointF pos7 = QPointF(524, 239);
    shapesWidget->clickedOnEllipse(rectPoints7, pos7, false);

    FourPoints rectPoints8;
    rectPoints8 << QPointF(216, 105);
    rectPoints8 << QPointF(216, 192);
    rectPoints8 << QPointF(334, 105);
    rectPoints8 << QPointF(334, 192);
    QPointF pos8 = QPointF(213, 106);
    shapesWidget->clickedOnEllipse(rectPoints8, pos8, false);

    FourPoints rectPoints9;
    rectPoints9 << QPointF(80, 131);
    rectPoints9 << QPointF(80, 288);
    rectPoints9 << QPointF(351, 131);
    rectPoints9 << QPointF(351, 288);
    QPointF pos9 = QPointF(219, 131);
    shapesWidget->clickedOnEllipse(rectPoints9, pos9, false);

    FourPoints rectPoints10;
    rectPoints10 << QPointF(252, 42);
    rectPoints10 << QPointF(252, 192);
    rectPoints10 << QPointF(334, 42);
    rectPoints10 << QPointF(334, 192);
    QPointF pos10 = QPointF(331, 116);
    shapesWidget->clickedOnEllipse(rectPoints10, pos10, false);

    FourPoints rectPoints11;
    rectPoints11 << QPointF(420, 85);
    rectPoints11 << QPointF(420, 330);
    rectPoints11 << QPointF(730, 85);
    rectPoints11 << QPointF(730, 330);
    QPointF pos11 = QPointF(651, 292);
    shapesWidget->clickedOnEllipse(rectPoints11, pos11, true);

    FourPoints rectPoints12;
    rectPoints12 << QPointF(182.329, 146.634);
    rectPoints12 << QPointF(126.683, 177.773);
    rectPoints12 << QPointF(201, 180);
    rectPoints12 << QPointF(145.353, 211.139);
    QPointF pos12 = QPointF(151, 166);
    shapesWidget->clickedOnEllipse(rectPoints12, pos12, false);

    FourPoints rectPoints13;
    rectPoints13 << QPointF(89, 86);
    rectPoints13 << QPointF(89, 321);
    rectPoints13 << QPointF(355, 86);
    rectPoints13 << QPointF(355, 321);
    QPointF pos13 = QPointF(90, 322);
    shapesWidget->clickedOnEllipse(rectPoints13, pos13, false);
}

TEST_F(ShapesWidgetTest, clickedOnArrow)
{
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    QPointF pos = QPointF(220, 233);
    EXPECT_FALSE(shapesWidget->clickedOnArrow(points, pos));

    QList<QPointF> points0;
    points0 << QPointF(567, 138);
    points0 << QPointF(427, 289);
    QPointF pos0 = QPointF(565, 138);
    shapesWidget->clickedOnArrow(points0, pos0);

    QList<QPointF> points1;
    points1 << QPointF(554, 240);
    points1 << QPointF(432, 397);
    QPointF pos1 = QPointF(430, 399);
    shapesWidget->clickedOnArrow(points1, pos1);

    QList<QPointF> points2;
    points2 << QPointF(567, 138);
    points2 << QPointF(427, 289);
    QPointF pos2 = QPointF(524, 187);
    shapesWidget->clickedOnArrow(points2, pos2);

    QList<QPointF> points3;
    points3 << QPointF(554, 240);
    points3 << QPointF(572, 370);
    QPointF pos3 = QPointF(565, 138);
    shapesWidget->clickedOnArrow(points3, pos3);

}

TEST_F(ShapesWidgetTest, clickedOnLine)
{
    FourPoints mainPoints;
    mainPoints << QPointF(96, 235);
    mainPoints << QPointF(96, 335);
    mainPoints << QPointF(265, 235);
    mainPoints << QPointF(265, 335);
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    QPointF pos = QPointF(220, 233);
    EXPECT_FALSE(shapesWidget->clickedOnLine(mainPoints, points, pos));
    EXPECT_FALSE(shapesWidget->clickedOnLine(mainPoints, points, QPoint(0, 0)));
    EXPECT_FALSE(shapesWidget->clickedOnLine(mainPoints, points, QPoint(100, 200)));

    FourPoints mainPoints1;
    mainPoints1 << QPointF(92, 307);
    mainPoints1 << QPointF(92, 329);
    mainPoints1 << QPointF(172, 307);
    mainPoints1 << QPointF(172, 329);
    QList<QPointF> points1;
    points1 << QPointF(102, 319);
    points1 << QPointF(109, 319);
    points1 << QPointF(114, 319);
    points1 << QPointF(119, 319);
    points1 << QPointF(124, 319);
    points1 << QPointF(129, 319);
    points1 << QPointF(133, 319);
    points1 << QPointF(137, 319);
    points1 << QPointF(143, 319);
    points1 << QPointF(147, 319);
    points1 << QPointF(162, 317);
    QPointF pos1 = QPointF(175, 331);
    shapesWidget->clickedOnLine(mainPoints1, points1, pos1);
    shapesWidget->clickedOnLine(mainPoints1, points1, QPoint(0, 0));
    shapesWidget->clickedOnLine(mainPoints1, points1, QPoint(10, 10));

    FourPoints mainPoints2;
    mainPoints2 << QPointF(92, 307);
    mainPoints2 << QPointF(92, 362);
    mainPoints2 << QPointF(163, 307);
    mainPoints2 << QPointF(163, 362);
    QList<QPointF> points2;
    points2 << QPointF(100.875, 337);
    points2 << QPointF(107.088, 337);
    points2 << QPointF(111.525, 337);
    points2 << QPointF(115.963, 337);
    points2 << QPointF(120.4, 337);
    points2 << QPointF(124.838, 337);
    points2 << QPointF(128.388, 337);
    points2 << QPointF(131.938, 337);
    points2 << QPointF(137.262, 337);
    points2 << QPointF(140.813, 337);
    points2 << QPointF(146.137, 334.5);
    points2 << QPointF(150.575, 334.5);
    points2 << QPointF(154.125, 332);
    QPointF pos2 = QPointF(126, 283);
    shapesWidget->clickedOnLine(mainPoints2, points2, pos2);

    FourPoints mainPoints3;
    mainPoints3 << QPointF(92, 307);
    mainPoints3 << QPointF(92, 329);
    mainPoints3 << QPointF(172, 307);
    mainPoints3 << QPointF(172, 329);
    QList<QPointF> points3;
    points3 << QPointF(102, 319);
    points3 << QPointF(109, 319);
    points3 << QPointF(114, 319);
    points3 << QPointF(119, 319);
    points3 << QPointF(124, 319);
    points3 << QPointF(129, 319);
    points3 << QPointF(133, 319);
    points3 << QPointF(137, 319);
    points3 << QPointF(143, 319);
    points3 << QPointF(147, 319);
    points3 << QPointF(153, 318);
    points3 << QPointF(158, 318);
    points3 << QPointF(162, 317);
    QPointF pos3 = QPointF(147, 321);
    shapesWidget->clickedOnLine(mainPoints3, points3, pos3);

    FourPoints mainPoints4;
    mainPoints4 << QPointF(92, 307);
    mainPoints4 << QPointF(92, 329);
    mainPoints4 << QPointF(172, 329);
    mainPoints4 << QPointF(172, 307);
    QList<QPointF> points4;
    points4 << QPointF(102, 319);
    points4 << QPointF(109, 319);
    points4 << QPointF(114, 319);
    points4 << QPointF(119, 319);
    points4 << QPointF(124, 319);
    points4 << QPointF(129, 319);
    points4 << QPointF(133, 319);
    points4 << QPointF(137, 319);
    points4 << QPointF(143, 319);
    points4 << QPointF(147, 319);
    points4 << QPointF(153, 318);
    points4 << QPointF(158, 318);
    points4 << QPointF(162, 317);
    QPointF pos4 = QPointF(175, 331);
    shapesWidget->clickedOnLine(mainPoints4, points4, pos4);

    FourPoints mainPoints5;
    mainPoints5 << QPointF(92, 307);
    mainPoints5 << QPointF(172, 329);
    mainPoints5 << QPointF(92, 329);
    mainPoints5 << QPointF(172, 307);
    QList<QPointF> points5;
    points5 << QPointF(102, 319);
    points5 << QPointF(109, 319);
    points5 << QPointF(114, 319);
    points5 << QPointF(119, 319);
    points5 << QPointF(124, 319);
    points5 << QPointF(129, 319);
    points5 << QPointF(133, 319);
    points5 << QPointF(137, 319);
    points5 << QPointF(143, 319);
    points5 << QPointF(147, 319);
    points5 << QPointF(153, 318);
    points5 << QPointF(158, 318);
    points5 << QPointF(162, 317);
    QPointF pos5 = QPointF(175, 331);
    shapesWidget->clickedOnLine(mainPoints5, points5, pos5);

    FourPoints mainPoints6;
    mainPoints6 << QPointF(172, 329);
    mainPoints6 << QPointF(92, 307);
    mainPoints6 << QPointF(92, 329);
    mainPoints6 << QPointF(172, 307);
    QList<QPointF> points6;
    points6 << QPointF(102, 319);
    points6 << QPointF(109, 319);
    points6 << QPointF(114, 319);
    points6 << QPointF(119, 319);
    points6 << QPointF(124, 319);
    points6 << QPointF(129, 319);
    points6 << QPointF(133, 319);
    points6 << QPointF(137, 319);
    points6 << QPointF(143, 319);
    points6 << QPointF(147, 319);
    points6 << QPointF(153, 318);
    points6 << QPointF(158, 318);
    points6 << QPointF(162, 317);
    QPointF pos6 = QPointF(175, 331);
    shapesWidget->clickedOnLine(mainPoints6, points6, pos6);
}

TEST_F(ShapesWidgetTest, hoverOnLine)
{
    FourPoints mainPoints;
    mainPoints << QPointF(96, 235);
    mainPoints << QPointF(96, 335);
    mainPoints << QPointF(265, 235);
    mainPoints << QPointF(265, 335);
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    QPointF pos = QPointF(220, 233);
    EXPECT_FALSE(shapesWidget->hoverOnLine(mainPoints, points, pos));
}

TEST_F(ShapesWidgetTest, hoverOnArrow)
{
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    QPointF pos = QPointF(220, 233);
    EXPECT_FALSE(shapesWidget->hoverOnArrow(points, pos));
}

TEST_F(ShapesWidgetTest, clickedOnText)
{
    FourPoints mainPoints;
    mainPoints << QPointF(96, 235);
    mainPoints << QPointF(96, 335);
    mainPoints << QPointF(265, 235);
    mainPoints << QPointF(265, 335);
    QPointF pos = QPointF(220, 233);
    EXPECT_FALSE(shapesWidget->clickedOnText(mainPoints, pos));
}

TEST_F(ShapesWidgetTest, rotateOnPoint)
{
    FourPoints mainPoints;
    mainPoints << QPointF(96, 235);
    mainPoints << QPointF(96, 335);
    mainPoints << QPointF(265, 235);
    mainPoints << QPointF(265, 335);
    QPointF pos = QPointF(220, 233);
    EXPECT_FALSE(shapesWidget->clickedOnText(mainPoints, pos));
}

TEST_F(ShapesWidgetTest, hoverOnRect)
{
    FourPoints rectPoints;
    rectPoints << QPointF(96, 235);
    rectPoints << QPointF(96, 335);
    rectPoints << QPointF(265, 235);
    rectPoints << QPointF(265, 335);
    QPointF pos = QPointF(220, 233);
    bool isTextBorder = false;
    EXPECT_TRUE(shapesWidget->hoverOnRect(rectPoints, pos, isTextBorder));
}

TEST_F(ShapesWidgetTest, hoverOnEllipse)
{
    FourPoints rectPoints;
    rectPoints << QPointF(96, 235);
    rectPoints << QPointF(96, 335);
    rectPoints << QPointF(265, 235);
    rectPoints << QPointF(265, 335);
    QPointF pos = QPointF(220, 233);
    EXPECT_TRUE(shapesWidget->hoverOnEllipse(rectPoints, pos));
}

TEST_F(ShapesWidgetTest, hoverOnShapes)
{
    Toolshape toolshape;
    EXPECT_FALSE(shapesWidget->hoverOnShapes(toolshape, QPointF(96, 235)));
}

TEST_F(ShapesWidgetTest, textEditIsReadOnly)
{
    shapesWidget->textEditIsReadOnly();
}

TEST_F(ShapesWidgetTest, undoDrawShapes)
{
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    toolShape1.index = 0;
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    toolShape2.index = 1;
    Toolshape toolShape3;
    toolShape3.type = QString("text");
    toolShape3.index = 2;
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    toolShape3.points = points;
    Toolshape toolShape4;
    toolShape4.type = QString("text");
    Toolshape toolShape5;
    toolShape5.type = QString("line");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    shapesWidget->undoDrawShapes();

    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    shapesWidget->undoDrawShapes();
    /*
        QMap<int, TextEdit *> editMap;
        TextEdit *textEdit = new TextEdit(-1,nullptr);
        TextEdit *textEdit0 = new TextEdit(0,nullptr);
        TextEdit *textEdit1 = new TextEdit(1,nullptr);
        TextEdit *textEdit2 = new TextEdit(2,nullptr);
        editMap.insert(-1,textEdit);
        editMap.insert(0,textEdit0);
        editMap.insert(1,textEdit1);
        editMap.insert(2,textEdit2);
        shapesWidget->m_editMap = editMap;
        shapesWidget->undoDrawShapes();

        access_private_field::ShapesWidgetm_shapes(*shapesWidget).clear();
        shapesWidget->undoDrawShapes();
    */
}

TEST_F(ShapesWidgetTest, undoAllDrawShapes)
{
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    toolShape1.index = 0;
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    toolShape2.index = 1;
    Toolshape toolShape3;
    toolShape3.type = QString("text");
    toolShape3.index = 2;
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    toolShape3.points = points;
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    toolShape4.points = points;
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    shapesWidget->undoAllDrawShapes();
    /*
        access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = -1;
        QMap<int, TextEdit *> editMap;
        TextEdit *textEdit = new TextEdit(-1,nullptr);
        TextEdit *textEdit0 = new TextEdit(0,nullptr);
        TextEdit *textEdit1 = new TextEdit(1,nullptr);
        TextEdit *textEdit2 = new TextEdit(2,nullptr);
        editMap.insert(-1,textEdit);
        editMap.insert(0,textEdit0);
        editMap.insert(1,textEdit1);
        editMap.insert(2,textEdit2);
        shapesWidget->m_editMap = editMap;
        shapesWidget->undoAllDrawShapes();
    */
}

TEST_F(ShapesWidgetTest, deleteCurrentShape)
{
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("text");
    toolShape1.index = 0;
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    toolShape2.index = 1;
    Toolshape toolShape3;
    toolShape3.type = QString("rectangle");
    toolShape3.index = 2;
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    toolShape3.points = points;
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    toolShape4.points = points;
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    QMap<int, TextEdit *> editMap;
    TextEdit *textEdit0 = new TextEdit(0, nullptr);
    TextEdit *textEdit1 = new TextEdit(1, nullptr);
    TextEdit *textEdit2 = new TextEdit(2, nullptr);
    editMap.insert(0, textEdit0);
    editMap.insert(1, textEdit1);
    editMap.insert(2, textEdit2);
    shapesWidget->m_editMap = editMap;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).index = 0;
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).type = QString("text");
    shapesWidget->deleteCurrentShape();
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 100;
    shapesWidget->deleteCurrentShape();
    access_private_field::ShapesWidgetm_shapes(*shapesWidget).clear();
    shapesWidget->deleteCurrentShape();

    delete  textEdit0;
    delete  textEdit1;
    delete  textEdit2;
}

TEST_F(ShapesWidgetTest, updateCursorShape)
{
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("line");
    shapesWidget->updateCursorShape();
    stub.set(ADDR(QVariant, toBool), toBool_stub);
    access_private_field::ShapesWidgetm_currentType(*shapesWidget) = QString("arrow");
    shapesWidget->updateCursorShape();
    stub.reset(ADDR(QVariant, toBool));
}

TEST_F(ShapesWidgetTest, menuCloseSlot)
{
    shapesWidget->menuCloseSlot();
}

//(QTapGesture *tap)
TEST_F(ShapesWidgetTest, tapTriggered)
{
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    Toolshape toolShape3;
    toolShape3.type = QString("arrow");
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;

    QTapGesture *tap = new QTapGesture();
    call_private_fun::ShapesWidgettapTriggered(*shapesWidget, tap);
    delete tap;
}

TEST_F(ShapesWidgetTest, setGlobalRect)
{
    shapesWidget->setGlobalRect(QRect(896, 445, 782, 512));
}

TEST_F(ShapesWidgetTest, setShiftKeyPressed)
{
    shapesWidget->setShiftKeyPressed(false);
}

TEST_F(ShapesWidgetTest, microAdjust)
{
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    toolShape1.index = 0;
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    toolShape2.index = 1;
    Toolshape toolShape3;
    toolShape3.type = QString("text");
    toolShape3.index = 2;
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    toolShape3.points = points;
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    toolShape4.points = points;
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    access_private_field::ShapesWidgetm_selectedIndex(*shapesWidget) = 0;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 0;
    shapesWidget->microAdjust(QString("Left"));
    shapesWidget->microAdjust(QString("Ctrl+Shift+Left"));
    shapesWidget->microAdjust(QString(""));

    //stub.set(ADDR(QVariant,toBool),toBool_stub);
    //stub.set(ADDR(QList,length),length_stub);

    QList<QList<qreal>> portion;
    QList<qreal> list;
    portion << list;
    toolShape1.portion = portion;
    access_private_field::ShapesWidgetm_selectedOrder(*shapesWidget) = 3;
    shapesWidget->microAdjust(QString("Left"));

//    shapesWidget->microAdjust("Left");
//    shapesWidget->microAdjust("Left");
//    shapesWidget->microAdjust("Left");
//    shapesWidget->microAdjust("Left");
//    shapesWidget->microAdjust("Left");


}

TEST_F(ShapesWidgetTest, handleResize)
{
    shapesWidget->m_isResize = true;
    shapesWidget->m_selectedIndex = 0;
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    Toolshape toolShape3;
    toolShape3.type = QString("arrow");
    QList<QPointF> points;
    points << QPointF(96, 235);
    points << QPointF(96, 335);
    points << QPointF(265, 235);
    points << QPointF(265, 335);
    toolShape3.points = points;
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    shapesWidget->m_selectedOrder = 2;

    FourPoints mainPoints;
    mainPoints << QPointF(96, 235);
    mainPoints << QPointF(96, 335);
    mainPoints << QPointF(265, 235);
    mainPoints << QPointF(265, 335);
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).mainPoints = mainPoints;
    QList<QPointF> lineFPoints;
    lineFPoints << QPointF(96, 235);
    lineFPoints << QPointF(96, 335);
    lineFPoints << QPointF(96, 600);
    lineFPoints << QPointF(60, 500);
    access_private_field::ShapesWidgetm_selectedShape(*shapesWidget).points = lineFPoints;
    shapesWidget->handleResize(QPointF(503, 462), 1);
}

TEST_F(ShapesWidgetTest, menuSaveSlot)
{
    shapesWidget->menuSaveSlot();
}

TEST_F(ShapesWidgetTest, clickedShapes)
{
    QList <Toolshape> toolShapes;
    Toolshape toolShape1;
    toolShape1.type = QString("rectangle");
    Toolshape toolShape2;
    toolShape2.type = QString("oval");
    Toolshape toolShape3;
    toolShape3.type = QString("arrow");
    Toolshape toolShape4;
    toolShape4.type = QString("line");
    Toolshape toolShape5;
    toolShape5.type = QString("text");
    toolShapes << toolShape1;
    toolShapes << toolShape2;
    toolShapes << toolShape3;
    toolShapes << toolShape4;
    toolShapes << toolShape5;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes;
    call_private_fun::ShapesWidgetclickedShapes(*shapesWidget, QPointF(10, 10));


    QList <Toolshape> toolShapes1;
    Toolshape toolShape11;
    toolShape11.type = QString("arrow");
    toolShapes1 << toolShape11;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes1;
    stub.set(ADDR(ShapesWidget, clickedOnRect), clickedOnRect_stub);
    call_private_fun::ShapesWidgetclickedShapes(*shapesWidget, QPointF(10, 10));
    stub.reset(ADDR(ShapesWidget, clickedOnRect));

    QList <Toolshape> toolShapes2;
    Toolshape toolShape22;
    toolShape22.type = QString("line");
    toolShapes2 << toolShape22;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes2;
    stub.set(ADDR(ShapesWidget, clickedOnEllipse), clickedOnEllipse_stub);
    call_private_fun::ShapesWidgetclickedShapes(*shapesWidget, QPointF(10, 10));
    stub.reset(ADDR(ShapesWidget, clickedOnEllipse));

    QList <Toolshape> toolShapes3;
    Toolshape toolShape33;
    toolShape33.type = QString("text");
    toolShapes3 << toolShape33;
    access_private_field::ShapesWidgetm_shapes(*shapesWidget) = toolShapes3;
    stub.set(ADDR(ShapesWidget, clickedOnArrow), clickedOnArrow_stub);
    call_private_fun::ShapesWidgetclickedShapes(*shapesWidget, QPointF(10, 10));
    stub.reset(ADDR(ShapesWidget, clickedOnArrow));

    stub.set(ADDR(ShapesWidget, clickedOnLine), clickedOnLine_stub);
    call_private_fun::ShapesWidgetclickedShapes(*shapesWidget, QPointF(10, 10));
    stub.reset(ADDR(ShapesWidget, clickedOnLine));

    stub.set(ADDR(ShapesWidget, clickedOnText), clickedOnText_stub);
    call_private_fun::ShapesWidgetclickedShapes(*shapesWidget, QPointF(10, 10));
    stub.reset(ADDR(ShapesWidget, clickedOnText));
}



