// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ccalligraphypen.h"
#include "application.h"
#include "cdrawscene.h"

#include <DToolButton>
#include <QtMath>


CCalligraphyPen::CCalligraphyPen(): IDrawTool(EDrawToolMode(MoreTool + 1))
{

}

QAbstractButton *CCalligraphyPen::initToolButton()
{
    DToolButton *btn = new DToolButton;
    btn->setShortcut(QKeySequence(Qt::Key_U));
    setWgtAccesibleName(btn, "Calligraphy Pen tool button");
    btn->setToolTip("Calligraphy Pen (U)");
    btn->setIconSize(QSize(48, 48));
    btn->setFixedSize(QSize(37, 37));
    btn->setCheckable(true);
    btn->setVisible(true);

    connect(btn, &DToolButton::toggled, btn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_brush tool_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_brush tool_active");
        btn->setIcon(b ? activeIcon : icon);
    });
    btn->setIcon(QIcon::fromTheme("ddc_brush tool_normal"));
    return btn;
}

int CCalligraphyPen::decideUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    return 1;
}

void CCalligraphyPen::toolStart(CDrawToolEvent *event, ITERecordInfo *pInfo)
{

}

void CCalligraphyPen::toolUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    if (pInfo->_opeTpUpdate == 1) {
        QPainter painter(&(event->scene()->sceneExImage()));
        QPointF  prePos = pInfo->_prePos - event->scene()->sceneRect().topLeft();
        QPointF  pos = event->pos() - event->scene()->sceneRect().topLeft();
        QLineF l(prePos, pos);
        const qreal angleDegress = 30;
        const qreal cW = 20;
        qreal offXLen = qCos(qDegreesToRadians(angleDegress)) * (cW / 2.0);
        qreal offYLen = qSin(qDegreesToRadians(angleDegress)) * (cW / 2.0);
        QPointF point1(prePos.x() - offXLen, prePos.y() - offYLen);
        QPointF point2(prePos.x() + offXLen, prePos.y() + offYLen);

        QPointF point3(pos.x() - offXLen, pos.y() - offYLen);
        QPointF point4(pos.x() + offXLen, pos.y() + offYLen);

        QPainterPath path;
        path.moveTo(point1);

        if (l.length() > 20) {
            path.quadTo(QLineF(point1, point2).center(), point2);
            path.quadTo(QLineF(point2, point4).center(), point4);
            path.quadTo(QLineF(point4, point3).center(), point3);
            path.quadTo(QLineF(point3, point1).center(), point1);
        } else {
            path.lineTo(point2);
            path.lineTo(point4);
            path.lineTo(point3);
            path.lineTo(point1);
        }
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(Qt::red);
        painter.setPen(Qt::NoPen);

        painter.drawPath(path);

        _resultImagePaths[event->uuid()].addPath(path);
    }
    event->scene()->update();
}

void CCalligraphyPen::toolFinish(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    if (pInfo->_opeTpUpdate == 1) {
        QPainter painter(&(event->scene()->sceneExImage()));
        foreach (auto path, _resultImagePaths) {
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(Qt::red);
            painter.setPen(Qt::NoPen);

            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.drawPath(path);
        }

        _resultImagePaths.clear();
        event->scene()->update();
    }
}

bool CCalligraphyPen::returnToSelectTool(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    return false;
}

IDrawTool *creatTool()
{
    qWarning() << "creat CCalligraphyPen !!!!!!!!!!!!!!!!!";
    return  new CCalligraphyPen;
}
