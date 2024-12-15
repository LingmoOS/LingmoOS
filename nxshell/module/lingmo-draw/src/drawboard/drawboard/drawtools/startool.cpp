// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "startool.h"
#include "pagescene.h"
#include "staritem.h"
#include "pageview.h"

#include <QToolButton>
#include <QtMath>

StarTool::StarTool(QObject *parent)
    : DrawItemTool(parent)
{
    QPixmap s_cur = QPixmap(":/cursorIcons/star_mouse.svg");
    cursorScale(s_cur);
    setCursor(QCursor(s_cur));

    auto m_starBtn = toolButton();
    m_starBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_F)));
    setWgtAccesibleName(m_starBtn, "Star tool button");
    m_starBtn->setToolTip(tr("Star (F)"));
    m_starBtn->setIconSize(TOOL_ICON_RECT);
    m_starBtn->setFixedSize(TOOL_BUTTON_RECT);
    m_starBtn->setCheckable(true);
    m_starBtn->setIcon(QIcon::fromTheme("star_normal"));
    connect(m_starBtn, &QToolButton::toggled, m_starBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("star_normal");
        QIcon activeIcon = QIcon::fromTheme("star_highlight");
        m_starBtn->setIcon(b ? activeIcon : icon);
    });
}

int StarTool::toolType() const
{
    return polygonalStar;
}

StarTool::~StarTool()
{

}

SAttrisList StarTool::attributions()
{
    SAttrisList result;
    result << defaultAttriVar(EBrushColor)
           << defaultAttriVar(EEnableBrushStyle)
           << defaultAttriVar(EPenColor)
           << defaultAttriVar(EEnablePenStyle)
           << defaultAttriVar(EPenWidth)
           << defaultAttriVar(EStartLineSep)
           << defaultAttriVar(EStarAnchor)
           << defaultAttriVar(ERotProperty)
           //<< defaultAttriVar(EGroupWgt)
           //<< defaultAttriVar(EOrderProperty)
           << defaultAttriVar(EStarInnerOuterRadio);
    return result;
}

void StarTool::drawItemUpdate(ToolSceneEvent *event, PageItem *p)
{
    StarItem *pItem = dynamic_cast<StarItem *>(p);
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

PageItem *StarTool::drawItemStart(ToolSceneEvent *event)
{
    if (event->isNormalPressed()) {
        StarItem *m_pItem = new StarItem;
        return m_pItem;
    }
    return nullptr;
}

