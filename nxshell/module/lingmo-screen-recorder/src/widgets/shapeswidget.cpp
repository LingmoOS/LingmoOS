// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../utils/calculaterect.h"
#include "../utils/configsettings.h"
#include "../utils/tempfile.h"
#include "shapeswidget.h"
#include "../utils.h"
#ifdef KF5_WAYLAND_FLAGE_ON
#include "../utils/waylandmousesimulator.h"
#endif

#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QGestureEvent>

#include <cmath>

#define LINEWIDTH(index) (index*2+3)

const int DRAG_BOUND_RADIUS = 8;
const int SPACING = 12;
const int RESIZEPOINT_WIDTH = 15;
const QSize ROTATE_ICON_SIZE = QSize(30, 30);

ShapesWidget::ShapesWidget(DWidget *parent)
    : DFrame(parent),
      m_lastAngle(0),
      m_isMoving(false),
      m_isSelected(false),
      m_isShiftPressed(false),
      m_editing(false),
      m_shapesIndex(-1),
      m_selectedIndex(-1),
      m_selectedOrder(-1),
      m_menuController(new MenuController)
{
    //订阅手势事件
    QList<Qt::GestureType> gestures;
    //    gestures << Qt::PanGesture;
    gestures << Qt::PinchGesture;
    //    gestures << Qt::SwipeGesture;
    gestures << Qt::TapGesture;
    //    gestures << Qt::TapGesture;
    //    gestures << Qt::TapAndHoldGesture;
    //    gestures << Qt::CustomGesture;
    //    gestures << Qt::LastGestureType;
    foreach (Qt::GestureType gesture, gestures)
        grabGesture(gesture);


    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAcceptDrops(true);

    m_penColor = BaseUtils::colorIndexOf(ConfigSettings::instance()->getValue("pen", "color_index").toInt());

    connect(m_menuController, &MenuController::shapePressed,
            this, &ShapesWidget::shapePressed);
    //    connect(m_menuController, &MenuController::saveBtnPressed,
    //            this, &ShapesWidget::saveBtnPressed);
    connect(m_menuController, &MenuController::saveAction,
            this, &ShapesWidget::menuSaveSlot);
    connect(m_menuController, &MenuController::closeAction,
            this, &ShapesWidget::menuCloseSlot);
    connect(m_menuController, &MenuController::unDoAction,
            this, &ShapesWidget::undoDrawShapes);
    connect(m_menuController, &MenuController::menuNoFocus,
            this, &ShapesWidget::menuNoFocus);
    connect(this, &ShapesWidget::setShapesUndo,
            m_menuController, &MenuController::setUndoEnable);
    connect(ConfigSettings::instance(), &ConfigSettings::shapeConfigChanged, this, &ShapesWidget::updateSelectedShape);
    //m_sideBar = new SideBar(this);
    //m_sideBar->hide();
    m_currentCursor = QCursor().pos();
}

ShapesWidget::~ShapesWidget()
{
    if (m_menuController) {
        delete m_menuController;
        m_menuController = nullptr;
    }
}
//更新选中的形状
void ShapesWidget::updateSelectedShape(const QString &group,
                                       const QString &key, int index)
{
//        qDebug() << ">>>>> function: " << __func__ << ", line: " << __LINE__
//                 << ", group: " << group
//                 << ", key: " << key
//                 << ", index: " << index
//                 << ", m_selectedIndex" << m_selectedIndex;
    if (m_isSelectedText) { //修复在截图区域，选中的文本框，字体会自动变颜色
        m_isSelectedText = false;
        return;
    }
    if (group == m_currentShape.type && key == "color_index") {
        m_penColor = BaseUtils::colorIndexOf(index);
    }

    if (m_selectedIndex != -1 && m_selectedOrder != -1 && m_selectedOrder < m_shapes.length()) {
        if ((m_selectedShape.type == "arrow" || m_selectedShape.type == "line") && key != "color_index") {
            m_selectedShape.lineWidth = LINEWIDTH(index);
        } else if (m_selectedShape.type == group && key == "line_width") {
            m_selectedShape.lineWidth = LINEWIDTH(index);
        } else if (group == "text" && m_selectedShape.type == group && key == "color_index") {
            int tmpIndex = m_shapes[m_selectedOrder].index;
            if (m_editMap.contains(tmpIndex)) {
                m_editMap.value(tmpIndex)->setColor(BaseUtils::colorIndexOf(index));
                m_editMap.value(tmpIndex)->update();
            }

        } else if (group == "text" && m_selectedShape.type == group && key == "fontsize")  {
            qDebug() << "change font size";
            int tmpIndex = m_shapes[m_selectedOrder].index;
            if (m_editMap.contains(tmpIndex)) {
                m_editMap.value(tmpIndex)->setFontSize(index);
                m_editMap.value(tmpIndex)->update();
            }
        } else if (group != "text" && m_selectedShape.type == group && key == "color_index") {
            m_selectedShape.colorIndex = index;
        } else if (group == "effect" && m_selectedShape.type == group &&
                   key == "radius" && (m_selectedShape.isOval == 0 || m_selectedShape.isOval == 1)) {

            m_selectedShape.radius = index * 3 + 10;
            if (m_selectedShape.isBlur) {
                emit reloadEffectImg("blur", m_selectedShape.radius);
            } else {
                emit reloadEffectImg("mosaic", m_selectedShape.radius);
            }
        }

        if (m_selectedOrder < m_shapes.length()) {
            m_shapes[m_selectedOrder] = m_selectedShape;
        }
        update();
    }
    //qDebug() << ">>>>> function: " << __func__ << ", line: " << __LINE__ << ", m_selectedShape.type: " << m_selectedShape.type;
}
/*
 * never used
void ShapesWidget::updatePenColor()
{
    setPenColor(colorIndexOf(ConfigSettings::instance()->value(
                                 "common", "color_index").toInt()));
}
*/
void ShapesWidget::setCurrentShape(QString shapeType)
{
    qDebug() << __FUNCTION__ << __LINE__ << "type: " << shapeType;
    m_currentType = shapeType;
    if (shapeType != "text")
        setAllTextEditReadOnly();
}

void ShapesWidget::clearSelected()
{
    for (int j = 0; j < m_selectedShape.mainPoints.length(); j++) {
        m_selectedShape.mainPoints[j] = QPointF(0, 0);
        m_hoveredShape.mainPoints[j] = QPointF(0, 0);
    }

    //qDebug() << "clear selected!!!";
    m_isSelected = false;
    m_selectedShape.points.clear();
    m_hoveredShape.points.clear();
    updateCursorShape();
}

void ShapesWidget::setAllTextEditReadOnly()
{
    QMap<int, TextEdit *>::iterator i = m_editMap.begin();
    while (i != m_editMap.end()) {
        i.value()->setReadOnly(true);
        i.value()->releaseKeyboard();
        i.value()->setEditing(false);

        QTextCursor textCursor =  i.value()->textCursor();
        textCursor.clearSelection();
        i.value()->setTextCursor(textCursor);
        ++i;
    }

    setNoChangedTextEditRemove();
    update();
}

void ShapesWidget::setNoChangedTextEditRemove()
{
    if (m_shapes.length() == 0) {
        return;
    }
    for (int i = 0; i < m_shapes.length(); i++) {
        if (m_shapes[i].type == "text") {
            int t_tempIndex = m_shapes[i].index;
            if (m_editMap.value(t_tempIndex)->document()->toPlainText() == QString(tr("Input text here"))
                    || m_editMap.value(t_tempIndex)->document()->toPlainText().isEmpty()) {
                m_shapes.removeAt(i);
                m_editMap.value(t_tempIndex)->clear();
                m_editMap.remove(t_tempIndex);

                break;
            }

        }

    }
    if (m_shapes.length() == 0) {
        emit setShapesUndo(false);
    }

    update();
}

void ShapesWidget::saveActionTriggered()
{
    qInfo() << __FUNCTION__ << __LINE__ << "正在执行清除图形编辑界面...";
    setAllTextEditReadOnly();
    clearSelected();
    m_clearAllTextBorder = true;
    qInfo() << __FUNCTION__ << __LINE__ << "已清楚图形编辑界面";
}

//点击某个形状
bool ShapesWidget::clickedOnShapes(QPointF pos)
{
    bool onShapes = false;
    m_selectedOrder = -1;
    //    qDebug() << ">>>>> function: " << __func__ << ", line: " << __LINE__
    //             << ", pos: " << pos
    //             << ", m_shapes.length(): " << m_shapes.length();
    for (int i = 0; i < m_shapes.length(); i++) {
        //当前是否有形状被选中
        bool currentOnShape = false;
        if (m_shapes[i].type == "rectangle") {
            if (clickedOnRect(m_shapes[i].mainPoints, pos, false)) {
                currentOnShape = true;
                emit shapeClicked("rect");
            }
        }
        if (m_shapes[i].type == "oval") {
            if (clickedOnEllipse(m_shapes[i].mainPoints, pos, false)) {
                currentOnShape = true;
                emit shapeClicked("circ");
            }
        }
        if (m_shapes[i].type == "effect") {
            if (m_shapes[i].isOval == 0) {
                if (clickedOnEllipse(m_shapes[i].mainPoints, pos, true)) {
                    currentOnShape = true;
                    emit shapeClicked("effect");
                }
            } else if (m_shapes[i].isOval == 1) {
                if (clickedOnRect(m_shapes[i].mainPoints, pos, true)) {
                    currentOnShape = true;
                    emit shapeClicked("effect");
                }
            } else {
                if (clickedOnLine(m_shapes[i].mainPoints, m_shapes[i].points, pos)) {
                    continue;
                }
            }

        }
        if (m_shapes[i].type == "arrow") {
            if (clickedOnArrow(m_shapes[i].points, pos)) {
                currentOnShape = true;
                emit shapeClicked("arrow");
            }
        }
        if (m_shapes[i].type == "line") {
            if (clickedOnArrow(m_shapes[i].points, pos)) {
                currentOnShape = true;
                emit shapeClicked("line");
            }
        }
        if (m_shapes[i].type == "pen") {
            if (clickedOnLine(m_shapes[i].mainPoints, m_shapes[i].points, pos)) {
                currentOnShape = true;
                emit shapeClicked("pen");
            }
        }

        if (m_shapes[i].type == "text") {
            if (clickedOnText(m_shapes[i].mainPoints, pos)) {
                currentOnShape = true;
                emit shapeClicked("text");
            }
        }

        if (currentOnShape) {
            m_selectedShape = m_shapes[i];
            m_selectedIndex = m_shapes[i].index;
            m_selectedOrder = i;
            onShapes = true;
            break;
        } else {
            m_selectedIndex = -1;
            m_selectedOrder = -1;
            update();
            continue;
        }
    }
    return onShapes;
}

//判断是否选中图形,不是真实鼠标事件会触发
bool ShapesWidget::clickedShapes(QPointF pos)
{
    for (int i = 0; i < m_shapes.length(); i++) {
        if (m_shapes[i].type == "rectangle") {
            if (clickedOnRect(m_shapes[i].mainPoints, pos, false)) {
                return true;
            }
        }
        if (m_shapes[i].type == "oval") {
            if (clickedOnEllipse(m_shapes[i].mainPoints, pos, false)) {
                return true;
            }
        }

        if (m_shapes[i].type == "effect") {
            if (m_shapes[i].isOval == 0) {
                if (clickedOnEllipse(m_shapes[i].mainPoints, pos, true)) {
                    return true;
                }
            } else if (m_shapes[i].isOval == 1) {
                if (clickedOnRect(m_shapes[i].mainPoints, pos, true)) {
                    return true;
                }
            } else {
                if (clickedOnLine(m_shapes[i].mainPoints, m_shapes[i].points, pos)) {
                    return true;
                }
            }
        }

        if (m_shapes[i].type == "arrow" || m_shapes[i].type == "line") {
            if (clickedOnArrow(m_shapes[i].points, pos)) {
                return true;
            }
        }
        if (m_shapes[i].type == "pen") {
            if (clickedOnLine(m_shapes[i].mainPoints, m_shapes[i].points, pos)) {
                return true;
            }
        }

        if (m_shapes[i].type == "text") {
            if (clickedOnText(m_shapes[i].mainPoints, pos)) {
                return true;
            }
        }
    }
    return false;
}

//TODO: selectUnique
//矩形是否被点击
bool ShapesWidget::clickedOnRect(FourPoints rectPoints, QPointF pos, bool isBlurMosaic)
{
    m_isSelected = false;
    m_isResize = false;
    m_isRotated = false;

    QPointF point1 = rectPoints[0];
    QPointF point2 = rectPoints[1];
    QPointF point3 = rectPoints[2];
    QPointF point4 = rectPoints[3];

    FourPoints otherFPoints = getAnotherFPoints(rectPoints);
    if (pointClickIn(point1, pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = First;
        m_resizeDirection = TopLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(point2, pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Second;
        m_resizeDirection = BottomLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(point3, pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Third;
        m_resizeDirection = TopRight;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(point4, pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fourth;
        m_resizeDirection = BottomRight;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[0], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fifth;
        m_resizeDirection = Left;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[1], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Sixth;
        m_resizeDirection = Top;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[2], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Seventh;
        m_resizeDirection = Right;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[3], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Eighth;
        m_resizeDirection = Bottom;
        m_pressedPoint = pos;
        return true;
    } else if (rotateOnPoint(rectPoints, pos)) {
        m_isSelected = true;
        m_isRotated = true;
        m_isResize = false;
        m_resizeDirection = Rotate;
        m_pressedPoint = pos;
        return true;
    } else if (pointOnLine(rectPoints[0], rectPoints[1], pos) ||
               pointOnLine(rectPoints[1], rectPoints[3], pos) ||
               pointOnLine(rectPoints[3], rectPoints[2], pos) ||
               pointOnLine(rectPoints[2], rectPoints[0], pos)) {
        m_isSelected = true;
        m_isResize = false;
        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else if (isBlurMosaic && pointInRect(rectPoints, pos)) {
        m_isSelected = true;
        m_isResize = false;
        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    }  else {
        m_isSelected = false;
        m_isResize = false;
        m_isRotated = false;
    }

    return false;
}

//圆形是否被点击
bool ShapesWidget::clickedOnEllipse(FourPoints mainPoints, QPointF pos, bool isBlurMosaic)
{
    m_isSelected = false;
    m_isResize = false;
    m_isRotated = false;

    m_pressedPoint = pos;
    FourPoints otherFPoints = getAnotherFPoints(mainPoints);
    if (pointClickIn(mainPoints[0], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = First;
        m_resizeDirection = TopLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(mainPoints[1], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Second;
        m_resizeDirection = BottomLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(mainPoints[2], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Third;
        m_resizeDirection = TopRight;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(mainPoints[3], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fourth;
        m_resizeDirection = BottomRight;
        m_pressedPoint = pos;
        return true;
    }  else if (pointClickIn(otherFPoints[0], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fifth;
        m_resizeDirection = Left;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[1], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Sixth;
        m_resizeDirection = Top;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[2], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Seventh;
        m_resizeDirection = Right;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[3], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Eighth;
        m_resizeDirection = Bottom;
        m_pressedPoint = pos;
        return true;
    } else if (rotateOnPoint(mainPoints, pos)) {
        m_isSelected = true;
        m_isRotated = true;
        m_isResize = false;
        m_resizeDirection = Rotate;
        m_pressedPoint = pos;
        return true;
    }  else if (pointOnEllipse(mainPoints, pos)) {
        m_isSelected = true;
        m_isResize = false;

        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else if (isBlurMosaic && pointInRect(mainPoints, pos)) {
        m_isSelected = true;
        m_isResize = false;
        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else {
        m_isSelected = false;
        m_isResize = false;
        m_isRotated = false;
    }

    return false;
}

//箭头是否被点击
bool ShapesWidget::clickedOnArrow(QList<QPointF> points, QPointF pos)
{
    if (points.length() != 2)
        return false;

    m_isSelected = false;
    m_isResize = false;
    m_isRotated = false;
    m_isArrowRotated = false;


    if (pointClickIn(points[0], pos)) {
        m_isSelected = true;
        m_isRotated = true;
        m_isArrowRotated = false;
        m_resizeDirection = Left;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(points[1], pos)) {
        m_isSelected = true;
        m_isRotated = true;
        m_isArrowRotated = false;
        m_resizeDirection = Right;
        m_pressedPoint = pos;
        return true;
    } else if (pointOnLine(points[0], points[1], pos)) {
        m_isSelected = true;
        m_isRotated = false;
        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else {
        m_isSelected = false;
        m_isRotated = false;
        m_isResize = false;
        m_isArrowRotated = false;
        m_resizeDirection = Outting;
        m_pressedPoint = pos;
        return false;
    }

}

//画出的线是否被点击
bool ShapesWidget::clickedOnLine(FourPoints mainPoints,
                                 QList<QPointF> points,
                                 QPointF pos)
{
    m_isSelected = false;
    m_isResize = false;
    m_isRotated = false;

    m_pressedPoint = QPoint(0, 0);
    FourPoints otherFPoints = getAnotherFPoints(mainPoints);
    if (pointClickIn(mainPoints[0], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = First;
        m_resizeDirection = TopLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(mainPoints[1], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Second;
        m_resizeDirection = BottomLeft;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(mainPoints[2], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Third;
        m_resizeDirection = TopRight;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(mainPoints[3], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fourth;
        m_resizeDirection = BottomRight;
        m_pressedPoint = pos;
        return true;
    }  else if (pointClickIn(otherFPoints[0], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Fifth;
        m_resizeDirection = Left;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[1], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Sixth;
        m_resizeDirection = Top;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[2], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Seventh;
        m_resizeDirection = Right;
        m_pressedPoint = pos;
        return true;
    } else if (pointClickIn(otherFPoints[3], pos)) {
        m_isSelected = true;
        m_isResize = true;
        m_clickedKey = Eighth;
        m_resizeDirection = Bottom;
        m_pressedPoint = pos;
        return true;
    } else if (rotateOnPoint(mainPoints, pos)) {
        m_isSelected = true;
        m_isRotated = true;
        m_isResize = false;
        m_resizeDirection = Rotate;
        m_pressedPoint = pos;
        return true;
    }  else if (pointOnArLine(points, pos)) {
        m_isSelected = true;
        m_isResize = false;

        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else {
        m_isSelected = false;
        m_isResize = false;
        m_isRotated = false;
    }

    return false;
}

//文本框是否被点击
bool ShapesWidget::clickedOnText(FourPoints mainPoints, QPointF pos)
{
    if (pointInRect(mainPoints, pos)) {
        m_isSelected = true;
        m_isResize = false;
        m_resizeDirection = Moving;

        return true;
    } else {
        m_isSelected = false;
        m_isResize = false;

        return false;
    }
}

bool ShapesWidget::hoverOnRect(FourPoints rectPoints, QPointF pos, bool isTextBorder)
{
    FourPoints tmpFPoints = getAnotherFPoints(rectPoints);
    if (pointClickIn(rectPoints[0], pos)) {
        m_resizeDirection = TopLeft;
        return true;
    } else if (pointClickIn(rectPoints[1], pos)) {
        m_resizeDirection = BottomLeft;
        return true;
    } else if (pointClickIn(rectPoints[2], pos)) {
        m_resizeDirection = TopRight;
        return true;
    } else if (pointClickIn(rectPoints[3], pos)) {
        m_resizeDirection = BottomRight;
        return true;
    } else if (rotateOnPoint(rectPoints, pos) && m_selectedIndex != -1
               && m_selectedIndex == m_hoveredIndex && !isTextBorder) {
        m_resizeDirection = Rotate;
        return true;
    }  else if (pointClickIn(tmpFPoints[0], pos)) {
        m_resizeDirection = Left;
        return true;
    } else if (pointClickIn(tmpFPoints[1], pos)) {
        m_resizeDirection = Top;
        return true;
    }  else if (pointClickIn(tmpFPoints[2], pos)) {
        m_resizeDirection = Right;
        return true;
    } else if (pointClickIn(tmpFPoints[3], pos)) {
        m_resizeDirection = Bottom;
        return true;
    } else if (pointOnLine(rectPoints[0],  rectPoints[1], pos) || pointOnLine(rectPoints[1],
                                                                              rectPoints[3], pos) || pointOnLine(rectPoints[3], rectPoints[2], pos) ||
               pointOnLine(rectPoints[2], rectPoints[0], pos)) {
        m_resizeDirection = Moving;
        return true;
    } else {
        m_resizeDirection = Outting;
    }
    return false;
}

bool ShapesWidget::hoverOnEllipse(FourPoints mainPoints, QPointF pos)
{
    FourPoints tmpFPoints = getAnotherFPoints(mainPoints);

    if (pointClickIn(mainPoints[0], pos)) {
        m_resizeDirection = TopLeft;
        return true;
    } else if (pointClickIn(mainPoints[1], pos)) {
        m_resizeDirection = BottomLeft;
        return true;
    } else if (pointClickIn(mainPoints[2], pos)) {
        m_resizeDirection = TopRight;
        return true;
    } else if (pointClickIn(mainPoints[3], pos)) {
        m_resizeDirection = BottomRight;
        return true;
    } else if (rotateOnPoint(mainPoints, pos) && m_selectedIndex != -1
               && m_selectedIndex == m_hoveredIndex) {
        m_resizeDirection = Rotate;
        return true;
    }  else if (pointClickIn(tmpFPoints[0], pos)) {
        m_resizeDirection = Left;
        return true;
    } else if (pointClickIn(tmpFPoints[1], pos)) {
        m_resizeDirection = Top;
        return true;
    }  else if (pointClickIn(tmpFPoints[2], pos)) {
        m_resizeDirection = Right;
        return true;
    } else if (pointClickIn(tmpFPoints[3], pos)) {
        m_resizeDirection = Bottom;
        return true;
    }  else if (pointOnEllipse(mainPoints, pos)) {
        m_isSelected = true;
        m_isResize = false;

        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else {
        m_resizeDirection = Outting;
    }
    return false;
}

bool ShapesWidget::hoverOnArrow(QList<QPointF> points, QPointF pos)
{
    if (points.length() != 2)
        return false;

    if (pointOnLine(points[0], points[1], pos)) {
        m_resizeDirection = Moving;
        return true;
    } else if (m_selectedIndex != -1 && m_selectedIndex == m_hoveredIndex
               && pointClickIn(points[0], pos)) {
        m_clickedKey = First;
        m_resizeDirection = Rotate;
        return true;
    } else if (m_selectedIndex != -1 && m_selectedIndex == m_hoveredIndex
               && pointClickIn(points[1], pos)) {
        m_clickedKey =   Second;
        m_resizeDirection = Rotate;
        return true;
    } else {
        m_resizeDirection = Outting;
        return false;
    }
}

bool ShapesWidget::hoverOnLine(FourPoints mainPoints, QList<QPointF> points,
                               QPointF pos)
{
    FourPoints tmpFPoints = getAnotherFPoints(mainPoints);

    if (pointClickIn(mainPoints[0], pos)) {
        m_resizeDirection = TopLeft;
        return true;
    } else if (pointClickIn(mainPoints[1], pos)) {
        m_resizeDirection = BottomLeft;
        return true;
    } else if (pointClickIn(mainPoints[2], pos)) {
        m_resizeDirection = TopRight;
        return true;
    } else if (pointClickIn(mainPoints[3], pos)) {
        m_resizeDirection = BottomRight;
        return true;
    } else if (rotateOnPoint(mainPoints, pos) && m_selectedIndex != -1
               && m_selectedIndex == m_hoveredIndex) {
        m_resizeDirection = Rotate;
        return true;
    }  else if (pointClickIn(tmpFPoints[0], pos)) {
        m_resizeDirection = Left;
        return true;
    } else if (pointClickIn(tmpFPoints[1], pos)) {
        m_resizeDirection = Top;
        return true;
    }  else if (pointClickIn(tmpFPoints[2], pos)) {
        m_resizeDirection = Right;
        return true;
    } else if (pointClickIn(tmpFPoints[3], pos)) {
        m_resizeDirection = Bottom;
        return true;
    }  else if (pointOnArLine(points, pos)) {
        m_isSelected = true;
        m_isResize = false;

        m_resizeDirection = Moving;
        m_pressedPoint = pos;
        return true;
    } else {
        m_resizeDirection = Outting;
    }
    return false;
}

bool ShapesWidget::hoverOnText(int textIndex, FourPoints mainPoints, QPointF pos)
{
    //qDebug() << "hoverOnText:" <<  mainPoints << pos;
    if (hoverOnRect(mainPoints, pos, true) ||
            (pos.x() >= mainPoints[0].x() - 5
             && pos.x() <= mainPoints[2].x() + 5
             && pos.y() >= mainPoints[0].y() - 5
             && pos.y() <= mainPoints[2].y() + 5)) {

        if (m_editMap.contains(textIndex) && m_editMap[textIndex]->isReadOnly()) {
            m_resizeDirection = Moving;
            return true;
        }
    }

    m_resizeDirection = Outting;
    return false;
}

bool ShapesWidget::hoverOnShapes(Toolshape toolShape, QPointF pos)
{
    if (toolShape.type == "rectangle") {
        return hoverOnRect(toolShape.mainPoints, pos);
    } else if (toolShape.type == "oval") {
        return hoverOnEllipse(toolShape.mainPoints, pos);
    } else if (toolShape.type == "arrow" || toolShape.type == "line") {
        return hoverOnArrow(toolShape.points, pos);
    } else if (toolShape.type == "pen") {
        return hoverOnLine(toolShape.mainPoints, toolShape.points, pos);
    } else if (toolShape.type == "text") {
        return hoverOnText(toolShape.index, toolShape.mainPoints, pos);
    } else if (toolShape.type == "effect" && toolShape.isOval == 0) {
        return hoverOnEllipse(toolShape.mainPoints, pos);
    } else if (toolShape.type == "effect" && toolShape.isOval == 1) {
        return hoverOnRect(toolShape.mainPoints, pos);
    }

    m_hoveredShape.type = "";
    return false;
}

bool ShapesWidget::rotateOnPoint(FourPoints mainPoints,
                                 QPointF pos)
{
    bool result = hoverOnRotatePoint(mainPoints, pos);
    return result;
}

bool ShapesWidget::hoverOnRotatePoint(FourPoints mainPoints, QPointF pos)
{
    QPointF rotatePoint = getRotatePoint(mainPoints[0], mainPoints[1],
                                         mainPoints[2], mainPoints[3]);
    rotatePoint = QPointF(rotatePoint.x() - 5, rotatePoint.y() - 5);
    bool result = false;
    if (pos.x() >= rotatePoint.x() - SPACING && pos.x() <= rotatePoint.x() + SPACING
            && pos.y() >= rotatePoint.y() - SPACING && pos.y() <= rotatePoint.y() + SPACING) {
        result = true;
    } else {
        result = false;
    }

    m_pressedPoint = rotatePoint;
    return result;
}

bool ShapesWidget::textEditIsReadOnly()
{
    qDebug() << "textEditIsReadOnly:" << m_editMap.count();

    QMap<int, TextEdit *>::iterator i = m_editMap.begin();
    while (i != m_editMap.end()) {
        if (m_editing || !i.value()->isReadOnly()) {
            setAllTextEditReadOnly();
            m_editing = false;
            m_currentShape.type = "";
            update();
            return true;
        }
        ++i;
    }

    return false;
}

void ShapesWidget::handleDrag(QPointF oldPoint, QPointF newPoint)
{
    //qDebug() << "handleDrag:" << m_selectedIndex << m_shapes.length();

    if (m_selectedIndex == -1) {
        return;
    }

    if (m_shapes[m_selectedOrder].type == "arrow" || m_shapes[m_selectedOrder].type == "line") {
        for (int i = 0; i < m_shapes[m_selectedOrder].points.length(); i++) {
            m_shapes[m_selectedOrder].points[i] = QPointF(
                                                      m_shapes[m_selectedOrder].points[i].x() + (newPoint.x() - oldPoint.x()),
                                                      m_shapes[m_selectedOrder].points[i].y() + (newPoint.y() - oldPoint.y())
                                                  );
        }
        return;
    }

    if (m_shapes[m_selectedOrder].mainPoints.length() == 4) {
        for (int i = 0; i < m_shapes[m_selectedOrder].mainPoints.length(); i++) {
            m_shapes[m_selectedOrder].mainPoints[i] = QPointF(
                                                          m_shapes[m_selectedOrder].mainPoints[i].x() + (newPoint.x() - oldPoint.x()),
                                                          m_shapes[m_selectedOrder].mainPoints[i].y() + (newPoint.y() - oldPoint.y())
                                                      );
        }
    }
    for (int i = 0; i < m_shapes[m_selectedOrder].points.length(); i++) {
        m_shapes[m_selectedOrder].points[i] = QPointF(
                                                  m_shapes[m_selectedOrder].points[i].x() + (newPoint.x() - oldPoint.x()),
                                                  m_shapes[m_selectedOrder].points[i].y() + (newPoint.y() - oldPoint.y())
                                              );
    }
}

////////////////////TODO: perfect handleRotate..
void ShapesWidget::handleRotate(QPointF pos)
{
    //qDebug() << "handleRotate:" << m_selectedIndex << m_shapes.length();

    if (m_selectedIndex == -1 || m_selectedShape.type == "text") {
        return;
    }

    if (m_selectedShape.type == "arrow" || m_selectedShape.type == "line") {
        if (m_isArrowRotated == false) {
            if (m_shapes[m_selectedOrder].isShiftPressed) {
                if (static_cast<int>(m_shapes[m_selectedOrder].points[0].x()) == static_cast<int>(m_shapes[m_selectedOrder].points[1].x())) {
                    if (m_clickedKey == First) {
                        m_shapes[m_selectedOrder].points[0] = QPointF(m_shapes[m_selectedOrder].points[1].x(),
                                                                      pos.y());
                    } else if (m_clickedKey == Second) {
                        m_shapes[m_selectedOrder].points[1] = QPointF(m_shapes[m_selectedOrder].points[0].x(),
                                                                      pos.y());
                    }
                } else {
                    if (m_clickedKey == First) {
                        m_shapes[m_selectedOrder].points[0] = QPointF(pos.x(), m_shapes[m_selectedOrder].points[1].y());
                    } else if (m_clickedKey == Second) {
                        m_shapes[m_selectedOrder].points[1] = QPointF(pos.x(), m_shapes[m_selectedOrder].points[0].y());
                    }
                }
            } else {
                if (m_clickedKey == First) {
                    m_shapes[m_selectedOrder].points[0] = m_pressedPoint;
                } else if (m_clickedKey == Second) {
                    m_shapes[m_selectedOrder].points[1] = m_pressedPoint;
                }
            }
        } else {
            QPointF t_rotatepos;

            qreal t_minx = qMin(m_selectedShape.points[1].x(), m_selectedShape.points[0].x());
            qreal t_miny = qMin(m_selectedShape.points[1].y(), m_selectedShape.points[0].y());
            int t_height = static_cast<int>(qAbs(m_selectedShape.points[1].y() - m_selectedShape.points[0].y()));
            int t_width = static_cast<int>(qAbs(m_selectedShape.points[1].x() - m_selectedShape.points[0].x()));

            t_rotatepos.setX(t_minx + t_width / 2);
            t_rotatepos.setY(t_miny + t_height / 2);

            qreal angle = calculateAngle(m_pressedPoint, pos, t_rotatepos) / 35;
            for (int k = 0; k < m_selectedShape.points.length(); k++) {
                m_shapes[m_selectedOrder].points[k] = pointRotate(t_rotatepos,
                                                                  m_selectedShape.points[k], angle);
            }
        }

        m_selectedShape.points  =  m_shapes[m_selectedOrder].points;
        m_hoveredShape.points = m_shapes[m_selectedOrder].points;
        m_pressedPoint = pos;
        return;
    }

    QPointF centerInPoint = QPointF((m_selectedShape.mainPoints[0].x() +
                                     m_selectedShape.mainPoints[3].x()) / 2,
                                    (m_selectedShape.mainPoints[0].y() +
                                     m_selectedShape.mainPoints[3].y()) / 2);
    QLineF linePressed(centerInPoint, m_movingPoint);
    qreal pressedAngle = 360 - linePressed.angle();
    qreal angle;
    if (0 == static_cast<int>(m_lastAngle)) {
        m_lastAngle = pressedAngle;
        angle = 0;
    } else {
        angle = pressedAngle - m_lastAngle;
    }
    if (angle < -100) {
        m_lastAngle = pressedAngle - m_lastAngle + 360;
        angle = m_lastAngle;
    } else if (angle > 100) {
        m_lastAngle = pressedAngle - m_lastAngle - 360;
        angle = m_lastAngle;
    }
    qreal rotationDelta = (angle * M_PI) / 180;
    qreal centerX = (m_shapes[m_selectedOrder].mainPoints[0].x() + m_shapes[m_selectedOrder].mainPoints[3].x()) / 2;
    qreal centerY = (m_shapes[m_selectedOrder].mainPoints[0].y() + m_shapes[m_selectedOrder].mainPoints[3].y()) / 2;
    for (int i = 0; i < m_shapes[m_selectedOrder].mainPoints.size(); ++i) {
        qreal x = centerX + (m_shapes[m_selectedOrder].mainPoints[i].x() - centerX) * cos(rotationDelta) - (m_shapes[m_selectedOrder].mainPoints[i].y() - centerY) * sin(rotationDelta);
        qreal y = centerY + (m_shapes[m_selectedOrder].mainPoints[i].x() - centerX) * sin(rotationDelta) + (m_shapes[m_selectedOrder].mainPoints[i].y() - centerY) * cos(rotationDelta);
        //图形
        m_shapes[m_selectedOrder].mainPoints[i].setX(x);
        m_shapes[m_selectedOrder].mainPoints[i].setY(y);
    }
    qreal centerMainX = (m_selectedShape.mainPoints[0].x() + m_selectedShape.mainPoints[3].x()) / 2;
    qreal centerMainY = (m_selectedShape.mainPoints[0].y() + m_selectedShape.mainPoints[3].y()) / 2;
    for (int k = 0; k < m_selectedShape.points.length(); k++) {
        qreal x = centerMainX + (m_shapes[m_selectedOrder].points[k].x() - centerMainX) * cos(rotationDelta) - (m_shapes[m_selectedOrder].points[k].y() - centerMainY) * sin(rotationDelta);
        qreal y = centerMainY + (m_shapes[m_selectedOrder].points[k].x() - centerMainX) * sin(rotationDelta) + (m_shapes[m_selectedOrder].points[k].y() - centerMainY) * cos(rotationDelta);
        //线条
        m_shapes[m_selectedOrder].points[k].setX(x);
        m_shapes[m_selectedOrder].points[k].setY(y);
    }
    m_lastAngle = pressedAngle;

    m_selectedShape.mainPoints = m_shapes[m_selectedOrder].mainPoints;
    m_hoveredShape.mainPoints =  m_shapes[m_selectedOrder].mainPoints;
    m_pressedPoint = pos;
}

void ShapesWidget::handleResize(QPointF pos, int key)
{
    if (m_isResize && m_selectedIndex != -1) {
        if (m_shapes[m_selectedOrder].portion.isEmpty()) {
            for (int k = 0; k < m_shapes[m_selectedOrder].points.length(); k++) {
                m_shapes[m_selectedOrder].portion.append(relativePosition(
                                                             m_selectedShape.mainPoints, m_selectedShape.points[k]));
            }
        }

        FourPoints newResizeFPoints = resizePointPosition(
                                          m_shapes[m_selectedOrder].mainPoints[0],
                                          m_shapes[m_selectedOrder].mainPoints[1],
                                          m_shapes[m_selectedOrder].mainPoints[2],
                                          m_shapes[m_selectedOrder].mainPoints[3], pos, key,
                                          m_isShiftPressed);

        m_shapes[m_selectedOrder].mainPoints = newResizeFPoints;
        m_selectedShape.mainPoints = newResizeFPoints;
        m_hoveredShape.mainPoints = newResizeFPoints;

        for (int j = 0; j <  m_shapes[m_selectedOrder].portion.length(); j++) {
            m_shapes[m_selectedOrder].points[j] =
                getNewPosition(m_shapes[m_selectedOrder].mainPoints,
                               m_shapes[m_selectedOrder].portion[j]);
        }

        m_selectedShape.points = m_shapes[m_selectedOrder].points;
        m_hoveredShape.points = m_shapes[m_selectedOrder].points;
    }
    m_pressedPoint = pos;
}

bool ShapesWidget::event(QEvent *event)
{
    if (QEvent::Gesture == event->type()) {
        if (QGesture *tap = static_cast<QGestureEvent *>(event)->gesture(Qt::TapGesture)) {
            tapTriggered(static_cast<QTapGesture *>(tap));
        }
        if (QGesture *pinch = static_cast<QGestureEvent *>(event)->gesture(Qt::PinchGesture)) {
            pinchTriggered(static_cast<QPinchGesture *>(pinch));
        }
        update();
    }
    return QWidget::event(event);
}

//重写鼠标按压事件
void ShapesWidget::mousePressEvent(QMouseEvent *e)
{
    m_lastAngle = 0;
    m_currentCursor = QCursor().pos();
    //选中图形后，重新按下真实鼠标左键
    if (Qt::MouseEventSource::MouseEventSynthesizedByQt != e->source()
            && m_selectedIndex != -1) {
        // 判空
        if (nullptr != m_editMap.value(m_lastEditMapKey)) {
            // 点击鼠标左键时，去掉未更改的textEdit文本框
            if (m_editMap.value(m_lastEditMapKey)->toPlainText() == QString(tr("Input text here")) ||
                    m_editMap.value(m_lastEditMapKey)->toPlainText().isEmpty()) {
                clearSelected();
                setAllTextEditReadOnly();
                m_editing = false;
                m_selectedIndex = -1;
                m_selectedOrder = -1;
                m_selectedShape.type = "";
                update();
                DFrame::mousePressEvent(e);
            }
        }
    }

    //选中图形后，重新按下触摸屏左键
    if (Qt::MouseEventSource::MouseEventSynthesizedByQt == e->source()
            && m_selectedIndex != -1
            && !clickedShapes(e->pos())
            && "text" != m_currentType) {
        clearSelected();
        setAllTextEditReadOnly();
        m_editing = false;
        m_selectedIndex = -1;
        m_selectedOrder = -1;
        m_selectedShape.type = "";
        update();
        DFrame::mousePressEvent(e);

        //return;

    }

    if (m_selectedIndex != -1) {
        if (!(clickedOnShapes(e->pos()) && m_isRotated) && m_selectedIndex == -1 && "text" == m_currentType) {
            clearSelected();
            setAllTextEditReadOnly();
            m_editing = false;
            m_selectedIndex = -1;
            m_selectedOrder = -1;
            m_selectedShape.type = "";
            update();
            DFrame::mousePressEvent(e);
            return;
        }
    }

    //鼠标右键点击
    if (Qt::MouseEventSource::MouseEventSynthesizedByQt != e->source() && e->button() == Qt::RightButton) {
        m_pos1 = QPointF(0, 0); // 修复触控屏绘制矩形后，长按屏幕会出现多余矩形的问题
        qDebug() << "RightButton clicked!" << e->source();
        m_menuController->showMenu(QPoint(mapToGlobal(e->pos())));
        DFrame::mousePressEvent(e);
        return;
    }

    m_pressedPoint = e->pos();
    m_isPressed = true;
    if (!clickedOnShapes(m_pressedPoint)) {

        m_isRecording = true;
        //qDebug() << "no one shape be clicked!" << m_selectedIndex << m_shapes.length();

        m_currentShape.type = m_currentType;
        m_currentShape.colorIndex = ConfigSettings::instance()->getValue(m_currentType, "color_index").toInt();
        m_currentShape.lineWidth = LINEWIDTH(ConfigSettings::instance()->getValue(m_currentType, "line_width").toInt());

        m_selectedIndex = -1;
        m_shapesIndex += 1;
        m_currentIndex = m_shapesIndex;

        if (m_pos1 == QPointF(0, 0)) {
            m_pos1 = e->pos();
            if (m_currentType == "pen") {
                m_currentShape.index = m_currentIndex;
                m_currentShape.points.append(m_pos1);
            } else if (m_currentType == "arrow") {
                m_currentShape.index = m_currentIndex;
                m_currentShape.isShiftPressed = m_isShiftPressed;
                m_currentShape.points.append(m_pos1);
                m_currentShape.lineWidth = LINEWIDTH(ConfigSettings::instance()->getValue("arrow", "line_width").toInt());
            } else if (m_currentType == "line") {
                m_currentShape.index = m_currentIndex;
                m_currentShape.isShiftPressed = m_isShiftPressed;
                m_currentShape.points.append(m_pos1);
                m_currentShape.lineWidth = LINEWIDTH(ConfigSettings::instance()->getValue("line", "line_width").toInt());
            } else if (m_currentType == "effect") {
                m_currentShape.isShiftPressed = m_isShiftPressed;
                m_currentShape.index = m_currentIndex;
                m_currentShape.isBlur = ConfigSettings::instance()->getValue("effect", "isBlur").toBool();
                m_currentShape.isOval = ConfigSettings::instance()->getValue("effect", "isOval").toInt();
                m_currentShape.radius = ConfigSettings::instance()->getValue("effect", "radius").toInt() * 3 + 10;
                m_currentShape.lineWidth = LINEWIDTH(ConfigSettings::instance()->getValue("effect", "line_width").toInt());
                if (m_currentShape.isBlur) {
                    emit reloadEffectImg("blur", m_currentShape.radius);
                } else {
                    emit reloadEffectImg("mosaic", m_currentShape.radius);
                }
                if (m_currentShape.isOval == 2) {
                    m_currentShape.points.append(m_pos1);
                }
            } else if (m_currentType == "rectangle") {
                m_currentShape.isShiftPressed = m_isShiftPressed;
                m_currentShape.index = m_currentIndex;
            } else if (m_currentType == "oval") {
                m_currentShape.isShiftPressed = m_isShiftPressed;
                m_currentShape.index = m_currentIndex;
            } else if (m_currentType == "text") {
                if (!m_editing) {
                    setAllTextEditReadOnly();
                    setNoChangedTextEditRemove();
                    m_currentShape.mainPoints[0] = m_pos1;
                    m_currentShape.index = m_currentIndex;
                    qDebug() << "new textedit:" << m_currentIndex;
                    TextEdit *edit = new TextEdit(m_currentIndex, this);
                    QString t_editText = QString(tr("Input text here"));
                    edit->appendPlainText(t_editText);
//                    edit->setPlainText(t_editText);
                    m_editing = true;
                    edit->setEditing(true);
                    int defaultFontSize = ConfigSettings::instance()->getValue("text", "fontsize").toInt();
                    m_currentShape.fontSize = defaultFontSize;


                    m_currentShape.mainPoints[0] = m_pos1;
                    m_currentShape.mainPoints[1] = QPointF(m_pos1.x(), m_pos1.y() + edit->height());
                    m_currentShape.mainPoints[2] = QPointF(m_pos1.x() + edit->width(), m_pos1.y());
                    m_currentShape.mainPoints[3] = QPointF(m_pos1.x() + edit->width(),
                                                           m_pos1.y() + edit->height());
                    m_editMap.insert(m_currentIndex, edit);
                    m_selectedIndex = m_currentIndex;
                    m_selectedShape = m_currentShape;
                    m_lastEditMapKey = m_currentIndex;


                    connect(edit, &TextEdit::repaintTextRect, this, &ShapesWidget::updateTextRect);
                    connect(edit, &TextEdit::clickToEditing, this, [ = ](int index) {
                        //                        setAllTextEditReadOnly();
                        for (int k = 0; k < m_shapes.length(); k++) {
                            if (m_shapes[k].type == "text" && m_shapes[k].index == index) {
                                m_selectedIndex = index;
                                m_selectedShape = m_shapes[k];
                                m_selectedOrder = k;
                                m_currentShape = m_selectedShape;
                                break;
                            }
                        }
                        QMap<int, TextEdit *>::iterator i = m_editMap.begin();
                        while (i != m_editMap.end()) {
                            if (i.key() != index) {
                                i.value()->setReadOnly(true);
                                i.value()->setEditing(false);
                            } else {
                                i.value()->setEditing(true);
                            }
                            QTextCursor textCursor =  i.value()->textCursor();
                            textCursor.clearSelection();
                            i.value()->setTextCursor(textCursor);
                            ++i;
                        }
                        m_editing = true;
                        m_isSelectedText = false;
                        emit shapeClicked("text");

                    });

                    connect(edit, &TextEdit::textEditSelected, this, [ = ](int index) {
                        //                        setAllTextEditReadOnly();
                        if (m_selectedIndex != index) {
                            m_editing = false;
                            m_currentShape.type = "";
                            for (int i = 0; i < m_currentShape.mainPoints.length(); i++) {
                                m_currentShape.mainPoints[i] = QPointF(0, 0);
                            }
                        }
                        for (int k = 0; k < m_shapes.length(); k++) {
                            if (m_shapes[k].type == "text" && m_shapes[k].index == index) {
                                m_selectedIndex = index;
                                m_selectedShape = m_shapes[k];
                                m_selectedOrder = k;
                                break;
                            }
                        }
                        QMap<int, TextEdit *>::iterator i = m_editMap.begin();
                        while (i != m_editMap.end()) {
                            if (i.key() != index) {
                                i.value()->setReadOnly(true);
                                i.value()->setEditing(false);
                            }

                            QTextCursor textCursor =  i.value()->textCursor();
                            textCursor.clearSelection();
                            i.value()->setTextCursor(textCursor);
                            ++i;
                        }
                        m_isSelectedText = true;
                        emit shapeClicked("text");
                    });

                    connect(edit, &TextEdit::textEditFinish, this, [ = ](int index) {
                        //                        setAllTextEditReadOnly();
                        Q_UNUSED(index);
                        setAllTextEditReadOnly();
                    });

                    edit->setSelecting(true);
                    edit->setFocus();
                    edit->move(static_cast<int>(m_pos1.x()), static_cast<int>(m_pos1.y()));
                    edit->show();
                    QTextCursor cs = edit->textCursor();
                    edit->moveCursor(QTextCursor::Start);
                    cs.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, t_editText.length());
                    edit->setTextCursor(cs);
                    edit->selectAll();
                    m_shapes.append(m_currentShape);


                    for (int k = 0; k < m_shapes.length(); k++) {
                        if (m_shapes[k].type == "text" && m_shapes[k].index == m_currentIndex) {
                            m_selectedOrder = k;
                            break;
                        }
                    }

                    qDebug() << "Insert text shape:" << m_shapes.size() << m_currentShape.type << m_currentShape.index;
                } else {
                    m_editing = false;
                    setAllTextEditReadOnly();

                }
            }
            update();
        }
    } else {
        m_isRecording = false;
        //qDebug() << "some on shape be clicked!";
        if (m_editing && m_editMap.contains(m_shapes[m_selectedOrder].index)) {
            m_editMap.value(m_shapes[m_selectedOrder].index)->setReadOnly(true);
            m_editMap.value(m_shapes[m_selectedOrder].index)->setCursorVisible(false);
            m_editMap.value(m_shapes[m_selectedOrder].index)->setFocusPolicy(Qt::NoFocus);
        }
        update();
    }
    DFrame::mousePressEvent(e);

}

void ShapesWidget::mouseReleaseEvent(QMouseEvent *e)
{
    m_isPressed = false;
    m_isMoving = false;

    if (Qt::MouseEventSource::MouseEventSynthesizedByQt == e->source()
            && -1 != m_selectedIndex
            && "text" != m_currentType) {
        return;
    }

    if (!m_isRotated && m_selectedIndex == -1 && "text" != m_currentType) {
        clearSelected();
        setAllTextEditReadOnly();
        m_editing = false;
        m_selectedIndex = -1;
        m_selectedOrder = -1;
        m_selectedShape.type = "";
        update();
        DFrame::mousePressEvent(e);
    }

    //qDebug() << m_isRecording << m_isSelected << m_pos2;
    if (m_isRecording && !m_isSelected && m_pos2 != QPointF(0, 0)) {
        if (m_currentType == "arrow" || m_currentType == "line") {
            if (m_currentShape.points.length() == 2) {
                if (m_isShiftPressed) {
                    if (std::atan2(std::abs(m_pos2.y() - m_pos1.y()), std::abs(m_pos2.x() - m_pos1.x()))
                            * 180 / M_PI < 45) {
                        m_pos2 = QPointF(m_pos2.x(), m_pos1.y());
                    } else {
                        m_pos2 = QPointF(m_pos1.x(), m_pos2.y());
                    }
                }

                m_currentShape.points[1] = m_pos2;
                m_currentShape.mainPoints = getMainPoints(m_currentShape.points[0], m_currentShape.points[1]);

                m_shapes.append(m_currentShape);
            }
        } else if (m_currentType == "pen") {
            FourPoints lineFPoints = fourPointsOfLine(m_currentShape.points);
            m_currentShape.mainPoints = lineFPoints;
            m_shapes.append(m_currentShape);
        } else if (m_currentType != "text") {
            FourPoints rectFPoints = getMainPoints(m_pos1, m_pos2, m_isShiftPressed);
            m_currentShape.mainPoints = rectFPoints;
            m_shapes.append(m_currentShape);
        }

        //qDebug() << "ShapesWidget num:" << m_shapes.length();
        clearSelected();
        //选中当前绘制的图形
        if (m_currentShape.type != "effect" || m_currentShape.isOval != 2) {
            m_selectedIndex = m_currentIndex;
            m_selectedShape = m_currentShape;
            m_selectedOrder = m_shapes.length() - 1;
            m_isSelected = true;
        }
    }

    m_isRecording = false;
    if (m_currentShape.type != "text") {
        for (int i = 0; i < m_currentShape.mainPoints.length(); i++) {
            m_currentShape.mainPoints[i] = QPointF(0, 0);
        }
    }

    m_currentShape.points.clear();
    m_pos1 = QPointF(0, 0);
    m_pos2 = QPointF(0, 0);

    update();
    DFrame::mouseReleaseEvent(e);
}

void ShapesWidget::mouseMoveEvent(QMouseEvent *e)
{
    m_isMoving = true;
    m_movingPoint = e->pos();
    m_currentCursor = QCursor().pos();

    if (m_isRecording && m_isPressed) {
        m_pos2 = e->pos();
        updateCursorShape();

        if (m_currentShape.type == "arrow" || m_currentShape.type == "line") {
            if (m_currentShape.points.length() <= 1) {
                if (m_isShiftPressed) {
                    if (std::atan2(std::abs(m_pos2.y() - m_pos1.y()),
                                   std::abs(m_pos2.x() - m_pos1.x())) * 180 / M_PI < 45) {
                        m_currentShape.points.append(QPointF(m_pos2.x(), m_pos1.y()));
                    } else {
                        m_currentShape.points.append(QPointF(m_pos1.x(), m_pos2.y()));
                    }
                } else {
                    m_currentShape.points.append(m_pos2);
                }
            } else {
                if (m_isShiftPressed) {
                    if (std::atan2(std::abs(m_pos2.y() - m_pos1.y()),
                                   std::abs(m_pos2.x() - m_pos1.x())) * 180 / M_PI < 45) {
                        m_currentShape.points[1] = QPointF(m_pos2.x(), m_pos1.y());
                    } else {
                        m_currentShape.points[1] = QPointF(m_pos1.x(), m_pos2.y());
                    }
                } else {
                    m_currentShape.points[1] = m_pos2;
                }
            }
        }
        if (m_currentShape.type == "pen") {
            if (getDistance(m_currentShape.points[m_currentShape.points.length() - 1], m_pos2) > 3) {
                m_currentShape.points.append(m_pos2);

            }
        }
        // 模糊笔
        if (m_currentShape.type == "effect" && m_currentShape.isOval == 2) {
            double distance = getDistance(m_currentShape.points[m_currentShape.points.length() - 1], m_pos2);
            if (distance > 14) {
                QList<QPointF> interpolationPoints = getInterpolationPoints(m_currentShape.points[m_currentShape.points.length() - 1], m_pos2,
                                                                            (distance / 7));
                for (int i = 0; i < interpolationPoints.size(); ++i) {
                    m_currentShape.points.append(interpolationPoints[i]);
                }
                m_currentShape.points.append(m_pos2);
            } else if (distance > 7) {
                m_currentShape.points.append(m_pos2);
            }
        }
        update();
    } else if (!m_isRecording && m_isPressed) {
        if (m_isRotated && m_isPressed) {
            handleRotate(e->pos());
            update();
        }

        if (m_isResize && m_isPressed) {
            // resize function
            handleResize(QPointF(e->pos()), m_clickedKey);
            update();
            DFrame::mouseMoveEvent(e);
            return;
        }

        if (m_isSelected && m_isPressed && m_selectedIndex != -1) {
            handleDrag(m_pressedPoint, m_movingPoint);
            //qDebug() << "move m_selectedOrder:" << m_selectedOrder;
            m_selectedShape = m_shapes[m_selectedOrder];
            m_hoveredShape = m_shapes[m_selectedOrder];

            if (m_selectedShape.type == "text") {
                m_editMap.value(m_selectedIndex)->move(static_cast<int>(m_selectedShape.mainPoints[0].x()),
                                                       static_cast<int>(m_selectedShape.mainPoints[0].y()));
            }

            m_pressedPoint = m_movingPoint;
            update();
        }



    } else {
        if (!m_isRecording) {
            m_isHovered = false;
            for (int i = 0; i < m_shapes.length(); i++) {
                m_hoveredIndex = m_shapes[i].index;

                if (hoverOnShapes(m_shapes[i],  e->pos())) {
                    m_isHovered = true;
                    m_hoveredShape = m_shapes[i];
                    //悬停状态时，根据悬停的位置不同，光标的形状也不同
                    if (m_resizeDirection == Left) {
                        if (m_isSelected || m_isRotated) {
                            qApp->setOverrideCursor(Qt::SizeHorCursor);
                        } else {
                            qApp->setOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == Top) {
                        if (m_isSelected || m_isRotated) {
                            qApp->setOverrideCursor(Qt::SizeVerCursor);
                        } else {
                            qApp->setOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == Right) {
                        if (m_isSelected || m_isRotated) {
                            qApp->setOverrideCursor(Qt::SizeHorCursor);
                        } else {
                            qApp->setOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == Bottom) {
                        if (m_isSelected || m_isRotated) {
                            qApp->setOverrideCursor(Qt::SizeVerCursor);
                        } else {
                            qApp->setOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == TopLeft) {
                        if (m_isSelected || m_isRotated) {
                            qApp->setOverrideCursor(Qt::SizeFDiagCursor);
                        } else {
                            qApp->setOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == BottomLeft) {
                        if (m_isSelected || m_isRotated) {
                            qApp->setOverrideCursor(Qt::SizeBDiagCursor);
                        } else {
                            qApp->setOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == TopRight) {
                        if (m_isSelected || m_isRotated) {
                            qApp->setOverrideCursor(Qt::SizeBDiagCursor);
                        } else {
                            qApp->setOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == BottomRight) {
                        if (m_isSelected || m_isRotated) {
                            qApp->setOverrideCursor(Qt::SizeFDiagCursor);
                        } else {
                            qApp->setOverrideCursor(Qt::ClosedHandCursor);
                        }
                    } else if (m_resizeDirection == Rotate) {
                        qApp->setOverrideCursor(BaseUtils::setCursorShape("rotate"));
                    } else if (m_resizeDirection == Moving) {
                        qApp->setOverrideCursor(Qt::ClosedHandCursor);
                    } else {
                        updateCursorShape();
                    }
                    //update();
                    break;
                } else {
                    updateCursorShape();
                    //update();
                }
            }
            if (!m_isHovered) {
                for (int j = 0; j < m_hoveredShape.mainPoints.length(); j++) {
                    m_hoveredShape.mainPoints[j] = QPointF(0, 0);
                }
                m_hoveredShape.type = "";
                //update();
            }
            if (m_shapes.length() == 0) {
                updateCursorShape();
            }
        } else {
            //TODO text
        }
    }
    update();
    DFrame::mouseMoveEvent(e);
}

//鼠标选中图形时，可以通过w/a/s/d及小键盘方向键控制光标移动
void ShapesWidget::keyPressEvent(QKeyEvent *keyEvent)
{
    if (m_isPressed) {
        Utils::cursorMove(m_currentCursor, keyEvent);
    }
    if (keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace) {
        deleteCurrentShape();
    } else {
        qDebug() << "ShapeWidget Exist keyEvent:" << keyEvent->text();
    }
}

void ShapesWidget::updateTextRect(TextEdit *edit, QRectF newRect)
{
    int index = edit->getIndex();
    //    qDebug() << "updateTextRect:" << newRect << index;
    for (int j = 0; j < m_shapes.length(); j++) {
        //        qDebug() << "updateTextRect  updating:" << j << m_shapes[j].index << index;
        if (m_shapes[j].type == "text" && m_shapes[j].index == index) {
            m_shapes[j].mainPoints[0] = QPointF(newRect.x(), newRect.y());
            m_shapes[j].mainPoints[1] = QPointF(newRect.x(), newRect.y() + newRect.height());
            m_shapes[j].mainPoints[2] = QPointF(newRect.x() + newRect.width(), newRect.y());
            m_shapes[j].mainPoints[3] = QPointF(newRect.x() + newRect.width(),
                                                newRect.y() + newRect.height());
            m_currentShape = m_shapes[j];
            m_selectedShape = m_shapes[j];
            m_selectedIndex = m_shapes[j].index;
            m_selectedOrder = j;
        }
    }
    update();
}

void ShapesWidget::paintImgPoint(QPainter &painter, QPointF pos, QPixmap img, bool isResize)
{
    if (isResize) {
        painter.drawPixmap(QPointF(pos.x() - DRAG_BOUND_RADIUS,
                                   pos.y() - DRAG_BOUND_RADIUS), img);
    } else {
        painter.drawPixmap(QPointF(pos.x() - 17,
                                   pos.y() - 10), img);
    }
}

void ShapesWidget::paintRect(QPainter &painter, FourPoints rectFPoints, int index,
                             ShapeBlurStatus rectStatus, bool isBlur, bool isMosaic, int radius)
{
    QPainterPath rectPath;
    if (rectStatus  == Hovered || ((isBlur || isMosaic) && index == m_selectedIndex)) {
        painter.setPen(QColor("#01bdff"));
    } else if (rectStatus == Drawing || ((isBlur | isMosaic) && index != m_selectedIndex)) {
        painter.setPen(Qt::transparent);
    }

    rectPath.moveTo(rectFPoints[0].x(), rectFPoints[0].y());
    rectPath.lineTo(rectFPoints[1].x(), rectFPoints[1].y());
    rectPath.lineTo(rectFPoints[3].x(), rectFPoints[3].y());
    rectPath.lineTo(rectFPoints[2].x(), rectFPoints[2].y());
    rectPath.lineTo(rectFPoints[0].x(), rectFPoints[0].y());
    painter.drawPath(rectPath);

    //    using namespace utils;
    if (isBlur) {
        painter.setClipPath(rectPath);
        painter.drawPixmap(0, 0,  width(), height(),  TempFile::instance()->getBlurPixmap(radius));
        painter.drawPath(rectPath);
    }
    if (isMosaic) {
        painter.setClipPath(rectPath);
        painter.drawPixmap(0, 0,  width(), height(),  TempFile::instance()->getMosaicPixmap(radius));
        painter.drawPath(rectPath);
    }
    painter.setClipping(false);
}

void ShapesWidget::paintEllipse(QPainter &painter, FourPoints ellipseFPoints, int index,
                                ShapeBlurStatus  ovalStatus, bool isBlur, bool isMosaic, int radius)
{
    if (ovalStatus  == Hovered || ((isBlur || isMosaic) && index == m_selectedIndex)) {
        painter.setPen(QColor("#01bdff"));
    } else if (ovalStatus == Drawing || ((isBlur | isMosaic) && index != m_selectedIndex)) {
        painter.setPen(Qt::transparent);
    }

    FourPoints minorPoints = getAnotherFPoints(ellipseFPoints);
    QList<QPointF> eightControlPoints = getEightControlPoint(ellipseFPoints);
    QPainterPath ellipsePath;
    QPainterPath rectPath;
    //    qDebug() << "here" << ellipseFPoints[0].y() - ellipseFPoints[2].y();

    if (qAbs(ellipseFPoints[0].y() - ellipseFPoints[2].y()) <= 0.1
            && qAbs(ellipseFPoints[1].y() - ellipseFPoints[3].y()) <= 0.1) {
        QRect t_rect;
        t_rect.setX(static_cast<int>(ellipseFPoints[0].x()));
        t_rect.setY(static_cast<int>(ellipseFPoints[0].y()));
        t_rect.setWidth(static_cast<int>(ellipseFPoints[3].x() - ellipseFPoints[0].x()));
        t_rect.setHeight(static_cast<int>(ellipseFPoints[3].y() - ellipseFPoints[0].y()));

        ellipsePath.addEllipse(t_rect);
        painter.drawPath(ellipsePath);
    }

    else {
        ellipsePath.moveTo(minorPoints[0].x(), minorPoints[0].y());
        ellipsePath.cubicTo(eightControlPoints[0], eightControlPoints[1], minorPoints[1]);
        ellipsePath.cubicTo(eightControlPoints[4], eightControlPoints[5], minorPoints[2]);
        ellipsePath.cubicTo(eightControlPoints[6], eightControlPoints[7], minorPoints[3]);
        ellipsePath.cubicTo(eightControlPoints[3], eightControlPoints[2], minorPoints[0]);
        painter.drawPath(ellipsePath);
    }


    //    using namespace utils;
    if (isBlur) {
        painter.setClipPath(ellipsePath);
        painter.drawPixmap(0, 0,  width(), height(),  TempFile::instance()->getBlurPixmap(radius));
        painter.drawPath(ellipsePath);
    }
    if (isMosaic) {
        painter.setClipPath(ellipsePath);
        painter.drawPixmap(0, 0,  width(), height(),  TempFile::instance()->getMosaicPixmap(radius));
        painter.drawPath(ellipsePath);
    }
    painter.setClipping(false);
}

void ShapesWidget::paintArrow(QPainter &painter, QList<QPointF> lineFPoints,
                              int lineWidth, bool isStraight)
{
    if (lineFPoints.length() == 2) {
        if (!isStraight) {
            QList<QPointF> arrowPoints = pointOfArrow(lineFPoints[0],
                                                      lineFPoints[1], 8 + (lineWidth - 1) * 2);
            QPainterPath path;
            const QPen oldPen = painter.pen();
            if (arrowPoints.length() >= 3) {
                painter.drawLine(lineFPoints[0], lineFPoints[1]);
                path.moveTo(arrowPoints[2].x(), arrowPoints[2].y());
                path.lineTo(arrowPoints[0].x(), arrowPoints[0].y());
                path.lineTo(arrowPoints[1].x(), arrowPoints[1].y());
                path.lineTo(arrowPoints[2].x(), arrowPoints[2].y());
            }
            painter.setPen(Qt :: NoPen);
            painter.fillPath(path, QBrush(oldPen.color()));
        } else {
            painter.drawLine(lineFPoints[0], lineFPoints[1]);
        }
    }
}

void ShapesWidget::paintLine(QPainter &painter, QList<QPointF> lineFPoints)
{
    QPainterPath linePaths;
    if (lineFPoints.length() >= 1)
        linePaths.moveTo(lineFPoints[0]);
    else
        return;

    for (int k = 1; k < lineFPoints.length() - 2; k++) {
        linePaths.quadTo(lineFPoints[k], lineFPoints[k + 1]);
    }
    painter.drawPath(linePaths);
}

void ShapesWidget::paintEffectLine(QPainter &painter, QList<QPointF> lineFPoints, bool isMosaic, int radius, int lineWidth)
{
    QPainterPath linePaths;
    if (lineFPoints.length() >= 1)
        linePaths.moveTo(lineFPoints[0]);
    else
        return;

    for (int k = 1; k < lineFPoints.length() - 2; k++) {
        FourPoints rectFoints = getRectPoints(lineFPoints[k - 1], lineFPoints[k + 1], lineWidth);
        paintRect(painter, rectFoints, 0, Drawing, isMosaic, !isMosaic, radius);
        //paintRect(painter, rectFoints, 0, Hovered, false, false, radius);
    }
}

void ShapesWidget::paintText(QPainter &painter, FourPoints rectFPoints)
{
    QPen textPen;
    textPen.setStyle(Qt::DashLine);
    textPen.setColor("#01bdff");
    painter.setPen(textPen);

    if (rectFPoints.length() >= 4) {
        painter.drawLine(rectFPoints[0], rectFPoints[1]);
        painter.drawLine(rectFPoints[1], rectFPoints[3]);
        painter.drawLine(rectFPoints[3], rectFPoints[2]);
        painter.drawLine(rectFPoints[2], rectFPoints[0]);
    }
}

void ShapesWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    handlePaint(painter);
    if (m_shapes.length() > 0) {
        emit setShapesUndo(true);
    }
}

void ShapesWidget::handlePaint(QPainter &painter)
{
    painter.setRenderHints(QPainter::Antialiasing);
    QPen pen;
    //绘制所有图形
    for (int i = 0; i < m_shapes.length(); i++) {
        pen.setColor(BaseUtils::colorIndexOf(m_shapes[i].colorIndex));
        pen.setWidthF(m_shapes[i].lineWidth - 0.5);

        if (m_shapes[i].type == "rectangle") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintRect(painter, m_shapes[i].mainPoints, m_shapes.length(), Normal, false, false);
        } else if (m_shapes[i].type == "oval") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintEllipse(painter, m_shapes[i].mainPoints, m_shapes.length(), Normal, false, false);
        } else if (m_shapes[i].type == "effect") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            if (m_shapes[i].isOval == 0) {
                paintEllipse(painter, m_shapes[i].mainPoints, i, Drawing, m_shapes[i].isBlur, !m_shapes[i].isBlur, m_shapes[i].radius);
            } else if (m_shapes[i].isOval == 1) {
                paintRect(painter, m_shapes[i].mainPoints, i, Drawing, m_shapes[i].isBlur, !m_shapes[i].isBlur, m_shapes[i].radius);
            } else {
                pen.setJoinStyle(Qt::RoundJoin);
                painter.setPen(pen);
                paintEffectLine(painter, m_shapes[i].points, m_shapes[i].isBlur, m_shapes[i].radius, m_shapes[i].lineWidth);
            }
        } else if (m_shapes[i].type == "arrow") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintArrow(painter, m_shapes[i].points, pen.width(), false);
        } else if (m_shapes[i].type == "line") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintArrow(painter, m_shapes[i].points, pen.width(), true);
        } else if (m_shapes[i].type == "pen") {
            pen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pen);
            paintLine(painter, m_shapes[i].points);
        } else if (m_shapes[i].type == "text" && !m_clearAllTextBorder) {
            //            qDebug() << "*&^" << m_shapes[i].type << m_shapes[i].index << m_selectedIndex << i;
            QMap<int, TextEdit *>::iterator m = m_editMap.begin();
            while (m != m_editMap.end()) {
                if (m.key() == m_shapes[i].index) {
                    if (!(m.value()->isReadOnly() && m_selectedIndex != i)) {
                        paintText(painter, m_shapes[i].mainPoints);
                    }
                    break;
                }
                ++m;
            }
        }
    }

    //绘制选中的图形
    if ((m_pos1 != QPointF(0, 0) && m_pos2 != QPointF(0, 0)) || m_currentShape.type == "text") {
        FourPoints currentFPoint =  getMainPoints(m_pos1, m_pos2, m_isShiftPressed);
        pen.setColor(BaseUtils::colorIndexOf(m_currentShape.colorIndex));
        pen.setWidthF(m_currentShape.lineWidth - 0.5);

        if (m_currentType == "rectangle" && m_currentShape.type != "text") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintRect(painter, currentFPoint, m_shapes.length(), Normal, false, false);
        } else if (m_currentType == "oval" && m_currentShape.type != "text") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintEllipse(painter, currentFPoint, m_shapes.length(), Normal, false, false);

        } else if (m_currentType == "effect") {
            if (m_currentShape.isOval == 0) {
                paintEllipse(painter, currentFPoint, m_shapes.length(), Drawing, m_currentShape.isBlur, !m_currentShape.isBlur, m_currentShape.radius);
            } else if (m_currentShape.isOval == 1) {
                paintRect(painter, currentFPoint, m_shapes.length(), Drawing, m_currentShape.isBlur, !m_currentShape.isBlur, m_currentShape.radius);
            } else {
                //qDebug() << m_selectedIndex << m_currentShape.isOval << m_currentShape.points.size() << m_currentShape.radius << m_currentShape.lineWidth;
                pen.setJoinStyle(Qt::RoundJoin);
                painter.setPen(pen);
                paintEffectLine(painter, m_currentShape.points, m_currentShape.isBlur, m_currentShape.radius, m_currentShape.lineWidth);
            }
        } else if (m_currentType == "arrow" && m_currentShape.type != "text") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintArrow(painter, m_currentShape.points, pen.width(), false);
        } else if (m_currentType == "line" && m_currentShape.type != "text") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintArrow(painter, m_currentShape.points, pen.width(), true);
        } else if (m_currentType == "pen" && m_currentShape.type != "text") {
            pen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pen);
            paintLine(painter, m_currentShape.points);
        } else if (m_currentType == "text" && !m_clearAllTextBorder) {
            if (m_editing) {
                paintText(painter, m_currentShape.mainPoints);
            }
        }

    }
    //绘制悬停状态的图形
    if ((m_hoveredShape.mainPoints[0] != QPointF(0, 0) ||  m_hoveredShape.points.length() != 0)
            && m_hoveredIndex != -1) {
        pen.setWidthF(0.5);
        pen.setColor("#01bdff");
        if (m_hoveredShape.type == "rectangle") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintRect(painter, m_hoveredShape.mainPoints, m_hoveredIndex,  Hovered, false, false);
        } else if (m_hoveredShape.type == "oval") {
            pen.setJoinStyle(Qt::MiterJoin);
            pen.setCapStyle(Qt::SquareCap);
            painter.setPen(pen);
            paintEllipse(painter, m_hoveredShape.mainPoints, m_hoveredIndex, Hovered, false, false);
        } else if (m_hoveredShape.type == "arrow") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintArrow(painter, m_hoveredShape.points, pen.width(), false);
        } else if (m_hoveredShape.type == "line") {
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);
            paintArrow(painter, m_hoveredShape.points, pen.width(), true);
        } else if (m_hoveredShape.type == "pen") {
            pen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pen);
            paintLine(painter, m_hoveredShape.points);
        } else if (m_hoveredShape.type == "effect" && m_hoveredShape.isOval == 2) {
            pen.setJoinStyle(Qt::RoundJoin);
            painter.setPen(pen);
            paintLine(painter, m_hoveredShape.points);
        } else {
            //        qDebug() << "hoveredShape type:" << m_hoveredShape.type;
        }
    }

    qreal ration =  this->devicePixelRatioF();
    QIcon icon(":/other/resize_handle_big.svg");
    QPixmap resizePointImg = icon.pixmap(QSize(RESIZEPOINT_WIDTH,
                                               RESIZEPOINT_WIDTH));
    resizePointImg.setDevicePixelRatio(ration);

    //只有当选中图形时m_selectedShape才会有内容
    if ((m_selectedShape.type == "arrow" ||  m_selectedShape.type == "line") && m_selectedShape.points.length() == 2) {
        paintImgPoint(painter, m_selectedShape.points[0], resizePointImg);
        paintImgPoint(painter, m_selectedShape.points[1], resizePointImg);
    } else if (m_selectedShape.type != "" && m_selectedShape.type != "text") {
        if (m_selectedShape.mainPoints[0] != QPointF(0, 0) || m_selectedShape.type == "arrow") {

            QPointF rotatePoint = getRotatePoint(m_selectedShape.mainPoints[0],
                                                 m_selectedShape.mainPoints[1],
                                                 m_selectedShape.mainPoints[2],
                                                 m_selectedShape.mainPoints[3]);

            if (m_selectedShape.type == "oval" || m_selectedShape.type == "pen" || (m_selectedShape.type == "effect" && m_selectedShape.isOval == 2)) {
                pen.setJoinStyle(Qt::MiterJoin);
                pen.setWidth(1);
                pen.setColor(QColor("#01bdff"));
                painter.setPen(pen);
                paintRect(painter,  m_selectedShape.mainPoints, -1);
            }

            QPixmap rotatePointImg;
            //            rotatePointImg = QIcon(":/resources/images/size/rotate.svg").pixmap(ROTATE_ICON_SIZE);
            rotatePointImg = QIcon(":/other/icon_rotate.svg").pixmap(ROTATE_ICON_SIZE);
            rotatePointImg.setDevicePixelRatio(this->devicePixelRatioF());
            paintImgPoint(painter, rotatePoint, rotatePointImg, false);

            //画出形状的主要骨架点
            for (int i = 0; i < m_selectedShape.mainPoints.length(); i ++) {

                paintImgPoint(painter, m_selectedShape.mainPoints[i], resizePointImg);
            }

            //画出形状的其他骨架点
            FourPoints anotherFPoints = getAnotherFPoints(m_selectedShape.mainPoints);
            for (int j = 0; j < anotherFPoints.length(); j++) {
                paintImgPoint(painter, anotherFPoints[j], resizePointImg);
            }
        }
    }
}

//将编辑的内容绘制到图片上
void ShapesWidget::paintImage(QImage &image)
{
    QPainter painter(&image);
    handlePaint(painter);
    //    backgroundImage.save("/home/uos/Desktop/temp1.png");
}

bool ShapesWidget::isExistsText()
{
    for (int i = 0; i < m_shapes.length(); i++) {
        if (m_shapes[i].type == "text") {
            return true;
        }
    }
    return  false;
}

void ShapesWidget::enterEvent(QEvent *e)
{
    Q_UNUSED(e);
    if (m_currentType == "pen") {
        qApp->setOverrideCursor(BaseUtils::setCursorShape("pen",  BaseUtils::colorIndex(m_penColor)));
    } else if (m_currentType == "effect") {
        int isOval = ConfigSettings::instance()->getValue("effect", "isOval").toInt();
        QCursor setCursorValue;
        if (isOval == 0) {
            setCursorValue = BaseUtils::setCursorShape("oval");
        } else if (isOval == 1) {
            setCursorValue = BaseUtils::setCursorShape("rectangle");
        } else {
            setCursorValue = BaseUtils::setCursorShape("pen", 0);
        }
        qApp->setOverrideCursor(setCursorValue);
    } else {
        qApp->setOverrideCursor(BaseUtils::setCursorShape(m_currentType));
    }
}

void ShapesWidget::pinchTriggered(QPinchGesture *pinch)
{
    if (-1 == m_selectedIndex || "text" == m_currentType)
        return;
    QPinchGesture::ChangeFlags changeFlags = pinch->changeFlags();
    if (changeFlags & QPinchGesture::RotationAngleChanged) {
        qreal rotationDelta = (pinch->rotationAngle() - pinch->lastRotationAngle()) * 0.01;
        qreal centerX = (m_shapes[m_selectedOrder].mainPoints[0].x() + m_shapes[m_selectedOrder].mainPoints[3].x()) / 2;
        qreal centerY = (m_shapes[m_selectedOrder].mainPoints[0].y() + m_shapes[m_selectedOrder].mainPoints[3].y()) / 2;
        for (int i = 0; i < m_shapes[m_selectedOrder].mainPoints.size(); ++i) {
            qreal x = centerX + (m_shapes[m_selectedOrder].mainPoints[i].x() - centerX) * cos(rotationDelta) - (m_shapes[m_selectedOrder].mainPoints[i].y() - centerY) * sin(rotationDelta);
            qreal y = centerY + (m_shapes[m_selectedOrder].mainPoints[i].x() - centerX) * sin(rotationDelta) + (m_shapes[m_selectedOrder].mainPoints[i].y() - centerY) * cos(rotationDelta);
            //图形
            m_shapes[m_selectedOrder].mainPoints[i].setX(x);
            m_shapes[m_selectedOrder].mainPoints[i].setY(y);
        }
        qreal centerMainX = (m_selectedShape.mainPoints[0].x() + m_selectedShape.mainPoints[3].x()) / 2;
        qreal centerMainY = (m_selectedShape.mainPoints[0].y() + m_selectedShape.mainPoints[3].y()) / 2;
        for (int i = 0; i < m_selectedShape.mainPoints.length(); i ++) {
            qreal x = centerMainX + (m_selectedShape.mainPoints[i].x() - centerMainX) * cos(rotationDelta) - (m_selectedShape.mainPoints[i].y() - centerMainY) * sin(rotationDelta);
            qreal y = centerMainY + (m_selectedShape.mainPoints[i].x() - centerMainX) * sin(rotationDelta) + (m_selectedShape.mainPoints[i].y() - centerMainY) * cos(rotationDelta);
            //选中状态，点+线条
            m_selectedShape.mainPoints[i].setX(x);
            m_selectedShape.mainPoints[i].setY(y);
        }
        for (int k = 0; k < m_selectedShape.points.length(); k++) {
            qreal x = centerMainX + (m_shapes[m_selectedOrder].points[k].x() - centerMainX) * cos(rotationDelta) - (m_shapes[m_selectedOrder].points[k].y() - centerMainY) * sin(rotationDelta);
            qreal y = centerMainY + (m_shapes[m_selectedOrder].points[k].x() - centerMainX) * sin(rotationDelta) + (m_shapes[m_selectedOrder].points[k].y() - centerMainY) * cos(rotationDelta);
            //线条
            m_shapes[m_selectedOrder].points[k].setX(x);
            m_shapes[m_selectedOrder].points[k].setY(y);
        }
    }
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        qreal scale = pinch->scaleFactor();
        qreal ascanle = (scale - 1) * 0.3 + 1;
        qreal centerX = (m_shapes[m_selectedOrder].mainPoints[0].x() + m_shapes[m_selectedOrder].mainPoints[3].x()) / 2;
        qreal centerY = (m_shapes[m_selectedOrder].mainPoints[0].y() + m_shapes[m_selectedOrder].mainPoints[3].y()) / 2;
        for (int i = 0; i < m_shapes[m_selectedOrder].mainPoints.size(); ++i) {
            qreal x = m_shapes[m_selectedOrder].mainPoints[i].x() * ascanle + centerX * (1 - ascanle);
            qreal y = m_shapes[m_selectedOrder].mainPoints[i].y() * ascanle + centerY * (1 - ascanle);
            m_shapes[m_selectedOrder].mainPoints[i].setX(x);
            m_shapes[m_selectedOrder].mainPoints[i].setY(y);
        }
        qreal centerMainX = (m_selectedShape.mainPoints[0].x() + m_selectedShape.mainPoints[3].x()) / 2;
        qreal centerMainY = (m_selectedShape.mainPoints[0].y() + m_selectedShape.mainPoints[3].y()) / 2;
        for (int i = 0; i < m_selectedShape.mainPoints.length(); i ++) {
            qreal x = m_selectedShape.mainPoints[i].x() * ascanle + centerMainX * (1 - ascanle);
            qreal y = m_selectedShape.mainPoints[i].y() * ascanle + centerMainY * (1 - ascanle);
            m_selectedShape.mainPoints[i].setX(x);
            m_selectedShape.mainPoints[i].setY(y);
        }
        for (int k = 0; k < m_selectedShape.points.length(); k++) {
            qreal x = m_shapes[m_selectedOrder].points[k].x() * ascanle + centerMainX * (1 - ascanle);
            qreal y = m_shapes[m_selectedOrder].points[k].y() * ascanle + centerMainY * (1 - ascanle);
            m_shapes[m_selectedOrder].points[k].setX(x);
            m_shapes[m_selectedOrder].points[k].setY(y);
        }
    }
}

void ShapesWidget::tapTriggered(QTapGesture *tap)
{
    if (tap->state() == Qt::GestureState::GestureFinished && !clickedOnShapes(m_movingPoint) && -1 != m_selectedIndex && "text" != m_currentType) {
        m_isPressed = false;
        m_isMoving = false;
        clearSelected();
        setAllTextEditReadOnly();
        m_editing = false;
        m_selectedIndex = -1;
        m_selectedOrder = -1;
        m_selectedShape.type = "";
    }
}

void ShapesWidget::deleteCurrentShape()
{
    qDebug() << "delete shape";
    if (m_selectedOrder < m_shapes.length()) {
        m_shapes.removeAt(m_selectedOrder);
    } else {
        qWarning() << "Invalid index";
    }

    if (m_selectedShape.type == "text" && m_editMap.contains(m_selectedShape.index)) {
        m_editMap.value(m_selectedShape.index)->clear();
        m_editMap.remove(m_selectedShape.index);
    }

    clearSelected();
    m_selectedShape.type = "";
    m_currentShape.type = "";
    for (int i = 0; i < m_currentShape.mainPoints.length(); i++) {
        m_currentShape.mainPoints[i] = QPointF(0, 0);
    }

    if (m_shapes.length() == 0) {
        emit setShapesUndo(false);
    }

    update();
    m_selectedIndex = -1;
    m_selectedOrder = -1;
}

void ShapesWidget::undoDrawShapes()
{
    textEditIsReadOnly();
    qDebug() << "undoDrawShapes m_selectedIndex:" << m_selectedIndex << m_shapes.length();
    if (m_selectedOrder < m_shapes.length() && m_selectedIndex != -1) {
        deleteCurrentShape();
    } else if (m_shapes.length() > 0) {
        int tmpIndex = m_shapes[m_shapes.length() - 1].index;
        if (m_shapes[m_shapes.length() - 1].type == "text" && m_editMap.contains(tmpIndex)) {
            m_editMap.value(tmpIndex)->clear();
            delete m_editMap.value(tmpIndex);
            m_editMap.remove(tmpIndex);
        }

        m_shapes.removeLast();
    }
    qDebug() << "undoDrawShapes m_selectedIndex:" << m_selectedIndex << m_shapes.length();

    if (m_shapes.length() == 0) {
        emit setShapesUndo(false);
    }
    m_isSelected = false;
    clearSelected();
    update();
}
void ShapesWidget::undoAllDrawShapes()
{
    qDebug() << "undoAllDrawShapes undoDrawShapes m_selectedIndex:" << m_selectedIndex << m_shapes.length();
    if (m_selectedOrder < m_shapes.length() && m_selectedIndex != -1) {
        deleteCurrentShape();
    } else if (m_shapes.length() > 0) {
        while (m_shapes.length() > 0) {
            int tmpIndex = m_shapes[m_shapes.length() - 1].index;
            if (m_shapes[m_shapes.length() - 1].type == "text" && m_editMap.contains(tmpIndex)) {
                m_editMap.value(tmpIndex)->clear();
                delete m_editMap.value(tmpIndex);
                m_editMap.remove(tmpIndex);
            }

            m_shapes.removeLast();
        }
    }
    qDebug() << "undoDrawShapes m_selectedIndex:" << m_selectedIndex << m_shapes.length();

    if (m_shapes.length() == 0) {
        emit setShapesUndo(false);
    }
    m_isSelected = false;
    clearSelected();
    update();
}

void ShapesWidget::isInUndoBtn(bool isUndo)
{
    //qDebug() << ">>>>>> isInUndoBtn"  << isUndo;
    m_isUnDo = isUndo;
}

void ShapesWidget::microAdjust(QString direction)
{
    if (m_selectedIndex != -1 && m_selectedOrder < m_shapes.length()) {
        if (m_shapes[m_selectedOrder].type  == "text") {
            return;
        }

        if (direction == "Left" || direction == "Right" || direction == "Up" || direction == "Down") {
            m_shapes[m_selectedOrder].mainPoints = pointMoveMicro(m_shapes[m_selectedOrder].mainPoints, direction);
        } else if (direction == "Ctrl+Shift+Left" || direction == "Ctrl+Shift+Right" || direction == "Ctrl+Shift+Up"
                   || direction == "Ctrl+Shift+Down") {
            m_shapes[m_selectedOrder].mainPoints = pointResizeMicro(m_shapes[m_selectedOrder].mainPoints, direction, false);
        } else {
            m_shapes[m_selectedOrder].mainPoints = pointResizeMicro(m_shapes[m_selectedOrder].mainPoints, direction, true);
        }

        if (m_shapes[m_selectedOrder].type == "line" || m_shapes[m_selectedOrder].type == "arrow") {
            if (m_shapes[m_selectedOrder].portion.length() == 0) {
                for (int k = 0; k < m_shapes[m_selectedOrder].points.length(); k++) {
                    m_shapes[m_selectedOrder].portion.append(relativePosition(m_shapes[m_selectedOrder].mainPoints,
                                                                              m_shapes[m_selectedOrder].points[k]));
                }
            }
            for (int j = 0; j < m_shapes[m_selectedOrder].points.length(); j++) {
                m_shapes[m_selectedOrder].points[j] = getNewPosition(
                                                          m_shapes[m_selectedOrder].mainPoints, m_shapes[m_selectedOrder].portion[j]);
            }
        }

        m_selectedShape.mainPoints = m_shapes[m_selectedOrder].mainPoints;
        m_selectedShape.points = m_shapes[m_selectedOrder].points;
        m_hoveredShape.type = "";
        update();
    }
}

void ShapesWidget::setShiftKeyPressed(bool isShift)
{
    m_isShiftPressed = isShift;
}

void ShapesWidget::updateCursorShape()
{
//    qDebug() << "func" << __FUNCTION__ << "line" << __LINE__;
    if (!m_isUnDo) {
        QCursor setCursorValue;
        if (m_currentType == "pen") {
            m_penColor = BaseUtils::colorIndexOf(ConfigSettings::instance()->getValue("pen", "color_index").toInt());
            setCursorValue = BaseUtils::setCursorShape(m_currentType, BaseUtils::colorIndex(m_penColor));
        } else if (m_currentType == "effect") {
            int isOval = ConfigSettings::instance()->getValue("effect", "isOval").toInt();
            if (isOval == 0) {
                setCursorValue = BaseUtils::setCursorShape("oval");
            } else if (isOval == 1) {
                setCursorValue = BaseUtils::setCursorShape("rectangle");
            } else {
                setCursorValue = BaseUtils::setCursorShape("pen", 0);
            }
        } else {
            setCursorValue = BaseUtils::setCursorShape(m_currentType);
        }
        // 避免相同的光标样式重复设置
        if (*qApp->overrideCursor() != setCursorValue) {
            qApp->changeOverrideCursor(setCursorValue);
        }
    }
}

void ShapesWidget::menuSaveSlot()
{
    emit saveFromMenu();
}

void ShapesWidget::menuCloseSlot()
{
    emit closeFromMenu();
}
void ShapesWidget::setGlobalRect(QRect rect)
{
    m_globalRect = rect;
}


