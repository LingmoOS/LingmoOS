// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RASTERCOMMAND_H
#define RASTERCOMMAND_H

#include <globaldefine.h>
#include <QPen>
#include <QBrush>
#include <QDataStream>
#include <QPicture>

using RasterPaint = QPicture;

class DRAWLIB_EXPORT RasterPaintCompsitor
{
public:
    RasterPaintCompsitor();
    RasterPaintCompsitor(const RasterPaintCompsitor &other);
    ~RasterPaintCompsitor();

    void merge(const RasterPaint &one);

    RasterPaint result() const;

    void clear();
private:
    PRIVATECLASS(RasterPaintCompsitor)
};

class RasterItem;
class DRAWLIB_EXPORT RasterCommandBase
{
public:
    virtual void doCommandFor(RasterItem *p) = 0;
    virtual int  cmdType() = 0;
    virtual void serialization(QDataStream &out) {}
    virtual void deserialization(QDataStream &in) {}

    static RasterCommandBase *creatCmd(int tp);
};


class DRAWLIB_EXPORT RasterCommand: public RasterCommandBase
{
public:
    RasterCommand() {}
    virtual ~RasterCommand() {}
    virtual void doCommandFor(RasterItem *p) = 0;
};

class DRAWLIB_EXPORT RasterGeomeCmd: public RasterCommand
{
public:
    RasterGeomeCmd(RasterItem *layer = nullptr);
    RasterGeomeCmd(const QPointF &pos, qreal rotate, qreal z, const QRectF &rct, const QTransform &trans);

    void setPos(const QPointF& pos){_pos = pos;}
    void setRotate(qreal rotate){_rotate = rotate;}
    void setZ(qreal z){_z = z;}
    void setRect(const QRectF& rct){_rect = rct;}
    void setTransform(const QTransform& trans){_trans = trans;}

    int  cmdType() override {return 1;}

    void doCommandFor(RasterItem *p) override;
    void serialization(QDataStream &out) override;
    void deserialization(QDataStream &in) override;

private:
    QPointF    _pos;
    qreal      _rotate;
    qreal      _z;
    QRectF     _rect;
    QTransform _trans;
};
class DRAWLIB_EXPORT RasterPathCmd: public RasterCommand
{
public:
    RasterPathCmd(const QPainterPath &path = QPainterPath(), const QPen &p = QPen());
    int  cmdType() override {return 2;}

    void doCommandFor(RasterItem *p) override;

    void serialization(QDataStream &out) override;
    void deserialization(QDataStream &in) override;

    QPainterPath _path;
    QPen _pen;
};

class DRAWLIB_EXPORT RasterPaintCmd: public RasterCommand
{
public:
    RasterPaintCmd(const RasterPaint &picture = RasterPaint(), bool dyImag = false);
    ~RasterPaintCmd();
    int  cmdType() override {return 3;}
    void doCommandFor(RasterItem *p) override;

    void serialization(QDataStream &out) override;
    void deserialization(QDataStream &in) override;

private:
    RasterPaint paintInfo;
    bool        _dyImag = false;
};

class DRAWLIB_EXPORT RasterBlurCmd: public RasterCommand
{
public:
    RasterBlurCmd(const QPainterPath &blurPath = QPainterPath(), int blurType = 0);
    int  cmdType() override {return 4;}
    void doCommandFor(RasterItem *p) override;

    void serialization(QDataStream &out) override;

    void deserialization(QDataStream &in) override;
private:

    QPainterPath _path;
    int _tp = 0;
};


//class DRAWLIB_EXPORT ImageBaseCmd
//{
//public:
//    enum ImageCmdType {PaintToImage,
//                       FillPixel,
//                       BlurImage,
//                       EraseImage
//                      };
//    virtual void doCommandFor(QImage &p) = 0;
//    virtual int  cmdType() = 0;
//};
//class DRAWLIB_EXPORT ImagePaintCmd: public ImageBaseCmd
//{
//public:
//    ImagePaintCmd(const RasterPaint &pt,bool dy = true);
//    void doCommandFor(QImage &p) override;
//    int  cmdType() override;

//    RasterPaint paint;
//    bool dyImag;
//};

//class DRAWLIB_EXPORT ImageBlurCmd: public ImageBaseCmd
//{
//public:
//    ImageBlurCmd(const QPainterPath& blurPath,int tp,int radius = 10);
//    void doCommandFor(QImage &p) override;
//    int  cmdType() override;

//    QPainterPath _path;
//    int _tp = 0;
//    int _radius = 10;
//};
#endif // RASTERCOMMAND_H
