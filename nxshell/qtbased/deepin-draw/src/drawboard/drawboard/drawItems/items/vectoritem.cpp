// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vectoritem.h"
#include "pageview.h"
#include "itemgroup.h"
#include "drawtool.h"
#include <QElapsedTimer>

#include <QApplication>

class VectorItem::VectorItem_private
{
public:
    explicit VectorItem_private(VectorItem *qp): q(qp)
    {
    }
    ~VectorItem_private()
    {
        if (_cachePixmap != nullptr) {
            delete _cachePixmap;
            _cachePixmap = nullptr;
        }
    }
    QPixmap getCachePixmap(bool baseOrg = false)
    {
        qreal devecePixelRatio = q->pageView() != nullptr ? q->pageView()->devicePixelRatio() : qApp->devicePixelRatio();

        QSizeF size = q->itemRect().size();
        QPixmap pix((size * devecePixelRatio).toSize());

        pix.setDevicePixelRatio(devecePixelRatio);

        pix.fill(QColor(0, 0, 0, 0));

        QPainter painter(&pix);

        painter.translate(-q->itemRect().topLeft());

        painter.setRenderHint(QPainter::Antialiasing);

        q->paintItemSelf(&painter, &_curStyleOption, EPaintForCache);

        return pix;
    }
    void resetCachePixmap()
    {
        if (q->operatingType() != -1)
            return;

        if (_useCachePixmap && _cachePixmap != nullptr && q->operatingType() == -1) {
            *_cachePixmap = getCachePixmap(false);
        }
    }

    void setDrawBorder(bool bDraw)
    {
        m_bDrawBorder = bDraw;
        q->update();
    }

    bool getDrawBorder()
    {
        return m_bDrawBorder;
    }

    void setDrawFill(bool bDraw)
    {
        m_bDrawFill = bDraw;
        q->update();
    }

    bool getDrawFill()
    {
        return m_bDrawFill;
    }

    VectorItem *q;
    QPen   pen = QPen(QColor(0, 0, 0));
    QBrush brush = QBrush(QColor(0, 0, 0, 0));

    QPainterPath orgPath;
    QPainterPath outerPath;
    QPainterPath penStrokerPath;

    QRectF  orgRect;
    QRectF  outerRect;

    QPixmap    *_cachePixmap    = nullptr;
    bool        _useCachePixmap = false;
    bool        _autoCache      = true;
    int         _autoEplMs      = 8;
    bool m_bDrawBorder = true;
    bool m_bDrawFill = true;

    QColor m_penPreviewColor;
    int    m_penWidth = 1;
    QColor m_brPreviewColor;
    bool   m_isPreviewCom[3] {0};
    QStyleOptionGraphicsItem  _curStyleOption;
};

QPainterPath VectorItem::getPenStrokerPath(const QPainterPath &orgPath,
                                           const QPen &pen,
                                           bool penStrokerShape,
                                           const qreal incW, bool doSimplified)
{
    // We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
    // if we pass a value of 0.0 to QPainterPathStroker::setWidth()
    const qreal penWidthZero = qreal(0.00000001);

    if (orgPath == QPainterPath() || pen == Qt::NoPen)
        return orgPath;
    QPainterPathStroker ps(pen);

    if (pen.widthF() <= 0.0)
        ps.setWidth(penWidthZero);
    else
        ps.setWidth(pen.widthF() + incW);

    QPainterPath p;

    //获取线条的填充区域路径(线条的宽度会形成一个填充区域，这个区域是实际显示是用的pen颜色的区域)
    if (penStrokerShape) {
        p = ps.createStroke(orgPath);
    } else {
        p = ps.createStroke(orgPath) + orgPath;
    }
    if (doSimplified) {
        p = p.simplified();
    }

    return p;
}

VectorItem::VectorItem(PageItem *parent): PageItem(parent), VectorItem_d(new VectorItem_private(this))
{
    setAutoCache(false);
}

SAttrisList VectorItem::attributions()
{
    SAttrisList result;

    result <<  SAttri(EBrushColor, brush().color())
           <<  SAttri(EPenColor, pen().color())
           <<  SAttri(EPenWidth, pen().width())
           <<  SAttri(ERotProperty,  drawRotation())
           <<  SAttri(EEnableBrushStyle,  d_VectorItem()->getDrawFill())
           <<  SAttri(EEnablePenStyle,  d_VectorItem()->getDrawBorder())
           <<  SAttri(EStyleProper, QVariant());

    return result;
}

void VectorItem::setAttributionVar(int attri, const QVariant &var, int phase)
{
    bool isPreview = (phase == EChangedBegin || phase == EChangedUpdate);
    switch (attri) {
    case  EPenColor: {
        setPenColor(var.value<QColor>(), isPreview);
        break;
    }
    case  EBrushColor: {
        setBrushColor(var.value<QColor>(), isPreview);
        break;
    }
    case  EBorderWidth: {
        setPenWidth(var.toInt(), isPreview);
        break;
    }
    case  EPenWidth: {
        setPenWidth(var.toInt(), isPreview);
        break;
    }
    case EEnableBrushStyle: {
        d_VectorItem()->setDrawFill(var.toInt() != Qt::Unchecked);
        break;
    }
    case EEnablePenStyle: {
        d_VectorItem()->setDrawBorder(var.toInt() != Qt::Unchecked);
        break;
    }
    default:
        break;
    }

    PageItem::setAttributionVar(attri, var, phase);
}

QPen VectorItem::pen() const
{
    return d_VectorItem()->pen;
}

void VectorItem::setPen(const QPen &pen)
{
    d_VectorItem()->pen = pen;

    // 重新获取缓存数据
    if (isCached()) {
        d_VectorItem()->resetCachePixmap();
    }
}

QBrush VectorItem::brush() const
{
    return d_VectorItem()->brush;
}

void VectorItem::setBrush(const QBrush &brush)
{
    d_VectorItem()->brush = brush;
    // 重新获取缓存数据
    if (isCached()) {
        d_VectorItem()->resetCachePixmap();
    }
    this->update();
}
void VectorItem::setPenColor(const QColor &c, bool isPreview)
{
    if (isPreview) {
        d_VectorItem()->m_penPreviewColor = c;
    } else {
        QPen p = pen();
        p.setColor(c);
        setPen(p);
    }
    d_VectorItem()->m_isPreviewCom[0] = isPreview;

    // 重新获取缓存数据
    if (isCached() && !isPreview) {
        d_VectorItem()->resetCachePixmap();
    }
    update();
}

void VectorItem::setPenWidth(int w, bool isPreview)
{
    if (isPreview) {
        d_VectorItem()->m_penWidth = w;
    } else {
        QPen p = pen();
        p.setWidth(w);
        p.setJoinStyle(Qt::MiterJoin);
        p.setStyle(Qt::SolidLine);
        p.setCapStyle(Qt::RoundCap);
        d_VectorItem()->pen = p;
        updateShape();
    }
    d_VectorItem()->m_isPreviewCom[1] = isPreview;

    // 重新获取缓存数据
    if (isCached() && !isPreview) {
        d_VectorItem()->resetCachePixmap();
    }
    update();
}

void VectorItem::setBrushColor(const QColor &c, bool isPreview)
{
    if (isPreview) {
        d_VectorItem()->m_brPreviewColor = c;
    } else {
        QBrush br = brush();
        br.setStyle(Qt::SolidPattern);
        br.setColor(c);
        d_VectorItem()->brush = br;
    }
    d_VectorItem()->m_isPreviewCom[2] = isPreview;
    // 重新获取缓存数据
    if (isCached() && !isPreview) {
        d_VectorItem()->resetCachePixmap();
    }
    update();
}

QBrush VectorItem::paintBrush() const
{
    QBrush br = brush();
    if (d_VectorItem()->m_isPreviewCom[2]) {
        br.setColor(d_VectorItem()->m_brPreviewColor);
    }
    return br;
}

QPen VectorItem::paintPen(Qt::PenJoinStyle style) const
{
    QPen p = pen();
    if (d_VectorItem()->m_isPreviewCom[0]) {
        p.setColor(d_VectorItem()->m_penPreviewColor);
    }
    if (d_VectorItem()->m_isPreviewCom[1]) {
        p.setWidth(d_VectorItem()->m_penWidth);
    }
    p.setJoinStyle(style);
    p.setCapStyle(Qt::RoundCap);
    return p;
}

QRectF VectorItem::itemRect() const
{
    return d_VectorItem()->outerRect;
}

QPainterPath VectorItem::itemShape() const
{
    return d_VectorItem()->outerPath;
}

QRectF VectorItem::orgRect() const
{
    return d_VectorItem()->orgRect;
}

void VectorItem::setCache(bool enable)
{
    d_VectorItem()->_useCachePixmap = enable;
    if (d_VectorItem()->_useCachePixmap) {
        if (d_VectorItem()->_cachePixmap == nullptr) {
            d_VectorItem()->_cachePixmap = new QPixmap;
            *(d_VectorItem()->_cachePixmap) = d_VectorItem()->getCachePixmap();
        }
    } else {
        if (d_VectorItem()->_cachePixmap != nullptr) {
            setAutoCache(false);
            delete d_VectorItem()->_cachePixmap;
            d_VectorItem()->_cachePixmap = nullptr;
        }
    }
}

bool VectorItem::isCached() const
{
    return (d_VectorItem()->_useCachePixmap && d_VectorItem()->_cachePixmap != nullptr);
}

void VectorItem::setAutoCache(bool autoCache, int autoCacheMs)
{
    d_VectorItem()->_autoCache = autoCache;
    d_VectorItem()->_autoEplMs = autoCacheMs;
    update();
}

bool VectorItem::isAutoCache() const
{
    return d_VectorItem()->_autoCache;
}

QPainterPath VectorItem::orgShape() const
{
    return d_VectorItem()->orgPath;
}

QPainterPath VectorItem::penStrokerShape() const
{
    return d_VectorItem()->penStrokerPath;
}

QImage VectorItem::rasterSelf() const
{
    return d_VectorItem()->getCachePixmap(false).toImage();
}

bool VectorItem::isPosPenetrable(const QPointF &posLocal)
{
    if (page() != nullptr) {
        int tool = page()->currentTool();
        if (tool == blur) {
            return false;
        }
    }
    bool result = false;
    bool brushIsTrans = brush().color().alpha() == 0 ? true : false;
    bool penIsTrans = (pen().color().alpha() == 0 || pen().width() == 0) ? true : false;

    if (brushIsTrans && penIsTrans) {
        return false;
    }

    if (penStrokerShape().contains(posLocal)) {
        result = penIsTrans;
    } else {
        result = brushIsTrans;
    }
    return result;
}

//bool VectorItem::contains(const QPointF &point) const
//{
//    if (penStrokerShape().contains(point)) {
//        return true;
//    } else {
//        qreal inLenth = /*this->incLength()*/0;
//        bool isInInSide = selfOrgShape().intersects(QRectF(point - QPointF(inLenth, inLenth),
//                                                           point + QPointF(inLenth, inLenth)));
//        if (isInInSide)
//            return true;
//    }
//    return false;
//}

bool VectorItem::isRectPenetrable(const QRectF &rectLocal)
{
    bool isPenetrable = true;
    QPainterPath shape = this->shape();
    if (shape.contains(rectLocal)) {
        bool brushIsTrans = brush().color().alpha() == 0 ? true : false;
        isPenetrable = brushIsTrans;
    } else if (shape.intersects(rectLocal)) {
        isPenetrable = false;
    }
    return isPenetrable;
}
void VectorItem::updateShape()
{
    preparePageItemGeometryChange();

    auto olOrgRect = d_VectorItem()->orgRect;
    //org base path and rect.
    d_VectorItem()->orgPath   = getOrgShape();
    d_VectorItem()->orgRect   = d_VectorItem()->orgPath.controlPointRect();

    d_VectorItem()->penStrokerPath = getPenStrokerShape();
    d_VectorItem()->outerRect = d_VectorItem()->penStrokerPath.controlPointRect();

    if (this->operatingType() == -1) {
        //outline path and outline rect.
        d_VectorItem()->outerPath = getOuterShape(d_VectorItem()->orgPath, d_VectorItem()->penStrokerPath);
        d_VectorItem()->outerRect = d_VectorItem()->outerPath.controlPointRect();

        if (isCached())
            d_VectorItem()->resetCachePixmap();
    }

    PageItem::updateShape();

    if (olOrgRect != d_VectorItem()->orgRect)
        notifyChanged(PageItemRectChanged, d_VectorItem()->orgRect);
}

QPainterPath VectorItem::highLightPath() const
{
    return orgShape();
}

bool VectorItem::isDrawFill()
{
    return  d_VectorItem()->getDrawFill();
}

bool VectorItem::isDarwBorder()
{
    return d_VectorItem()->getDrawBorder();
}

void VectorItem::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    beginCheckIns(painter);

    const QPen curPen = this->paintPen();
    QPen pen = (curPen.width() == 0 || !d_VectorItem()->getDrawBorder()) ? Qt::NoPen : curPen;
    painter->setPen(pen);
    QBrush brush = d_VectorItem()->getDrawFill() ? this->paintBrush() : QBrush(Qt::NoBrush);
    painter->setBrush(brush);
    painter->drawPath(orgShape());

    endCheckIns(painter);

    paintMutBoundingLine(painter, option);
}
void VectorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    if (isCached()) {
        beginCheckIns(painter);
        d_VectorItem()->_curStyleOption = *option;
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::SmoothPixmapTransform);

        painter->save();

        //初始化翻转信息
        //painter->setTransform(getFilpTransform(), true);

        //绘制缓冲图
        //qWarning() << d_VectorItem()->_cachePixmap->size() << "selfRect() = " << selfRect().size();
        painter->drawPixmap(itemRect(),
                            *(d_VectorItem()->_cachePixmap),
                            QRectF(0, 0, d_VectorItem()->_cachePixmap->width(),
                                   d_VectorItem()->_cachePixmap->height()));

        painter->restore();

        paintMutBoundingLine(painter, option);

        endCheckIns(painter);
    } else {
        QElapsedTimer *time = nullptr;
        if (d_VectorItem()->_autoCache) {
            time = new QElapsedTimer;
            time->start();
        }
        //绘制图元
        paintItemSelf(painter, option, EPaintForNoCache);

        if (d_VectorItem()->_autoCache && time) {
            int elp = time->elapsed();
            this->setCache(elp > d_VectorItem()->_autoEplMs);
            delete time;
        }
    }
    paintChidren(painter, childPageItems(), widget);
}

QVariant VectorItem::pageItemChange(int change, const QVariant &value)
{
    return PageItem::pageItemChange(change, value);
}


void VectorItem::paintItemSelf(QPainter *painter, const QStyleOptionGraphicsItem *option,
                               EPaintReson paintReson)
{
    if (paintReson == EPaintForNoCache)
        beginCheckIns(painter);

    painter->save();

    //当是动态绘制(无缓冲绘制)绘制时才实现图片的翻转(缓冲绘制是绘制到Pixmap上,需要获取到原图)
    if (paintReson == EPaintForNoCache) {
        //painter->setTransform(getFilpTransform(), true);
    }

    //绘制自身
    paintSelf(painter, option);

    if (paintReson == EPaintForNoCache) {
        //绘制模糊特效
        //paintAllBlur(painter);
    }

    painter->restore();

    if (paintReson == EPaintForNoCache)
        endCheckIns(painter);

    if (paintReson == EPaintForNoCache)
        paintMutBoundingLine(painter, option);
}

void VectorItem::loadVectorData(const UnitHead &head)
{
    this->setPen(head.pen);
    this->setBrush(head.brush);
    PageItem::loadHeadData(head);
}

QPainterPath VectorItem::getPenStrokerShape() const
{
    return getPenStrokerPath(orgShape(), pen(), true, 0, false);
}

QPainterPath VectorItem::getOuterShape(const QPainterPath &orgPath, const QPainterPath &strokerPath) const
{
    return (orgPath + strokerPath).simplified();
}
