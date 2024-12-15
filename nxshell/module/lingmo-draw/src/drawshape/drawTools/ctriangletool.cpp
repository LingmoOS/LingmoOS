// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ctriangletool.h"
#include "cdrawscene.h"
#include "cgraphicstriangleitem.h"
#include "frame/cgraphicsview.h"
#include "cattributeitemwidget.h"
#include "application.h"

#include <DToolButton>
#include <QtMath>

CTriangleTool::CTriangleTool()
    : IDrawTool(triangle)
{

}

CTriangleTool::~CTriangleTool()
{

}

DrawAttribution::SAttrisList CTriangleTool::attributions()
{
    DrawAttribution::SAttrisList result;
    result << defaultAttriVar(DrawAttribution::EBrushColor)
           << defaultAttriVar(DrawAttribution::EPenColor)
           << defaultAttriVar(DrawAttribution::EBorderWidth);
    return result;
}

QCursor CTriangleTool::cursor() const
{
    static QPixmap s_cur = QPixmap(":/cursorIcons/triangle_mouse.svg");
    return QCursor(s_cur);
}

QAbstractButton *CTriangleTool::initToolButton()
{
    DToolButton *m_triangleBtn = new DToolButton;
    m_triangleBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_S)));
    setWgtAccesibleName(m_triangleBtn, "Triangle tool button");
    m_triangleBtn->setToolTip(tr("Triangle (S)"));
    m_triangleBtn->setIconSize(QSize(48, 48));
    m_triangleBtn->setFixedSize(QSize(37, 37));
    m_triangleBtn->setCheckable(true);

    connect(m_triangleBtn, &DToolButton::toggled, m_triangleBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_triangle tool_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_triangle tool_active");
        m_triangleBtn->setIcon(b ? activeIcon : icon);
    });
    m_triangleBtn->setIcon(QIcon::fromTheme("ddc_triangle tool_normal"));
    return m_triangleBtn;
}

void CTriangleTool::toolCreatItemUpdate(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    if (pInfo != nullptr) {
        CGraphicsTriangleItem *pItem = dynamic_cast<CGraphicsTriangleItem *>(pInfo->businessItem);
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

void CTriangleTool::toolCreatItemFinish(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    if (pInfo != nullptr) {
        CGraphicsTriangleItem *m_pItem = dynamic_cast<CGraphicsTriangleItem *>(pInfo->businessItem);
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

CGraphicsItem *CTriangleTool::creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)
    if ((event->eventType() == CDrawToolEvent::EMouseEvent && event->mouseButtons() == Qt::LeftButton)
            || event->eventType() == CDrawToolEvent::ETouchEvent) {

        CGraphicsTriangleItem *pItem =  new CGraphicsTriangleItem(event->pos().x(), event->pos().y(), 1, 1);
//        PageView *pView = event->scene()->drawView();
//        pItem->setPen(pView->getDrawParam()->getPen());
//        pItem->setBrush(pView->getDrawParam()->getBrush());
        event->scene()->addCItem(pItem);
        return pItem;
    }
    return nullptr;
}

