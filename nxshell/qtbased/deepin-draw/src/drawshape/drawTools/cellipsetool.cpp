// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cellipsetool.h"
#include "cdrawscene.h"
#include "cgraphicsellipseitem.h"
#include "cattributeitemwidget.h"

#include "frame/cgraphicsview.h"
#include "application.h"

#include <QtMath>
#include <DToolButton>

CEllipseTool::CEllipseTool()
    : IDrawTool(ellipse)
{

}

CEllipseTool::~CEllipseTool()
{

}

DrawAttribution::SAttrisList CEllipseTool::attributions()
{
    DrawAttribution::SAttrisList result;
    result << defaultAttriVar(DrawAttribution::EBrushColor)
           << defaultAttriVar(DrawAttribution::EPenColor)
           << defaultAttriVar(DrawAttribution::EBorderWidth);
    return result;
}

QAbstractButton *CEllipseTool::initToolButton()
{
    DToolButton *m_roundBtn = new DToolButton;
    m_roundBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_O)));
    setWgtAccesibleName(m_roundBtn, "Ellipse tool button");
    m_roundBtn->setToolTip(tr("Ellipse (O)"));
    m_roundBtn->setIconSize(QSize(48, 48));
    m_roundBtn->setFixedSize(QSize(37, 37));
    m_roundBtn->setCheckable(true);
    connect(m_roundBtn, &DToolButton::toggled, m_roundBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_round tool_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_round tool_active");
        m_roundBtn->setIcon(b ? activeIcon : icon);
    });
    m_roundBtn->setIcon(QIcon::fromTheme("ddc_round tool_normal"));
    return m_roundBtn;
}

QCursor CEllipseTool::cursor() const
{
    static QPixmap s_cur = QPixmap(":/cursorIcons/round_mouse.svg");
    return QCursor(s_cur);
}

void CEllipseTool::toolCreatItemUpdate(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    if (pInfo != nullptr) {
        CGraphicsEllipseItem *pItem = dynamic_cast<CGraphicsEllipseItem *>(pInfo->businessItem);
        if (nullptr != pItem) {
            QPointF pointMouse = event->pos();
            bool shiftKeyPress = event->keyboardModifiers() & Qt::ShiftModifier;
            bool altKeyPress = event->keyboardModifiers() & Qt::AltModifier;
            QRectF resultRect;
            //按下SHIFT键
            if (shiftKeyPress && !altKeyPress) {
                QPointF resultPoint = pointMouse;
                qreal w = resultPoint.x() - pInfo->_startPos.x();
                qreal h = resultPoint.y() - pInfo->_startPos.y();
                qreal abslength = abs(w) - abs(h);
                if (abslength >= 0.1) {
                    if (h >= 0) {
                        resultPoint.setY(pInfo->_startPos.y() + abs(w));
                    } else {
                        resultPoint.setY(pInfo->_startPos.y() - abs(w));
                    }
                } else {
                    if (w >= 0) {
                        resultPoint.setX(pInfo->_startPos.x() + abs(h));
                    } else {
                        resultPoint.setX(pInfo->_startPos.x() - abs(h));
                    }
                }
                QRectF rectF(pInfo->_startPos, resultPoint);
                resultRect = rectF.normalized();
            }
            //按下ALT键
            else if (!shiftKeyPress && altKeyPress) {
                QPointF point1 = pointMouse;
                QPointF centerPoint = pInfo->_startPos;
                QPointF point2 = 2 * centerPoint - point1;
                QRectF rectF(point1, point2);
                resultRect = rectF.normalized();
            }
            //ALT SHIFT都按下
            else if (shiftKeyPress && altKeyPress) {
                QPointF resultPoint = pointMouse;
                qreal w = resultPoint.x() - pInfo->_startPos.x();
                qreal h = resultPoint.y() - pInfo->_startPos.y();
                qreal abslength = abs(w) - abs(h);
                if (abslength >= 0.1) {
                    if (h >= 0) {
                        resultPoint.setY(pInfo->_startPos.y() + abs(w));
                    } else {
                        resultPoint.setY(pInfo->_startPos.y() - abs(w));
                    }
                } else {
                    if (w >= 0) {
                        resultPoint.setX(pInfo->_startPos.x() + abs(h));
                    } else {
                        resultPoint.setX(pInfo->_startPos.x() - abs(h));
                    }
                }

                QPointF point1 = resultPoint;
                QPointF centerPoint = pInfo->_startPos;
                QPointF point2 = 2 * centerPoint - point1;
                QRectF rectF(point1, point2);
                resultRect = rectF.normalized();
            }
            //都没按下
            else {
                QPointF resultPoint = pointMouse;
                QRectF rectF(pInfo->_startPos, resultPoint);
                resultRect = rectF.normalized();
            }
            pItem->setRect(resultRect);
        }
    }
}

void CEllipseTool::toolCreatItemFinish(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    if (pInfo != nullptr) {
        CGraphicsEllipseItem *m_pItem = dynamic_cast<CGraphicsEllipseItem *>(pInfo->businessItem);
        if (nullptr != m_pItem) {
            if (!pInfo->hasMoved()) {
                event->scene()->removeCItem(m_pItem, true);
                pInfo->businessItem = nullptr;
            } else {
                if (m_pItem->scene() == nullptr) {
                    m_pItem->drawScene()->addCItem(m_pItem);
                }
                m_pItem->setSelected(true);
            }
        }
    }

    IDrawTool::toolCreatItemFinish(event, pInfo);
}

CGraphicsItem *CEllipseTool::creatItem(CDrawToolEvent *eventpInfo, ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)
    if ((eventpInfo->eventType() == CDrawToolEvent::EMouseEvent && eventpInfo->mouseButtons() == Qt::LeftButton)
            || eventpInfo->eventType() == CDrawToolEvent::ETouchEvent) {

        CGraphicsEllipseItem *m_pItem =  new CGraphicsEllipseItem(eventpInfo->pos().x(), eventpInfo->pos().y(), 0, 0);
        eventpInfo->scene()->addCItem(m_pItem);
        return m_pItem;
    }
    return nullptr;
}


