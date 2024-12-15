// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rasteritem.h"
#include "pagescene.h"
#include "cundoredocommand.h"
#include "cgraphicsitemevent.h"
#include "global.h"
#include "pageview.h"
//#include "blurwidget.h"

REGISTITEMCLASS(RasterItem, int(RasterItemType))


class RasterItem::RasterItem_private
{
public:
    explicit RasterItem_private(RasterItem *qq, const QImage &image, ERasterType layerType):
        q(qq), layerType(layerType), img(image.convertToFormat(QImage::Format_ARGB32)),
        baseImg(img), baseRect(img.rect()), rect(img.rect())
    {}

    inline QImage &currentImage()
    {
        return img;
    }

    void mergeRectChangedToPreGemeCmd()
    {
        //baseRect 是绘制动态增长时必要的参数，所以显示大小rect变化时，
        //如果没有任何绘制参数那么baseRect需要进行设置为当前的rect大小，从而避免添加一个大小变化的命令
        if (commands.isEmpty()) {
            baseRect = q->orgRect();
            return;
        }

        for (int i = commands.count() - 1; i >= 0; --i) {
            auto p = commands.at(i).data();
            if (p->cmdType() == 1) {
                auto gemCmd = dynamic_cast<RasterGeomeCmd *>(p);
                gemCmd->setPos(q->pos());
                gemCmd->setRotate(q->drawRotation());
                gemCmd->setRect(q->orgRect());
                gemCmd->setTransform(q->transform());
                gemCmd->setZ(q->pageZValue());
                return;
            } else if (p->cmdType() == 3) {
                break;
            }
        }
        auto newCmd = new RasterGeomeCmd(q);
        q->appendComand(newCmd, false, false);
    }
    void setRect_Helper(const QRectF &rct, bool considerAddGemoCmd = true)
    {
        if (rct != rect) {
            q->preparePageItemGeometryChange();
            rect = rct;
            q->notifyChanged(PageItemRectChanged, rct);
        }
        if (considerAddGemoCmd) {
            mergeRectChangedToPreGemeCmd();
        }
    }
    RasterItem *q;


    QImage img;

    QImage baseImg;

    QRectF baseRect;

    QRectF rect;

    QList<QSharedPointer<RasterCommandBase>> commands;

    //QList<QSharedPointer<ImageBaseCmd>> imageCmds;

    bool isBlocked = false;

    int  layerType = EPenType;
};

void Raster_SetRect(RasterItem *item, const QRectF &rct, bool addcmd)
{
    item->d_RasterItem()->setRect_Helper(rct, addcmd);
}

RasterItem::RasterItem(const QImage &img, ERasterType layerType, PageItem *parent): PageItem(parent)
    , RasterItem_d(new RasterItem_private(this, img, layerType))
{
}

RasterItem::~RasterItem()
{
    clear();
}

int RasterItem::type() const
{
    return RasterItemType;
}

RasterItem::ERasterType RasterItem::rasterType() const
{
    return RasterItem::ERasterType(d_RasterItem()->layerType);
}

void RasterItem::setRasterType(ERasterType layerTp)
{
    d_RasterItem()->layerType = layerTp;
}

void RasterItem::setBlocked(bool b)
{
    d_RasterItem()->isBlocked = b;
}

bool RasterItem::isBlocked() const
{
    return d_RasterItem()->isBlocked;
}

SAttrisList RasterItem::attributions()
{
    SAttrisList result;
    if (type() == RasterItemType) {
        bool enable = (pageScene()->selectedItemCount() == 1);
        QList<QVariant> couple;
        result <<  SAttri(EImageLeftRot, enable)
               <<  SAttri(EImageRightRot, enable)
               <<  SAttri(EImageHorFilp, enable)
               <<  SAttri(EImageVerFilp, enable)
               <<  SAttri(ERotProperty, drawRotation());
        //<<  SAttri(EGroupWgt, couple)
        //<<  SAttri(EOrderProperty, QVariant())

        if (rasterType() == ERasterType::EImageType)
            result <<  SAttri(EImageAdaptScene, sceneBoundingRect() != pageScene()->currentTopLayerSceneRect());
    }
    return result;
}

void RasterItem::setAttributionVar(int attri, const QVariant &var, int phase)
{
    Q_UNUSED(var)
    switch (attri) {
    case EImageLeftRot: {
        rot90Angle(true);
        auto p = new RasterGeomeCmd(this);
        appendComand(p, false, false);
        break;
    }
    case EImageRightRot: {
        rot90Angle(false);
        auto p = new RasterGeomeCmd(this);
        appendComand(p, false, false);
        break;
    }
    case EImageHorFilp: {
        doFilp(EFilpHor);
        auto p = new RasterGeomeCmd(this);
        appendComand(p, false, false);
        break;
    }
    case EImageVerFilp: {
        doFilp(EFilpVer);
        auto p = new RasterGeomeCmd(this);
        appendComand(p, false, false);
        break;
    }
    case EImageAdaptScene: {
        break;
    }
    case ERectProperty: {
        QRect tRect = var.toRect();
        QPoint tmpPos = QPoint(tRect.topLeft().x() - itemRect().topLeft().x(), tRect.topLeft().y() - itemRect().topLeft().y());
        setPos(tmpPos);
        tRect.setX(itemRect().x());
        tRect.setY(itemRect().y());
        setRect(tRect);
        break;
    }
    default:
        break;
    }
}

void RasterItem::clear()
{
    d_RasterItem()->commands.clear();
    preparePageItemGeometryChange();
    setPos(QPointF(0, 0));
    setDrawRotatin(0);
    resetTransform();
    d_RasterItem()->setRect_Helper(d_RasterItem()->baseRect, false);
    d_RasterItem()->img = d_RasterItem()->baseImg;
}


//void RasterItem::addPenPath(const QPainterPath &path, const QPen &pen, int type, bool creatCmd)
//{
//    if (_isBlocked)
//        return;

//    QImage oldImg = _img;

//    QPainterPathStroker ps(pen);

//    QPainterPath pathStroke;

//    pathStroke = ps.createStroke(path);

//    pathStroke = pathStroke.simplified();

//    auto rect = _img.isNull() ? pathStroke.boundingRect() : boundingRect() | pathStroke.boundingRect();

//    if (type == 1) {
//        rect = boundingRect();
//    }

//    if (oldImg.size() != rect.size().toSize()) {
//        _img = QImage(rect.size().toSize(), QImage::Format_ARGB32);
//        _img.fill(Qt::transparent);
//    }

//    QPainter painter(&_img);
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.translate(-rect.topLeft());

//    if (!oldImg.isNull() && oldImg.size() != rect.size().toSize()) {
//        painter.setRenderHint(QPainter::SmoothPixmapTransform);
//        painter.drawImage(boundingRect(), oldImg, QRectF(0, 0, oldImg.width(), oldImg.height()));
//    }

//    QPen pen1 = pen;
//    if (type == 1)
//        painter.setCompositionMode(QPainter::CompositionMode_Source);

//    pen1.setJoinStyle(Qt::RoundJoin);
//    pen1.setCapStyle(Qt::RoundCap);
//    painter.setPen(pen1);
//    painter.drawPath(path);

//    setRect(rect);

//    if (creatCmd) {
//        auto p = new JPathCommand(path, pen, this);
//        appendComand(p);
//    }
//}

void RasterItem::addPaint(const RasterPaint &pt, bool creatCmd, bool dyImag, bool addToStack)
{
    if (d_RasterItem()->isBlocked)
        return;
    auto p = new RasterPaintCmd(pt, dyImag);
    p->doCommandFor(this);
    if (creatCmd) {
        appendComand(p, false, addToStack);
    } else {
        delete p;
    }
}

void RasterItem::appendComand(RasterCommand *cmd, bool doCmd, bool addToStack)
{
    UndoRecorder blocker(addToStack ? this : nullptr);
    d_RasterItem()->commands.append(QSharedPointer<RasterCommandBase>(cmd));
    if (doCmd) {
        cmd->doCommandFor(this);
    }
}

QPainterPath RasterItem::itemShape() const
{
    QPainterPath path;
    path.addRect(d_RasterItem()->rect);
    return path;
}

bool RasterItem::contains(const QPointF &point) const
{
    if (pageGroup() != nullptr)
        return false;
    return itemRect().contains(point);
}

void RasterItem::updateShape()
{
    PageItem::updateShape();
}

QRectF RasterItem::itemRect() const
{
    return d_RasterItem()->rect;
}

void RasterItem::setRect(const QRectF &rct)
{
    d_RasterItem()->setRect_Helper(rct, true);
}

QImage &RasterItem::rImage()
{
    return d_RasterItem()->img;
}

QImage RasterItem::image() const
{
    return d_RasterItem()->img;
}

void RasterItem::setImage(const QImage &image)
{
    d_RasterItem()->commands.clear();
    d_RasterItem()->img = image;
    d_RasterItem()->baseImg = image;
    d_RasterItem()->baseRect = image.rect();

    //保证当前itemRect的topleft点不变
    if (!itemRect().isValid() && !image.isNull()) {
        d_RasterItem()->baseRect = QRectF(itemRect().topLeft(), image.size());
    }
}

QPointF RasterItem::mapScenePosToImage(const QPointF &pos) const
{
    auto result = mapFromScene(pos);
    return mapLocalToImage(result);
}

QPointF RasterItem::mapLocalToImage(const QPointF &pos)const
{
    return imgTrans().map(pos);
}

void RasterItem::loadUnit(const Unit &data)
{
    this->clear();
    RasterUnitData i = data.value<RasterUnitData>();
    d_RasterItem()->baseImg   = i.baseImg;
    d_RasterItem()->baseRect  = i.baseRect;
    d_RasterItem()->isBlocked = i.blocked;
    d_RasterItem()->layerType = i.layerType;
    d_RasterItem()->setRect_Helper(i.baseRect, false);

    d_RasterItem()->img = i.baseImg;
    LayerBlockerKeeper keeper(this, false);
    auto cmds = i.commands;
    foreach (auto c, cmds) {
        c->doCommandFor(this);
    }
    d_RasterItem()->commands = cmds;


    PageItem::loadUnit(data);
}

Unit RasterItem::getUnit(int usage) const
{
    Unit unit = PageItem::getUnit(usage);
    RasterUnitData i;
    if (usage != UnitUsage_Save) {
        i.baseImg   = d_RasterItem()->baseImg;
        i.baseRect  = d_RasterItem()->baseRect;
        i.blocked   = d_RasterItem()->isBlocked;
        i.commands  = d_RasterItem()->commands;
        i.layerType = d_RasterItem()->layerType;
    } else {
        //用于保存那么不再需要commands,直接使用img即可
        i.baseImg   = d_RasterItem()->img;
        i.baseRect  = orgRect();
        i.blocked   = d_RasterItem()->isBlocked;
        i.layerType = d_RasterItem()->layerType;
    }
    unit.setValue<RasterUnitData>(i);
    return unit;
}

void RasterItem::operatingEnd(PageItemEvent *event)
{
    PageItem::operatingEnd(event);
}

void RasterItem::doMoving(PageItemMoveEvent *event)
{
    QPointF move = event->_scenePos - event->_oldScenePos;
    this->moveBy(move.x(), move.y());
}

void RasterItem::doScaling(PageItemScalEvent *event)
{
    QTransform trans = event->trans();
    QRectF rct = this->itemRect();
    QPointF pos1 = trans.map(rct.topLeft());
    QPointF pos4 = trans.map(rct.bottomRight());
    QRectF newRect = QRectF(pos1, pos4);

    if (newRect.isValid())
        this->setRect(newRect);
}

void RasterItem::doFilp(EFilpDirect dir)
{
    PageItem::doFilp(dir);

//    QPointF center = boundingRect().center();
//    QTransform trans(dir == EFilpHor ? -1 : 1, 0, 0,
//                     0, dir == EFilpVer  ? -1 : 1, 0,
//                     0, 0, 1);

//    trans = (QTransform::fromTranslate(-center.x(), -center.y()) * trans * QTransform::fromTranslate(center.x(), center.y()));

//    setTransform(trans, true);
}

void RasterItem::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    beginCheckIns(painter);

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    auto img = d_RasterItem()->currentImage();
    painter->drawImage(itemRect(), img, img.rect());

    endCheckIns(painter);
}

QList<RasterCommand *> RasterItem::commands()
{
    QList<RasterCommand *> result;
    foreach (auto p, d_RasterItem()->commands) {
        result.append(static_cast<RasterCommand *>(p.data()));
    }
    return result;
}

//item pos to image pos
QTransform RasterItem::imgTrans() const
{
    if (d_RasterItem()->img.isNull())
        return QTransform();

    auto pos = itemRect().topLeft();
    QTransform scaledTrans;
    scaledTrans.scale(d_RasterItem()->img.width() / boundingRect().width(), d_RasterItem()->img.height() / itemRect().height());
    scaledTrans.translate(-pos.x(), -pos.y());
    return scaledTrans;
}
