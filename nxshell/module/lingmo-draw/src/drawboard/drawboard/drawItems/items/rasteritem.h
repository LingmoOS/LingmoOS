// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSLAYER_H
#define CGRAPHICSLAYER_H
#include "pageitem.h"
#include "rastercommand.h"
#include <QPicture>

class DRAWLIB_EXPORT RasterItem: public PageItem
{
public:
    enum ERasterType {EPenType, EImageType};

    RasterItem(const QImage &img = QImage(),
               ERasterType layerType = EPenType,
               PageItem *parent = nullptr);

    ~RasterItem() override;

    int  type() const override;

    ERasterType  rasterType() const;
    void         setRasterType(ERasterType layerTp);

    QImage &rImage();

    QImage image()const;
    void   setImage(const QImage &image);

    void setBlocked(bool b);
    bool isBlocked() const;

    SAttrisList attributions() override;
    void setAttributionVar(int attri, const QVariant &var, int phase) override;

    void clear();

    void addPaint(const RasterPaint &pt, bool creatCmd = true, bool dyImag = false, bool addToStack = true);
    void appendComand(RasterCommand *cmd, bool doCmd = false, bool addToStack = true);
    QList<RasterCommand *> commands();

    QPainterPath itemShape() const override;
    bool contains(const QPointF &point) const override;

    void updateShape() override;

    QRectF itemRect() const override;
    void   setRect(const QRectF &rct);

    QTransform imgTrans() const;
    QPointF mapScenePosToImage(const QPointF &pos) const;
    QPointF mapLocalToImage(const QPointF &pos) const;

    void loadUnit(const Unit &data) override;
    Unit getUnit(int reson) const override;

    void operatingEnd(PageItemEvent *event) override;

    void doMoving(PageItemMoveEvent *event) override;
    void doScaling(PageItemScalEvent *event) override;

    void doFilp(EFilpDirect dir = EFilpHor) override;

    void paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option) override;

    virtual bool isBlurEnable()const {return isBlocked();}

    virtual bool isEraserEnable()const {return boundingRect().isValid();}

    class DRAWLIB_EXPORT LayerBlockerKeeper
    {
    public:
        LayerBlockerKeeper(RasterItem *layer, bool block = true): _blocked(layer->isBlocked()), _layer(layer)
        {
            _layer->setBlocked(block);
        }
        ~LayerBlockerKeeper() {_layer->setBlocked(_blocked);}
    private:
        bool _blocked = false;
        RasterItem *_layer = nullptr;
    };

    PRIVATECLASS(RasterItem)
};

#endif // CGRAPHICSLAYER_H
