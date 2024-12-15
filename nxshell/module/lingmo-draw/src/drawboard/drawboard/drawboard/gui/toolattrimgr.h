// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLATTRIMGR_H
#define TOOLATTRIMGR_H
#include "drawboardattrimgr.h"
#include <QVBoxLayout>
#ifdef USE_DTK
#include <DArrowRectangle>
DWIDGET_USE_NAMESPACE
#else
#include <QWidget>
#endif


#include <QPainterPath>


class DrawBoard;
class DrawBoardToolMgr;
class QAbstractButton;


class DRAWLIB_EXPORT ToolAttriMgr : public
#ifdef USE_DTK
    DArrowRectangle
#else
    QWidget
#endif
    , public AttributionManager
{
    Q_OBJECT
public:

    enum ArrowDirection {
        NoArrow,
        ArrowLeft,
        ArrowRight,
        ArrowTop,
        ArrowBottom
    };

public:
    ToolAttriMgr(QWidget *parent = nullptr);

    void showAt(int active, const QPoint &pos) override;
    void showWidgets(int active, const QList<QWidget *> &oldWidgets, const QList<QWidget *> &exposeWidgets) override;
    void resizeEvent(QResizeEvent *event) override;
#ifndef USE_DTK
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
#endif
#ifndef USE_DTK
private:
    QPainterPath getDrawPath(ArrowDirection direction);
    QRect  getValidRect();
    void getDrawRectPointsAndArcRect(QRect rect, QVector<QPoint> &points, QVector<QRectF> &rects);
    void drawCubicToPath(QPainterPath &path, QPoint p1, QPoint p2, QPoint endPoint);
#endif
private:
    QVBoxLayout *m_vLayout;
    int arrowOffset = 20;
    QPoint m_pos;
#ifndef USE_DTK
    QPoint      currentWindowPos;
    QPoint      wantArrowPosInGlobel;

    QAbstractButton *m_curBtn = nullptr;
    QPainterPath     m_paintPath;
    ArrowDirection   m_curArrowDirention;
    bool             m_bArcArrow = true;

    int arrowLenth = 20;
    int margin = 6;
    int radius = 15;
    int arrowWidth = 45;
    int arrowHeight = 15;
    int buttonHeight = 36;
#else
    int margin = 30;
#endif

};

#endif // TOOLATTRIMGR_H
