// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clinetool.h"
#include "cdrawscene.h"
#include "cgraphicslineitem.h"
#include "cdrawparamsigleton.h"
#include "frame/cgraphicsview.h"
#include "cattributeitemwidget.h"
#include "application.h"
#include <QtMath>
#include <DToolButton>

CLineTool::CLineTool()
    : IDrawTool(line)
{

}

CLineTool::~CLineTool()
{

}

DrawAttribution::SAttrisList CLineTool::attributions()
{
    DrawAttribution::SAttrisList result;
    result << defaultAttriVar(DrawAttribution::EPenColor)
           << defaultAttriVar(DrawAttribution::EBorderWidth)
           << DrawAttribution::SAttri(1775)
           << defaultAttriVar(DrawAttribution::EStreakBeginStyle)
           << defaultAttriVar(DrawAttribution::EStreakEndStyle);
    return result;
}

QAbstractButton *CLineTool::initToolButton()
{
    DToolButton *m_lineBtn = new DToolButton;
    m_lineBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_L)));
    setWgtAccesibleName(m_lineBtn, "Line tool button");
    m_lineBtn->setToolTip(tr("Line (L)"));
    m_lineBtn->setIconSize(QSize(48, 48));
    m_lineBtn->setFixedSize(QSize(37, 37));
    m_lineBtn->setCheckable(true);
    connect(m_lineBtn, &DToolButton::toggled, m_lineBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_line tool_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_line tool_active");
        m_lineBtn->setIcon(b ? activeIcon : icon);
    });
    m_lineBtn->setIcon(QIcon::fromTheme("ddc_line tool_normal"));
    return m_lineBtn;
}

QCursor CLineTool::cursor() const
{
    static QPixmap s_cur = QPixmap(":/cursorIcons/line_mouse.svg");
    return QCursor(s_cur);
}

void CLineTool::toolCreatItemUpdate(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    Q_UNUSED(event)
    if (pInfo != nullptr) {
        CGraphicsLineItem *pItem = dynamic_cast<CGraphicsLineItem *>(pInfo->businessItem);
        if (nullptr != pItem) {
            bool shiftKeyPress = event->keyboardModifiers() & Qt::ShiftModifier;
            if (!shiftKeyPress) {
                pItem->setLine(QLineF(pInfo->_startPos, event->pos()));
            } else {
                QPointF local = pItem->mapFromScene(event->pos());
                QPointF p1;
                QPointF p2;

                p1 = pItem->line().p1();
                p2 = local;

                QLineF v = QLineF(p1, p2);
                if (fabs(v.dx()) - fabs(v.dy()) > 0.0001) {
                    p2.setY(p1.y());
                } else {
                    p2.setX(p1.x());
                }

                p1 = pItem->mapToScene(p1);
                p2 = pItem->mapToScene(p2);
                pItem->setRotation(0);
                pItem->setPos(0, 0);
                pItem->setLine(p1, p2);
            }
        }
    }
}

void CLineTool::toolCreatItemFinish(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo)
{
    if (pInfo != nullptr) {
        CGraphicsLineItem *m_pItem = dynamic_cast<CGraphicsLineItem *>(pInfo->businessItem);
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

CGraphicsItem *CLineTool::creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(pInfo)
    if ((event->eventType() == CDrawToolEvent::EMouseEvent && event->mouseButtons() == Qt::LeftButton)
            || event->eventType() == CDrawToolEvent::ETouchEvent) {

        CGraphicsLineItem *m_pItem = new CGraphicsLineItem(event->pos().x(), event->pos().y(), event->pos().x(), event->pos().y());
        event->scene()->addCItem(m_pItem);
        return m_pItem;
    }
    return nullptr;
}

int CLineTool::allowedMaxTouchPointCount()
{
    return 10;
}

