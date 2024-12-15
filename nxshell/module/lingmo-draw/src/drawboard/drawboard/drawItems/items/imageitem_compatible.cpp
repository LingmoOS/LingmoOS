// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageitem_compatible.h"
#include "pagescene.h"
#include "global.h"
#include "pageview.h"
#include "cgraphicsitemevent.h"
#include "attributewidget.h"
#include "itemgroup.h"
#include "pageitem_p.h"

#include <QPainter>
#include <QPainterPath>
#include <QDebug>

ImageItem_Compatible::ImageItem_Compatible(const QPixmap &pixmap, PageItem *parent, const QByteArray &fileSrcData)
    : RectBaseItem(parent)
    , m_pixmap(pixmap)
    , m_angle(0.0)
    , _srcByteArry(fileSrcData)
{
    updateShape();
}


ImageItem_Compatible::ImageItem_Compatible(const QRectF &rect, const QPixmap &pixmap, PageItem *parent, const QByteArray &fileSrcData)
    : RectBaseItem(rect, parent)
    , m_pixmap(pixmap)
    , m_angle(0.0)
    , _srcByteArry(fileSrcData)
{
    this->setPen(Qt::NoPen);
    updateShape();
}

ImageItem_Compatible::~ImageItem_Compatible()
{

}

SAttrisList ImageItem_Compatible::attributions()
{
    SAttrisList result;
//    bool enable = (pageScene()->selectGroup()->getNotGroupItems(true).count() == 1);
//    result <<  SAttri(EImageLeftRot, enable)
//           <<  SAttri(EImageRightRot, enable)
//           <<  SAttri(EImageHorFilp, enable)
//           <<  SAttri(EImageVerFilp, enable)
//           <<  SAttri(EImageAdaptScene,
//                      pageScene()->selectGroup()->sceneBoundingRect() != pageScene()->sceneRect());
    return result;
}

void ImageItem_Compatible::setAttributionVar(int attri, const QVariant &var, int phase)
{
    Q_UNUSED(var)
    switch (attri) {
    case EImageLeftRot: {
        rot90Angle(true);
        break;
    }
    case EImageRightRot: {
        rot90Angle(false);
        break;
    }
    case EImageHorFilp: {
        doFilp(EFilpHor);
        break;
    }
    case EImageVerFilp: {
        doFilp(EFilpVer);
        break;
    }
    case EImageAdaptScene: {
        break;
    }
    default:
        break;
    }
}

int  ImageItem_Compatible::type() const
{
    return PictureType;
}

void ImageItem_Compatible::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    Q_UNUSED(option)

    QRectF pictureRect = QRectF(0, 0, m_pixmap.width(), m_pixmap.height());
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(orgRect(), m_pixmap, pictureRect);
}

void ImageItem_Compatible::setAngle(const qreal &angle)
{
    m_angle = angle;
}

bool ImageItem_Compatible::isPosPenetrable(const QPointF &posLocal)
{
    Q_UNUSED(posLocal)
    return false;
}

bool ImageItem_Compatible::isRectPenetrable(const QRectF &rectLocal)
{
    Q_UNUSED(rectLocal)
    return false;
}

QPainterPath ImageItem_Compatible::calOrgShapeBaseRect(const QRectF &rect) const
{
    QPainterPath path;
    path.addRect(rect);
    return path;
}

void ImageItem_Compatible::operatingEnd(PageItemEvent *event)
{
//    if (event->toolEventType() == 3) {
//        //刷新出新的模糊图
//        updateBlurPixmap(true);
//    }
    RectBaseItem::operatingEnd(event);
}

void ImageItem_Compatible::operatingBegin(PageItemEvent *event)
{
    _trans.reset();
    RectBaseItem::operatingBegin(event);
}

//void ImageItem_Compatible::updateShape()
//{
//    m_selfOrgPathShape   = getSelfOrgShape();
//    m_penStroerPathShape = m_selfOrgPathShape;
//    m_boundingShape      = m_selfOrgPathShape;
//    m_boundingRect       = Rectangle();

//    m_boundingShapeTrue  = m_selfOrgPathShape;
//    m_boundingRectTrue   = Rectangle();

//    if (pageScene() != nullptr)
//        pageScene()->selectGroup()->updateBoundingRect();
//}

void ImageItem_Compatible::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
}

QPixmap &ImageItem_Compatible::pixmap()
{
    return m_pixmap;
}

void ImageItem_Compatible::loadUnit(const Unit &data)
{
    ImageUnitData_Comp i = data.data.data.value<ImageUnitData_Comp>();
    RectBaseItem::loadGraphicsRectUnit(i.rect);

    if (data.usage == UnitUsage_Copy || data.usage == UnitUsage_Save) {
        m_pixmap = QPixmap::fromImage(i.image);
        _srcByteArry = i.srcByteArry;
    }
    d_PageItem()->_flipHorizontal = i.flipHorizontal;
    d_PageItem()->_flipVertical = i.flipVertical;

    loadHeadData(data.head);

    update();
}

Unit ImageItem_Compatible::getUnit(int reson) const
{
    Unit unit;
    unit.usage = reson;

    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(ImageUnitData_Comp);

    unit.head.pos = this->pos();
    unit.head.rotate = this->drawRotation();
    unit.head.zValue = this->zValue();
    //unit.head.blurCount = _blurInfos.count();
    //unit.head.blurInfos = _blurInfos;
    unit.head.trans = this->transform();

    ImageUnitData_Comp i;
    i.rect.topLeft = itemRect().topLeft();
    i.rect.bottomRight = itemRect().bottomRight();
    i.flipHorizontal = d_PageItem()->_flipHorizontal;
    i.flipVertical = d_PageItem()->_flipVertical;
    //仅在 '复制' 或者 '保存/加载到ddf文件时'读取图片原数据
    if (reson == UnitUsage_Copy || reson == UnitUsage_Save) {
        i.image = m_pixmap.toImage();

        //源数据不为空,那么进行对当前图片
        if (_srcByteArry.isEmpty()) {
            QBuffer buferTemp;
            if (buferTemp.open(QIODevice::ReadWrite)) {
                m_pixmap.save(&buferTemp, "png", 100);
                i.srcByteArry = buferTemp.buffer();
                buferTemp.close();
            }
        } else {
            i.srcByteArry = _srcByteArry;
        }
    }
    unit.data.data.setValue<ImageUnitData_Comp>(i);

    return unit;
}
