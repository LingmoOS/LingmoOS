// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cpolygonalstartool.h"
#include "cdrawscene.h"
#include "cgraphicspolygonalstaritem.h"
#include "frame/cgraphicsview.h"
#include "cattributeitemwidget.h"
#include "cattributemanagerwgt.h"
#include "cspinbox.h"
#include "seperatorline.h"
#include "application.h"

#include <DToolButton>


using namespace DrawAttribution;

#include <QtMath>

CPolygonalStarTool::CPolygonalStarTool()
    : IDrawTool(polygonalStar)
{

}

QAbstractButton *CPolygonalStarTool::initToolButton()
{
    DToolButton *m_starBtn = new DToolButton;
    m_starBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_F)));
    setWgtAccesibleName(m_starBtn, "Star tool button");
    m_starBtn->setToolTip(tr("Star (F)"));
    m_starBtn->setIconSize(QSize(48, 48));
    m_starBtn->setFixedSize(QSize(37, 37));
    m_starBtn->setCheckable(true);

    connect(m_starBtn, &DToolButton::toggled, m_starBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_star tool_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_star tool_active");
        m_starBtn->setIcon(b ? activeIcon : icon);
    });
    m_starBtn->setIcon(QIcon::fromTheme("ddc_star tool_normal"));
    return m_starBtn;
}

CPolygonalStarTool::~CPolygonalStarTool()
{

}

DrawAttribution::SAttrisList CPolygonalStarTool::attributions()
{
    DrawAttribution::SAttrisList result;
    result << defaultAttriVar(DrawAttribution::EBrushColor)
           << defaultAttriVar(DrawAttribution::EPenColor)
           << defaultAttriVar(DrawAttribution::EBorderWidth)
           << defaultAttriVar(EStartLineSep)
           << defaultAttriVar(DrawAttribution::EStarAnchor)
           << defaultAttriVar(DrawAttribution::EStarInnerOuterRadio);
    return result;
}



QCursor CPolygonalStarTool::cursor() const
{
    static QPixmap s_cur = QPixmap(":/cursorIcons/star_mouse.svg");
    return QCursor(s_cur);
}
void CPolygonalStarTool::registerAttributionWidgets()
{
    //5.注册星型点数设置控件
    auto starAnr = new CSpinBoxSettingWgt(tr("Points"));
    starAnr->setAttribution(EStarAnchor);
    starAnr->spinBox()->setSpinRange(3, 50);
    setWgtAccesibleName(starAnr->spinBox(), "Star Anchor spinbox");
    drawBoard()->attributionWidget()->installComAttributeWgt(EStarAnchor, starAnr, 5);

    //6.注册星型内外圆半径比例设置控件
    auto starRadio = new CSpinBoxSettingWgt(tr("Radius"));
    setWgtAccesibleName(starRadio->spinBox(), "Star inner radius spinbox");
    starRadio->setAttribution(EStarInnerOuterRadio);
    starRadio->spinBox()->setSpinRange(0, 100);
    starRadio->spinBox()->setSuffix("%");
    drawBoard()->attributionWidget()->installComAttributeWgt(EStarInnerOuterRadio, starRadio, 50);

    //注册分隔符
    auto spl = new SeperatorLine();
    spl->setProperty(WidgetShowInVerWindow, false);
    drawBoard()->attributionWidget()->installComAttributeWgt(EStartLineSep, spl);
}

void CPolygonalStarTool::toolCreatItemUpdate(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    if (pInfo != nullptr) {
        CGraphicsPolygonalStarItem *pItem = dynamic_cast<CGraphicsPolygonalStarItem *>(pInfo->businessItem);
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

void CPolygonalStarTool::toolCreatItemFinish(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    if (pInfo != nullptr) {
        CGraphicsPolygonalStarItem *m_pItem = dynamic_cast<CGraphicsPolygonalStarItem *>(pInfo->businessItem);
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

CGraphicsItem *CPolygonalStarTool::creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)
    if ((event->eventType() == CDrawToolEvent::EMouseEvent && event->mouseButtons() == Qt::LeftButton)
            || event->eventType() == CDrawToolEvent::ETouchEvent) {

        CGraphicsPolygonalStarItem *m_pItem =  new CGraphicsPolygonalStarItem;
        event->scene()->addCItem(m_pItem);

        return m_pItem;
    }
    return nullptr;
}

