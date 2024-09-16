// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ceraser.h"
#include "application.h"
#include "cdrawscene.h"
#include "cgraphicsview.h"

#include <DToolButton>
#include <QtMath>


CEraser::CEraser(): IDrawTool(EDrawToolMode(MoreTool + 3))
{

}

QAbstractButton *CEraser::initToolButton()
{
    DToolButton *btn = new DToolButton;
    setWgtAccesibleName(btn, "eraser tool button");
    btn->setToolTip("eraser");
    btn->setIconSize(QSize(48, 48));
    btn->setFixedSize(QSize(37, 37));
    btn->setCheckable(true);
    btn->setVisible(true);

//    connect(btn, &DToolButton::toggled, btn, [ = ](bool b) {
//        QIcon icon       = QIcon::fromTheme("ddc_brush tool_normal");
//        QIcon activeIcon = QIcon::fromTheme("ddc_brush tool_active");
//        btn->setIcon(b ? activeIcon : icon);
//    });
    //:/icons/deepin/builtin/texts/eraser tool_normal_36px.svg
    btn->setIcon(QIcon::fromTheme("eraser tool_normal"));
    return btn;
}

int CEraser::decideUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    auto layer = currentLayer(event->scene());
    if (layer != nullptr) {
        _activePictures[event->uuid()].beginSubPicture();

        return 1;
    }
    return 0;
}

void CEraser::toolStart(CDrawToolEvent *event, ITERecordInfo *pInfo)
{

}

void CEraser::toolUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
//    QPainter painter(&(event->scene()->sceneExImage()));

//    QPointF  prePos = pInfo->_prePos - event->scene()->sceneRect().topLeft();
//    QPointF  pos = event->pos() - event->scene()->sceneRect().topLeft();
//    QLineF line(prePos, pos);

//    const int inc = 20;

//    QPen pen;
//    pen.setWidth(inc);
//    pen.setCapStyle(Qt::RoundCap);

//    QPainterPathStroker ps(pen);

//    QPainterPath path;
//    path.moveTo(line.p1());
//    path.lineTo(line.p2());

//    path = ps.createStroke(path);

//    path = path.simplified();

//    painter.setCompositionMode(QPainter::CompositionMode_Source);

//    painter.setPen(QColor(0, 0, 0, 0));
//    painter.setBrush(QColor(0, 0, 0, 0));

//    painter.drawPath(path);
//    event->scene()->update();

    auto layer = currentLayer(event->scene());

    CGraphicsView *pView = event->scene()->drawView();
    QPicture picture;
    QPainter painter(&picture);

    QPointF  prePos = layer->mapFromScene(pInfo->_prePos) ;
    QPointF  pos = layer->mapFromScene((event->pos())) ;
    QLineF line(prePos, pos);
    QPen pen;
    pen.setWidthF(10 + pView->getDrawParam()->value(EPenWidth).value<qreal>());
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.setPen(pen);
    painter.drawLine(line);
    painter.end();

    layer->addPicture(picture, false, false);


    _activePictures[event->uuid()].addSubPicture(picture);


    //return picture;
}

void CEraser::toolFinish(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    auto layer = currentLayer(event->scene());
    if (layer != nullptr) {
        auto picture = _activePictures.take(event->uuid());
        picture.endSubPicture();
        currentLayer(event->scene())->addPicture(picture.picture(), true);
    }
}

bool CEraser::returnToSelectTool(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    return false;
}

void CEraser::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
{
    Q_UNUSED(oldStatus)
    Q_UNUSED(nowStatus)
}
JDynamicLayer *CEraser::currentLayer(CDrawScene *scene)
{
    auto items = scene->selectGroup()->items();

    if (items.count() == 1) {
        return dynamic_cast<JDynamicLayer *>(items.first());
    }
    return nullptr;
}

IDrawTool *creatTool()
{
    qWarning() << "creat CEraser !!!!!!!!!!!!!!!!!";
    return  new CEraser;
}


