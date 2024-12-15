// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ellipsetool.h"
#include "pagescene.h"
#include "ellipseitem.h"
#include "attributewidget.h"

#include "pageview.h"

#include <QtMath>
#include <QToolButton>

EllipseTool::EllipseTool(QObject *parent)
    : DrawItemTool(parent)
{
    QPixmap s_cur = QPixmap(":/cursorIcons/round_mouse.svg");
    cursorScale(s_cur);
    setCursor(QCursor(s_cur));

    auto m_roundBtn = toolButton();
    m_roundBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_O)));
    setWgtAccesibleName(m_roundBtn, "Ellipse tool button");
    m_roundBtn->setToolTip(tr("Ellipse (O)"));
    m_roundBtn->setIconSize(TOOL_ICON_RECT);
    m_roundBtn->setFixedSize(TOOL_BUTTON_RECT);
    m_roundBtn->setCheckable(true);
    m_roundBtn->setIcon(QIcon::fromTheme("circular_normal"));
    connect(m_roundBtn, &QToolButton::toggled, m_roundBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("circular_normal");
        QIcon activeIcon = QIcon::fromTheme("circular_highlight");
        m_roundBtn->setIcon(b ? activeIcon : icon);
    });
}

EllipseTool::~EllipseTool()
{

}

int EllipseTool::toolType() const
{
    return ellipse;
}

SAttrisList EllipseTool::attributions()
{
    SAttrisList result;
    result << defaultAttriVar(EBrushColor)
           << defaultAttriVar(EEnableBrushStyle)
           << defaultAttriVar(EPenColor)
           << defaultAttriVar(EEnablePenStyle)
           << defaultAttriVar(EPenWidth)
           << defaultAttriVar(ERotProperty)
           //<< defaultAttriVar(EGroupWgt)
           //<< defaultAttriVar(EOrderProperty)
           << SAttri(EStyleProper, QVariant());
    return result;
}

PageItem *EllipseTool::drawItemStart(ToolSceneEvent *eventpInfo)
{
    if (eventpInfo->isNormalPressed()) {
        EllipseItem *item =  new EllipseItem(eventpInfo->currentLayerPos().x(), eventpInfo->currentLayerPos().y(), 0, 0);
        return item;
    }
    return nullptr;
}

void EllipseTool::drawItemUpdate(ToolSceneEvent *event, PageItem *p)
{
    EllipseItem *pItem = dynamic_cast<EllipseItem *>(p);
    QPointF startPos = event->firstEvent()->currentLayerPos();
    if (nullptr != pItem) {
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

void EllipseTool::drawItemFinish(ToolSceneEvent *event, PageItem *pItem)
{
    EllipseItem *m_pItem = dynamic_cast<EllipseItem *>(pItem);
    if (nullptr != m_pItem) {
        if (m_pItem->scene() == nullptr) {
            m_pItem->pageScene()->addPageItem(m_pItem);
        }
        m_pItem->setSelected(true);
    }
}




