// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "triangletool.h"
#include "pagescene.h"
#include "triangleitem.h"
#include "pageview.h"

#include <QToolButton>
#include <QtMath>

TriangleTool::TriangleTool(QObject *parent)
    : DrawItemTool(parent)
{
    QPixmap s_cur = QPixmap(":/cursorIcons/triangle_mouse.svg");
    cursorScale(s_cur);
    setCursor(QCursor(s_cur));

    auto m_triangleBtn = toolButton();
    m_triangleBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_S)));
    setWgtAccesibleName(m_triangleBtn, "Triangle tool button");
    m_triangleBtn->setToolTip(tr("Triangle (S)"));
    m_triangleBtn->setIconSize(TOOL_ICON_RECT);
    m_triangleBtn->setFixedSize(TOOL_BUTTON_RECT);
    m_triangleBtn->setCheckable(true);

    m_triangleBtn->setIcon(QIcon::fromTheme("triangles_normal"));
    connect(m_triangleBtn, &QToolButton::toggled, m_triangleBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("triangles_normal");
        QIcon activeIcon = QIcon::fromTheme("triangles_highlight");
        m_triangleBtn->setIcon(b ? activeIcon : icon);
    });
}

TriangleTool::~TriangleTool()
{

}

int TriangleTool::toolType() const
{
    return triangle;
}

SAttrisList TriangleTool::attributions()
{
    SAttrisList result;
    result << defaultAttriVar(EBrushColor)
           << defaultAttriVar(EEnableBrushStyle)
           << defaultAttriVar(EPenColor)
           << defaultAttriVar(EEnablePenStyle)
           << defaultAttriVar(EBorderWidth)
           << defaultAttriVar(EPenWidth)
           << defaultAttriVar(ERotProperty)
           //<< defaultAttriVar(EGroupWgt)
           //<< defaultAttriVar(EOrderProperty)
           << SAttri(EStyleProper, QVariant());
    return result;
}

void TriangleTool::drawItemUpdate(ToolSceneEvent *event, PageItem *p)
{
    TriangleItem *pItem = dynamic_cast<TriangleItem *>(p);
    if (nullptr != pItem) {
        auto startPos = event->firstEvent()->currentLayerPos();
        QPointF pointMouse = event->currentLayerPos();
        bool shiftKeyPress = event->keyboardModifiers() & Qt::ShiftModifier;
        bool altKeyPress = event->keyboardModifiers() & Qt::AltModifier;
        QRectF resultRect;
        //按下SHIFT键
        if (shiftKeyPress && !altKeyPress) {
            QPointF resultPoint = pointMouse;
            qreal w = resultPoint.x() - startPos.x();
            qreal h = resultPoint.y() - startPos.y();
            qreal abslength = abs(w) - abs(h);
            if (abslength >= 0.1) {
                if (h >= 0) {
                    resultPoint.setY(startPos.y() + abs(w));
                } else {
                    resultPoint.setY(startPos.y() - abs(w));
                }
            } else {
                if (w >= 0) {
                    resultPoint.setX(startPos.x() + abs(h));
                } else {
                    resultPoint.setX(startPos.x() - abs(h));
                }
            }
            QRectF rectF(startPos, resultPoint);
            resultRect = rectF.normalized();
        }
        //按下ALT键
        else if (!shiftKeyPress && altKeyPress) {
            QPointF point1 = pointMouse;
            QPointF centerPoint = startPos;
            QPointF point2 = 2 * centerPoint - point1;
            QRectF rectF(point1, point2);
            resultRect = rectF.normalized();
        }
        //ALT SHIFT都按下
        else if (shiftKeyPress && altKeyPress) {
            QPointF resultPoint = pointMouse;
            qreal w = resultPoint.x() - startPos.x();
            qreal h = resultPoint.y() - startPos.y();
            qreal abslength = abs(w) - abs(h);
            if (abslength >= 0.1) {
                if (h >= 0) {
                    resultPoint.setY(startPos.y() + abs(w));
                } else {
                    resultPoint.setY(startPos.y() - abs(w));
                }
            } else {
                if (w >= 0) {
                    resultPoint.setX(startPos.x() + abs(h));
                } else {
                    resultPoint.setX(startPos.x() - abs(h));
                }
            }

            QPointF point1 = resultPoint;
            QPointF centerPoint = startPos;
            QPointF point2 = 2 * centerPoint - point1;
            QRectF rectF(point1, point2);
            resultRect = rectF.normalized();
        }
        //都没按下
        else {
            QPointF resultPoint = pointMouse;
            QRectF rectF(startPos, resultPoint);
            resultRect = rectF.normalized();
        }
        pItem->setRect(resultRect);
    }
}

PageItem *TriangleTool::drawItemStart(ToolSceneEvent *event)
{
    if (event->isNormalPressed()) {
        TriangleItem *pItem =  new TriangleItem(event->currentLayerPos().x(), event->currentLayerPos().y(), 0, 0);
        return pItem;
    }
    return nullptr;
}

