// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cpolygontool.h"
#include "cdrawscene.h"
#include "cgraphicspolygonitem.h"

#include "frame/cgraphicsview.h"
#include "cattributeitemwidget.h"
#include "cattributemanagerwgt.h"
#include "seperatorline.h"
#include "cspinbox.h"
#include "application.h"

#include <QtMath>
#include <DToolButton>

CPolygonTool::CPolygonTool()
    : IDrawTool(polygon)
{

}

CPolygonTool::~CPolygonTool()
{

}
DrawAttribution::SAttrisList CPolygonTool::attributions()
{
    DrawAttribution::SAttrisList result;
    result << defaultAttriVar(DrawAttribution::EBrushColor)
           << defaultAttriVar(DrawAttribution::EPenColor)
           << defaultAttriVar(DrawAttribution::EBorderWidth)
           << defaultAttriVar(EPolygonLineSep)
           << defaultAttriVar(DrawAttribution::EPolygonSides);
    return result;
}



QCursor CPolygonTool::cursor() const
{
    static QPixmap s_cur = QPixmap(":/cursorIcons/pengaton_mouse.svg");
    return QCursor(s_cur);
}

QAbstractButton *CPolygonTool::initToolButton()
{
    DToolButton *m_polygonBtn = new DToolButton;
    m_polygonBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_H)));
    setWgtAccesibleName(m_polygonBtn, "Polygon tool button");
    m_polygonBtn->setToolTip(tr("Polygon (H)"));
    m_polygonBtn->setIconSize(QSize(48, 48));
    m_polygonBtn->setFixedSize(QSize(37, 37));
    m_polygonBtn->setCheckable(true);

    connect(m_polygonBtn, &DToolButton::toggled, m_polygonBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_hexagon tool_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_hexagon tool_active");
        m_polygonBtn->setIcon(b ? activeIcon : icon);
    });
    m_polygonBtn->setIcon(QIcon::fromTheme("ddc_hexagon tool_normal"));
    return m_polygonBtn;
}

void CPolygonTool::registerAttributionWidgets()
{
    //7.注册多边形边数设置控件
    auto polygonSides = new CSpinBoxSettingWgt(tr("Sides"));
    polygonSides->setAttribution(EPolygonSides);
    polygonSides->spinBox()->setSpinRange(4, 10);
    setWgtAccesibleName(polygonSides->spinBox(), "Polgon edges spinbox");
    drawBoard()->attributionWidget()->installComAttributeWgt(EPolygonSides, polygonSides, 5);


    //注册分隔符
    auto spl = new SeperatorLine();
    drawBoard()->attributionWidget()->installComAttributeWgt(EPolygonLineSep, spl);
}

void CPolygonTool::toolCreatItemUpdate(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    if (pInfo != nullptr) {
        CGraphicsPolygonItem *pItem = dynamic_cast<CGraphicsPolygonItem *>(pInfo->businessItem);
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

void CPolygonTool::toolCreatItemFinish(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    if (pInfo != nullptr) {
        CGraphicsPolygonItem *m_pItem = dynamic_cast<CGraphicsPolygonItem *>(pInfo->businessItem);
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

CGraphicsItem *CPolygonTool::creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)
    if ((event->eventType() == CDrawToolEvent::EMouseEvent && event->mouseButtons() == Qt::LeftButton)
            || event->eventType() == CDrawToolEvent::ETouchEvent) {

        CGraphicsPolygonItem *m_pItem =  new CGraphicsPolygonItem(5, event->pos().x(), event->pos().y(), 0, 0);

        event->scene()->addCItem(m_pItem);
        return m_pItem;
    }
    return nullptr;
}


