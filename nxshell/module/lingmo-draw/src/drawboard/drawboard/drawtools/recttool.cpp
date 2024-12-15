// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recttool.h"
#include "pagescene.h"
#include "rectitem.h"
#include "pageview.h"
#include "cgraphicsitemevent.h"

#include <QToolButton>
#include <QtMath>

RectTool::RectTool(QObject *parent)
    : DrawItemTool(parent)
{
    QPixmap s_cur = QPixmap(":/cursorIcons/rectangle_mouse.svg");
    cursorScale(s_cur);
    setCursor(QCursor(s_cur));

    auto m_rectBtn = toolButton();
    m_rectBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_R)));
    setWgtAccesibleName(m_rectBtn, "Rectangle tool button");
    m_rectBtn->setToolTip(tr("Rectangle (R)"));
    m_rectBtn->setIconSize(TOOL_ICON_RECT);
    m_rectBtn->setFixedSize(TOOL_BUTTON_RECT);
    m_rectBtn->setCheckable(true);
    m_rectBtn->setIcon(QIcon::fromTheme("square_normal"));
    connect(m_rectBtn, &QToolButton::toggled, m_rectBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("square_normal");
        QIcon activeIcon = QIcon::fromTheme("square_highlight");
        m_rectBtn->setIcon(b ? activeIcon : icon);
    });

}

RectTool::~RectTool()
{

}

int RectTool::toolType() const
{
    return rectangle;
}

SAttrisList RectTool::attributions()
{
    SAttrisList result;
    result << defaultAttriVar(EBrushColor)
           << defaultAttriVar(EEnableBrushStyle)
           << defaultAttriVar(EPenColor)
           << defaultAttriVar(EEnablePenStyle)
           << defaultAttriVar(EPenWidth)
           << defaultAttriVar(ERectRadius)
           << defaultAttriVar(ERotProperty)
           << SAttri(EStyleProper, QVariant());

    return result;
}

void processItemsScal(ToolSceneEvent *event, EChangedPhase phase, PageItem *pItem)
{
    PageItemScalEvent scal(PageItemEvent::EScal);
    scal.setEventPhase(phase);
    scal.setToolEventType(0);
    scal.setPressedDirection(HandleNode::Resize_RB);
    scal._scenePos = event->pos();
    scal._oldScenePos = event->lastEvent()->pos();
    scal._sceneBeginPos = event->firstEvent()->pos();

    bool xBlock = false;
    bool yBlock = false;
    HandleNode::EInnerType dir = HandleNode::Resize_RB;
    HandleNode::getTransBlockFlag(dir, xBlock, yBlock);
    scal.setXTransBlocked(xBlock);
    scal.setYTransBlocked(yBlock);

    bool xNegitiveOffset = false;
    bool yNegitiveOffset = false;
    HandleNode::getTransNegtiveFlag(dir, xNegitiveOffset, yNegitiveOffset);
    scal.setXNegtiveOffset(xNegitiveOffset);
    scal.setYNegtiveOffset(yNegitiveOffset);
    scal.setKeepOrgRadio(event->keyboardModifiers() & Qt::ShiftModifier);

    //分发事件
    if (PageScene::isPageItem(pItem) /*|| pItem == event->scene()->selectGroup()*/) {
        PageItem *pBzItem = pItem;

        scal.setPos(pBzItem->mapFromScene(event->pos()));
        scal.setOldPos(pBzItem->mapFromScene(event->lastEvent()->pos()));
        scal.setOrgSize(pBzItem->orgRect().size());
        scal.setCenterPos((event->keyboardModifiers() & Qt::AltModifier) ? pBzItem->orgRect().center() :
                          HandleNode::transCenter(dir, pBzItem));
        scal._sceneCenterPos = pBzItem->mapToScene(scal.centerPos());

        if (phase == EChangedBegin) {
            pBzItem->operatingBegin(&scal);
        } else if (phase == EChangedUpdate) {
            pBzItem->operating(&scal);
        } else if (phase == EChangedFinished) {
            pBzItem->operatingEnd(&scal);
        }
    }
    event->view()->viewport()->update();
}
PageItem *RectTool::drawItemStart(ToolSceneEvent *event)
{
    if (event->isNormalPressed()) {
        RectItem *m_pRectItem = new RectItem(event->currentLayerPos().x(), event->currentLayerPos().y(), 1, 1);
        //processItemsScal(event, EChangedBegin, m_pRectItem);
        return m_pRectItem;
    }
    return nullptr;
}



void RectTool::drawItemUpdate(ToolSceneEvent *event, PageItem *p)
{
    RectItem *pRectItem = dynamic_cast<RectItem *>(p);

    //processItemsScal(event, EChangedUpdate, pRectItem);

    //return;
    if (nullptr != pRectItem) {
        QPointF startPos = event->firstEvent()->currentLayerPos();

        QPointF pointMouse = event->currentLayerPos();
        bool shiftKeyPress = event->keyboardModifiers() & Qt::ShiftModifier;
        bool altKeyPress = event->keyboardModifiers() & Qt::AltModifier;
        QRectF resultRect;

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
        } else {
            QPointF resultPoint = pointMouse;
            QRectF rectF(startPos, resultPoint);
            resultRect = rectF.normalized();
        }

        pRectItem->setRect(resultRect);
        event->setAccepted(true);
    }
}

void RectTool::drawItemFinish(ToolSceneEvent *event, PageItem *p)
{
    //RectItem *pRectItem = dynamic_cast<RectItem *>(p);

    //processItemsScal(event, EChangedFinished, pRectItem);
}
