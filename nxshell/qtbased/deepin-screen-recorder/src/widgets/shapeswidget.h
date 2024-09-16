// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAPESWIDGET_H
#define SHAPESWIDGET_H

#include "../utils/shapesutils.h"
#include "../utils/baseutils.h"
#include "../widgets/textedit.h"
#include "../widgets/sidebar.h"
#include "../menucontroller/menucontroller.h"

#include <DFrame>
#include <QGestureEvent>
#include <QMouseEvent>

class ShapesWidget : public DFrame
{
    Q_OBJECT
public:
    explicit ShapesWidget(DWidget *parent = 0);
    ~ShapesWidget();

    enum ShapeBlurStatus {
        Drawing,
        Selected,
        Hovered,
        Normal,
    };
    enum ClickedKey {
        First,
        Second,
        Third,
        Fourth,
        Fifth,
        Sixth,
        Seventh,
        Eighth,
    };


signals:
    void reloadEffectImg(QString effect, int radius);
    void requestScreenshot();
    void shapePressed(QString shape);
    void saveBtnPressed(SaveAction action);
    void requestExit();
    void menuNoFocus();
    void saveFromMenu();
    void closeFromMenu();
    //选中某个形状后对应工具栏切换
    void shapeClicked(QString shape);
    void setShapesUndo(bool status);

public slots:
    /**
     * @brief updateSelectedShape: 更新选中的形状
     * @param group:
     * @param key:
     * @param index:
     */
    void updateSelectedShape(const QString &group, const QString &key, int index);
    void setCurrentShape(QString shapeType);
    //void updatePenColor();
    //void setPenColor(QColor color);
    /**
     * @brief 清除选择
     */
    void clearSelected();
    /**
     * @brief 设置所有的文本编辑框为只读状态
     */
    void setAllTextEditReadOnly();
    /**
     * @brief 设置移除没有输入的文本框
     */
    void setNoChangedTextEditRemove();
    void saveActionTriggered();

    void handleDrag(QPointF oldPoint, QPointF newPoint);
    void handleRotate(QPointF pos);
    void handleResize(QPointF pos, int key);

    /**
     * @brief clickedOnShapes 图形是否被点击，发射点击信号
     * @param pos
     * @return 被点击：true 未被点击：false
     */
    bool clickedOnShapes(QPointF pos);

    /**
     * @brief clickedOnRect: 矩形是否被点击
     * @param rectPoints: 矩形的四个点
     * @param pos: 当前鼠标的位置
     * @param isBlurMosaic: 是否有模糊效果或者马赛克效果
     * @return
     */
    bool clickedOnRect(FourPoints rectPoints, QPointF pos, bool isBlurMosaic = false);

    /**
     * @brief clickedOnEllipse: 圆形是否被点击
     * @param mainPoints: 圆形的主要点
     * @param pos: 当前鼠标的位置
     * @param isBlurMosaic: 是否有模糊效果或者马赛克效果
     * @return
     */
    bool clickedOnEllipse(FourPoints mainPoints, QPointF pos, bool isBlurMosaic = false);

    /**
     * @brief clickedOnArrow: 箭头是否被点击
     * @param points: 箭头的主要点
     * @param pos: 当前鼠标的位置
     * @return
     */
    bool clickedOnArrow(QList<QPointF> points, QPointF pos);

    /**
     * @brief clickedOnLine: 画出的线是否被点击
     * @param mainPoints: 画笔线的主要点
     * @param points:
     * @param pos: 当前鼠标的位置
     * @return
     */
    bool clickedOnLine(FourPoints mainPoints, QList<QPointF> points, QPointF pos);

    /**
     * @brief clickedOnText: 文本框是否被点击
     * @param mainPoints: 文本框的主要点
     * @param pos: 当前鼠标的位置
     * @return
     */
    bool clickedOnText(FourPoints mainPoints, QPointF pos);
    bool rotateOnPoint(FourPoints mainPoints, QPointF pos);

    bool hoverOnShapes(Toolshape toolShape, QPointF pos);
    bool hoverOnRect(FourPoints rectPoints, QPointF pos, bool isTextBorder = false);
    bool hoverOnEllipse(FourPoints mainPoints, QPointF pos);
    bool hoverOnArrow(QList<QPointF> points, QPointF pos);
    bool hoverOnLine(FourPoints mainPoints, QList<QPointF> points, QPointF pos);
    bool hoverOnText(int textIndex, FourPoints mainPoints, QPointF pos);

    bool hoverOnRotatePoint(FourPoints mainPoints, QPointF pos);
    bool textEditIsReadOnly();

    void undoDrawShapes();
    void undoAllDrawShapes();
    /**
     * @brief isInUndoBtn 光标是否在撤销按钮内？在离开撤销按钮之后，再更新光标样式
     */
    void isInUndoBtn(bool isUndo);
    void deleteCurrentShape();
    //QString  getCurrentType();
    void microAdjust(QString direction);
    void setShiftKeyPressed(bool isShift);
    void updateCursorShape();
    void menuSaveSlot();
    void menuCloseSlot();
    //void updateSideBarPosition();
    void setGlobalRect(QRect rect);

    /**
     * @brief paintImage: 绘制图片
     * 将编辑的内容绘制到图片上
     */
    void paintImage(QImage &image);
    /**
     * @brief isExistsText: 是否存在文字图形，
     * @return
     */
    bool isExistsText();
protected:
    bool event(QEvent *event);
    /**
     * @brief mousePressEvent: 重写鼠标按压事件
     * @param e
     */
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    /**
     * @brief 可通过w/a/s/d及方向键改变标注内容位置及大小
     * @param e
     */
    void keyPressEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent *);
    /**
     * @brief handlePaint:执行绘制操作
     * @param painter:画笔
     */
    void handlePaint(QPainter &painter);
    void enterEvent(QEvent *e);

    /**
     * @brief clickeShapes:只是用来判断是否选中图形,不是真实鼠标事件会触发（触摸屏）
     * @param pos:坐标
     * @return
     */
    bool clickedShapes(QPointF pos);
    /**
     * @brief pinchTriggered:捏合手势事件处理
     * @param pinch:捏合手势
     */
    void pinchTriggered(QPinchGesture *pinch);
    /**
     * @brief tapTriggered:单击手势事件处理
     * @param tap:单击手势
     */
    void tapTriggered(QTapGesture *tap);

private:
    QPointF m_pos1 = QPointF(0, 0);
    QPointF m_pos2 = QPointF(0, 0);
    QPointF m_pos3, m_pos4;

    /**
     * @brief m_pressedPoint: 当前鼠标按下的位置
     */
    QPointF m_pressedPoint;
    QPointF m_movingPoint;
    qreal   m_lastAngle;

    bool m_isRecording;
    bool m_isMoving;
    bool m_isSelected;
    bool m_isPressed;
    bool m_isHovered;
    bool m_isRotated;
    bool m_isArrowRotated;
    bool m_isResize;
    bool m_isShiftPressed;
    bool m_editing;

    /**
     * @brief m_isSelectedText: 是否选中文本框
     */
    bool m_isSelectedText;
    ResizeDirection m_resizeDirection;
    ClickedKey m_clickedKey;

    int m_shapesIndex;
    /**
     * @brief m_selectedIndex 被选中图形的索引号
     */
    int m_selectedIndex;
    int m_currentIndex;
    int m_hoveredIndex;
    int m_selectedOrder;
    //bool m_blurEffectExist = false;
    //bool m_mosaicEffectExist = false;
    /**
        * @brief m_lastEditMapKey 记录最后一个textedit在Map中的key值
        */
    int m_lastEditMapKey = -1;

    /**
     * @brief m_currentType: 当前的图形类型
     * 现阶段存在5种图形：
     * line: 通过画笔画出的线条
     * arrow: 箭头
     * rectangle: 矩形
     * oval: 圆形
     * text: 文本
     */
    QString m_currentType = "rectangle";
    QColor m_penColor;
    bool m_clearAllTextBorder = false;
    bool m_sideBarInit = false;

    /**
     * @brief m_currentShape:当前正在绘制的图形
     */
    Toolshape m_currentShape;
    /**
     * @brief m_selectedShape:当前选中的图形
     */
    Toolshape m_selectedShape;
    /**
     * @brief m_hoveredShape:当前悬停的图形
     */
    Toolshape m_hoveredShape;

    QMap<int, TextEdit *> m_editMap;
    void updateTextRect(TextEdit *edit, QRectF newRect);
    /**
     * @brief m_shapes:所有形状
     */
    Toolshapes m_shapes;
    MenuController *m_menuController;
    //SideBar *m_sideBar;

    QRect m_globalRect;
    /**
     * @brief 当前光标的位置
     */
    QPoint m_currentCursor;

    void paintImgPoint(QPainter &painter, QPointF pos, QPixmap img, bool isResize = true);
    //void paintImgPointArrow(QPainter &painter, QPointF pos, QPixmap img);
    void paintRect(QPainter &painter, FourPoints rectFPoints, int index,
                   ShapeBlurStatus  rectStatus = Normal, bool isBlur = false, bool isMosaic = false, int radius = 10);
    void paintEllipse(QPainter &painter, FourPoints ellipseFPoints, int index,
                      ShapeBlurStatus  ovalStatus = Normal, bool isBlur = false, bool isMosaic = false, int radius = 10);
    void paintArrow(QPainter &painter, QList<QPointF> lineFPoints,
                    int lineWidth, bool isStraight = false);
    void paintLine(QPainter &painter, QList<QPointF> lineFPoints);
    void paintEffectLine(QPainter &painter, QList<QPointF> lineFPoints, bool isMosaic, int radius, int lineWidth);
    void paintText(QPainter &painter, FourPoints rectFPoints);

    bool m_isUnDo = false;
};
#endif // SHAPESWIDGET_H
