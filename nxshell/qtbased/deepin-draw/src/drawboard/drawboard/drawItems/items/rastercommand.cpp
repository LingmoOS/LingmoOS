// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rastercommand.h"
#include "rasteritem.h"
#include "global.h"

#include <QPainter>

//class RasterPaint::RasterPaint_private
//{
//public:
//    explicit RasterPaint_private(RasterPaint *qq): q(qq) {}

//    RasterPaintCompsitor *getCompsitor()
//    {
//        if (compsitor == nullptr) {
//            compsitor = QSharedPointer<QPainter>(new RasterPaintCompsitor(q));
//        }
//        return compsitor.data();
//    }

//    RasterPaint *q;

//    QSharedPointer<RasterPaintCompsitor> compsitor = nullptr;
//};
//RasterPaint::RasterPaint(): RasterPaint_d(new RasterPaint_private(this))
//{

//}

//void RasterPaint::merge(const RasterPaint &one)
//{
//    d_RasterPaint()->getCompsitor()->merge(one);
//}

//void RasterPaint::end() const
//{
//    d_RasterPaint()->getCompsitor()->result();
//}
class RasterPaintCompsitor::RasterPaintCompsitor_private
{
public:
    enum EComposBy {ESubPictures, EPainterPath, ECount};
    explicit RasterPaintCompsitor_private(RasterPaintCompsitor *qq): q(qq),
        pictureData(new RasterPaint), pictureDataCache(new RasterPaint)
    {
    }


    inline QPicture &r_rasterPaint()
    {
        return *(pictureData.data());
    }
    inline QPainter *painter()
    {
        if (sharePainter.data() == nullptr) {
            sharePainter = QSharedPointer<QPainter>(new QPainter);
        }
        return sharePainter.data();
    }
    inline void beginSubPaint()
    {
        if (!painter()->isActive()) {
            painter()->begin(&(r_rasterPaint()));

            //还原之前的结果
            if (!pictureDataCache.data()->isNull()) {
                painter()->drawPicture(0, 0, *pictureDataCache.data());
            }
        }
    }
    inline void endSubPicture()
    {
        if (painter()->isActive()) {
            painter()->end();

            //备份当前的结果，以便下次还原
            painter()->begin(pictureDataCache.data());
            painter()->drawPicture(0, 0, *(pictureData.data()));
            painter()->end();
        }
    }

    RasterPaintCompsitor *q;

    //data should only one , so we use shared pointer to manager the memory.
    QSharedPointer<RasterPaint> pictureData;

    QSharedPointer<RasterPaint> pictureDataCache;

    QSharedPointer<QPainter> sharePainter;

};
RasterPaintCompsitor::RasterPaintCompsitor():
    RasterPaintCompsitor_d(new RasterPaintCompsitor_private(this))
{
}

RasterPaintCompsitor::RasterPaintCompsitor(const RasterPaintCompsitor &other):
    RasterPaintCompsitor_d(new RasterPaintCompsitor_private(this))
{
    d_RasterPaintCompsitor()->pictureData = other.d_RasterPaintCompsitor()->pictureData;
    d_RasterPaintCompsitor()->pictureDataCache = other.d_RasterPaintCompsitor()->pictureDataCache;
    d_RasterPaintCompsitor()->sharePainter = other.d_RasterPaintCompsitor()->sharePainter;
}

RasterPaintCompsitor::~RasterPaintCompsitor()
{
//    delete RasterPaintCompsitor_d;
//    RasterPaintCompsitor_d = nullptr;
}

void RasterPaintCompsitor::merge(const RasterPaint &one)
{
    if (!one.isNull()) {
        d_RasterPaintCompsitor()->beginSubPaint();
        d_RasterPaintCompsitor()->painter()->drawPicture(QPointF(0, 0), one);
        d_RasterPaintCompsitor()->endSubPicture();
    }
}

RasterPaint RasterPaintCompsitor::result() const
{
    d_RasterPaintCompsitor()->endSubPicture();
    return *(d_RasterPaintCompsitor()->pictureData.data());
}

void RasterPaintCompsitor::clear()
{
    d_RasterPaintCompsitor()->endSubPicture();
    d_RasterPaintCompsitor()->pictureData      = QSharedPointer<RasterPaint>(new QPicture);
    d_RasterPaintCompsitor()->pictureDataCache = QSharedPointer<RasterPaint>(new QPicture);
}

RasterCommandBase *RasterCommandBase::creatCmd(int tp)
{
    RasterCommandBase *result = nullptr;
    switch (tp) {
    case 1: {
        result = new RasterGeomeCmd();
        break;
    }
    case 2: {
        result = new RasterPathCmd();
        break;
    }
    case 3: {
        result = new RasterPaintCmd();
        break;
    }
    case 4: {
        result = new RasterBlurCmd();
        break;
    }
    }

    return result;
}
RasterGeomeCmd::RasterGeomeCmd(RasterItem *layer): RasterCommand()
{
    if (layer != nullptr) {
        _pos = layer->pos();
        _rotate = layer->drawRotation();
        _z = layer->pageZValue();
        _trans = layer->transform();
        _rect = layer->itemRect();
    }
}

RasterGeomeCmd::RasterGeomeCmd(const QPointF &pos, qreal rotate, qreal z,
                               const QRectF &rct, const QTransform &trans): RasterCommand(),
    _pos(pos),
    _rotate(rotate),
    _z(z),
    _trans(trans),
    _rect(rct)
{
}

void RasterGeomeCmd::doCommandFor(RasterItem *p)
{
    extern void Raster_SetRect(RasterItem * item, const QRectF & rct, bool addcmd);
    if (p != nullptr) {
        p->setPos(_pos);
        p->setDrawRotatin(_rotate);
        p->setPageZValue(_z);
        p->setTransform(_trans);

        Raster_SetRect(p, _rect, false);
        //p->setRect(_rect);
    }
}

void RasterGeomeCmd::serialization(QDataStream &out)
{
    out << _pos;
    out << _rotate;
    out << _z;
    out << _rect;
    out << _trans;
}

void RasterGeomeCmd::deserialization(QDataStream &in)
{
    in >> _pos;
    in >> _rotate;
    in >> _z;
    in >> _rect;
    in >> _trans;
}

RasterPathCmd::RasterPathCmd(const QPainterPath &path, const QPen &p)
    : RasterCommand(), _path(path), _pen(p)
{
}

void RasterPathCmd::doCommandFor(RasterItem *p)
{
    if (p != nullptr) {
        RasterItem::LayerBlockerKeeper blocker(p, false);
        //_layer->addPenPath(_path, _pen, 0, false);
    }
}

void RasterPathCmd::serialization(QDataStream &out)
{
    out << _path;
    out << _pen;
}

void RasterPathCmd::deserialization(QDataStream &in)
{
    in >> _path;
    in >> _pen;
}

RasterPaintCmd::RasterPaintCmd(const RasterPaint &picture, bool dyImag): RasterCommand(),
    paintInfo(picture), _dyImag(dyImag)
{
}

RasterPaintCmd::~RasterPaintCmd()
{
    paintInfo = RasterPaint();
}

void RasterPaintCmd::doCommandFor(RasterItem *p)
{
    if (p != nullptr) {
        //RasterItem::LayerBlockerKeeper blocker(p, false);
        //p->addPaint(paintInfo, false, _dyImag);
        auto pt = paintInfo;
        QImage &img = p->rImage();
        QImage oldImg = p->rImage();
        QRect pictureRectInlayer = pt.boundingRect();
        auto rect = (img.isNull() ?  pictureRectInlayer : p->itemRect()).toRect();
        if (_dyImag) {
            rect = (p->itemRect() | pictureRectInlayer).toRect();
        }
        if (oldImg.size() != rect.size()) {
            img = QImage(rect.size(), QImage::Format_ARGB32);
            img.fill(Qt::transparent);
            QPainter painter(&img);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.translate(-rect.topLeft());
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            painter.drawImage(p->itemRect(), oldImg, QRectF(0, 0, oldImg.width(), oldImg.height()));
        }
        QPainter painter(&img);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.translate(-rect.topLeft());
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPicture(QPoint(0, 0), pt);

        extern void Raster_SetRect(RasterItem * item, const QRectF & rct, bool addcmd);
        //p->setRasterRect(rect);
        Raster_SetRect(p, rect, false);
    }
}

void RasterPaintCmd::serialization(QDataStream &out)
{
    out << paintInfo;
    out << _dyImag;
}

void RasterPaintCmd::deserialization(QDataStream &in)
{
    in >> paintInfo;
    in >> _dyImag;
}

RasterBlurCmd::RasterBlurCmd(const QPainterPath &blurPath, int blurType): RasterCommand(),
    _path(blurPath), _tp(blurType)
{
}

void RasterBlurCmd::doCommandFor(RasterItem *p)
{
    if (p != nullptr) {
        auto blurImag = NSBlur::blurImage(p->rImage(), 10, _tp);
        QPainter painter(&p->rImage());
        painter.setClipPath(_path);
        painter.drawImage(QRect(0, 0, blurImag.width(), blurImag.height()), blurImag);
    }
}

void RasterBlurCmd::serialization(QDataStream &out)
{
    out << _path;
    out << _tp;
}

void RasterBlurCmd::deserialization(QDataStream &in)
{
    in >> _path;
    in >> _tp;
}

//ImagePaintCmd::ImagePaintCmd(const RasterPaint &pt, bool dy):
//    ImageBaseCmd(),
//    paint(pt),
//    dyImag(dy)
//{

//}

//void ImagePaintCmd::doCommandFor(QImage &p)
//{
//    QImage &_img = p;
//    QImage oldImg = p;
//    RasterPaint picture = paint;

//    QRect pictureRectInlayer = picture.boundingRect();
//    auto rect = (_img.isNull() ?  pictureRectInlayer : _img.rect());
//    if (dyImag) {
//        rect = _img.rect() | pictureRectInlayer;
//    }
//    if (oldImg.size() != rect.size()) {
//        _img = QImage(rect.size(), QImage::Format_ARGB32);
//        _img.fill(Qt::transparent);
//        QPainter painter(&_img);
//        painter.setRenderHint(QPainter::Antialiasing);
//        painter.translate(-rect.topLeft());
//        painter.setRenderHint(QPainter::SmoothPixmapTransform);
//        painter.drawImage(_img.rect(), oldImg, QRectF(0, 0, oldImg.width(), oldImg.height()));
//    }
//    QPainter painter(&_img);
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.translate(-rect.topLeft());
//    painter.setRenderHint(QPainter::SmoothPixmapTransform);
//    painter.drawPicture(QPoint(0, 0), picture);
//    painter.end();
//}

//int ImagePaintCmd::cmdType()
//{
//    return PaintToImage;
//}

//ImageBlurCmd::ImageBlurCmd(const QPainterPath &blurPath, int tp, int radius):_path(blurPath),
//    _tp(tp),_radius(radius)
//{

//}

//void ImageBlurCmd::doCommandFor(QImage &p)
//{
//    auto blurImag = NSBlur::blurImage(p, 10, _tp);
//    QPainter painter(&p);
//    painter.setClipPath(_path);
//    painter.drawImage(QRect(0, 0, blurImag.width(), blurImag.height()), blurImag);
//}

//int ImageBlurCmd::cmdType()
//{
//    return BlurImage;
//}
