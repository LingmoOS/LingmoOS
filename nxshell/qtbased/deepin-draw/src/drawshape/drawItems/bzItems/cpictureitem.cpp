// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cpictureitem.h"
#include "cdrawscene.h"
#include "global.h"
#include "cgraphicsview.h"
#include "cgraphicsitemevent.h"
#include "cattributeitemwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QDebug>

//REGISTITEMCLASS(CPictureItem, PictureType)
CPictureItem::CPictureItem(const QPixmap &pixmap, CGraphicsItem *parent, const QByteArray &fileSrcData)
    : CGraphicsRectItem(parent)
    , m_pixmap(pixmap)
    , m_angle(0.0)
    , _srcByteArry(fileSrcData)
{
    updateShape();
}


CPictureItem::CPictureItem(const QRectF &rect, const QPixmap &pixmap, CGraphicsItem *parent, const QByteArray &fileSrcData)
    : CGraphicsRectItem(rect, parent)
    , m_pixmap(pixmap)
    , m_angle(0.0)
    , _srcByteArry(fileSrcData)
{
    this->setPen(Qt::NoPen);
    updateShape();
}

CPictureItem::~CPictureItem()
{

}

DrawAttribution::SAttrisList CPictureItem::attributions()
{
    DrawAttribution::SAttrisList result;
    bool enable = (drawScene()->selectGroup()->getBzItems(true).count() == 1);
    result << DrawAttribution::SAttri(DrawAttribution::EImageLeftRot, enable)
           << DrawAttribution::SAttri(DrawAttribution::EImageRightRot, enable)
           << DrawAttribution::SAttri(DrawAttribution::EImageHorFilp, enable)
           << DrawAttribution::SAttri(DrawAttribution::EImageVerFilp, enable)
           << DrawAttribution::SAttri(DrawAttribution::EImageAdaptScene,
                                      drawScene()->selectGroup()->sceneBoundingRect() != drawScene()->sceneRect());
    return result;
}

void CPictureItem::setAttributionVar(int attri, const QVariant &var, int phase)
{
    Q_UNUSED(var)
    switch (attri) {
    case EImageLeftRot: {
        setRotation90(true);
        break;
    }
    case EImageRightRot: {
        setRotation90(false);
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

int  CPictureItem::type() const
{
    return PictureType;
}

void CPictureItem::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    Q_UNUSED(option)

    QRectF pictureRect = QRectF(0, 0, m_pixmap.width(), m_pixmap.height());
    //painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(boundingRectTruly(), m_pixmap, pictureRect);

}

void CPictureItem::setAngle(const qreal &angle)
{
    m_angle = angle;
}

bool CPictureItem::isPosPenetrable(const QPointF &posLocal)
{
    Q_UNUSED(posLocal)
    return false;
}

bool CPictureItem::isRectPenetrable(const QRectF &rectLocal)
{
    Q_UNUSED(rectLocal)
    return false;
}

QPainterPath CPictureItem::getSelfOrgShape() const
{
    QPainterPath path;
    path.addRect(this->rect());
    return path;
}

void CPictureItem::operatingEnd(CGraphItemEvent *event)
{
    if (event->toolEventType() == 3) {
        //刷新出新的模糊图
        updateBlurPixmap(true);
    }
    CGraphicsRectItem::operatingEnd(event);
}

void CPictureItem::operatingBegin(CGraphItemEvent *event)
{
    _trans.reset();
    CGraphicsRectItem::operatingBegin(event);
}

void CPictureItem::updateShape()
{
    //CGraphicsRectItem::updateShape();
    m_selfOrgPathShape   = getSelfOrgShape();
    m_penStroerPathShape = m_selfOrgPathShape;
    m_boundingShape      = m_selfOrgPathShape;
    m_boundingRect       = rect();

    m_boundingShapeTrue  = m_selfOrgPathShape;
    m_boundingRectTrue   = rect();

    if (drawScene() != nullptr)
        drawScene()->selectGroup()->updateBoundingRect();
}

bool CPictureItem::isBlurEnable() const
{
    return true;
}

void CPictureItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    CGraphicsItem::paint(painter, option, widget);
}

void CPictureItem::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
}

QPixmap &CPictureItem::pixmap()
{
    return m_pixmap;
}

//void CPictureItem::setRotation90(bool leftOrRight)
//{

//    QPointF center = this->boundingRect().center();
//    qreal angle = 0.0;

//    // 左旋转减小 右旋转增大
//    if (leftOrRight == true) {
//        angle = - 90.0;
//    } else {
//        angle = 90.0;
//    }

//    // 矩阵变换
//    QTransform trans;
//    trans.translate(center.x(), center.y());
//    trans.rotate(angle);
//    trans.translate(-center.x(), -center.y());

//    // 设置当前旋转角度
//    setDrawRotatin(angle + drawRotation());
//    setTransform(trans, true);

//    updateShapeRecursion();
//}

void CPictureItem::loadGraphicsUnit(const CGraphicsUnit &data)
{
    if (data.data.pPic != nullptr) {
        CGraphicsRectItem::loadGraphicsRectUnit(data.data.pPic->rect);

        //qDebug() << "-------data.reson = " << data.reson;
        //仅在 '复制' 或者 '保存/加载到ddf文件时'加载图片原数据
        if (data.reson == EDuplicate || data.reson == ESaveToDDf) {
            m_pixmap = QPixmap::fromImage(data.data.pPic->image);
            _srcByteArry = data.data.pPic->srcByteArry;
        }
        this->_flipHorizontal = data.data.pPic->flipHorizontal;
        this->_flipVertical = data.data.pPic->flipVertical;
    }
    loadHeadData(data.head);
    update();
}

CGraphicsUnit CPictureItem::getGraphicsUnit(EDataReason reson) const
{
    CGraphicsUnit unit;
    unit.reson = reson;

    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(SGraphicsPictureUnitData);

    unit.head.pos = this->pos();
    unit.head.rotate = this->drawRotation();
    unit.head.zValue = this->zValue();
    unit.head.blurCount = _blurInfos.count();
    unit.head.blurInfos = _blurInfos;
    unit.head.trans = this->transform();

    unit.data.pPic = new SGraphicsPictureUnitData();
    unit.data.pPic->rect.topLeft = this->rect().topLeft();
    unit.data.pPic->rect.bottomRight = this->rect().bottomRight();
    unit.data.pPic->flipHorizontal = this->_flipHorizontal;
    unit.data.pPic->flipVertical = this->_flipVertical;

    //仅在 '复制' 或者 '保存/加载到ddf文件时'读取图片原数据
    if (reson == EDuplicate || reson == ESaveToDDf) {
        unit.data.pPic->image = m_pixmap.toImage();

        //源数据不为空,那么进行对当前图片
        if (_srcByteArry.isEmpty()) {
            QBuffer buferTemp;
            if (buferTemp.open(QIODevice::ReadWrite)) {
                m_pixmap.save(&buferTemp, "png", 100);
                unit.data.pPic->srcByteArry = buferTemp.buffer();
                buferTemp.close();
            }
        } else {
            unit.data.pPic->srcByteArry = _srcByteArry;
        }
    }
    return unit;
}
