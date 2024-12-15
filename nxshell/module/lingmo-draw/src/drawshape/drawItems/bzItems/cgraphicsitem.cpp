// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cgraphicsitem.h"
#include "cgraphicstextitem.h"
#include "cgraphicsproxywidget.h"
#include "cdrawscene.h"
#include "widgets/ctextedit.h"
#include "frame/cgraphicsview.h"
#include "cviewmanagement.h"
#include "drawItems/cgraphicsitemselectedmgr.h"
#include "application.h"
#include "global.h"
#include "cgraphicsitemevent.h"
#include "../../../service/dyncreatobject.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QVariant>
#include <QtMath>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>

#include "cgraphicscutitem.h"
#include "cgraphicsellipseitem.h"
#include "cgraphicslineitem.h"
#include "cgraphicspenitem.h"
#include "cgraphicspolygonalstaritem.h"
#include "cgraphicspolygonitem.h"
#include "cgraphicstextitem.h"
#include "cgraphicstriangleitem.h"
#include "cpictureitem.h"
#include "cattributeitemwidget.h"
#include "cgraphicslayer.h"
#include "blurwidget.h"

REGISTITEMCLASS(CGraphicsPenItem, PenType)
REGISTITEMCLASS(CPictureItem, PictureType)
class CGraphicsItem::CGraphicsItem_private
{
public:
    explicit CGraphicsItem_private(CGraphicsItem *pItem): _item(pItem) {}
    static QMap<int, QString> &registedClassNameMap()
    {
        static QMap<int, QString> *s_map = nullptr;
        if (s_map == nullptr) {
            s_map = new QMap<int, QString>;
        }
        return *s_map;
    }

private:
    CGraphicsItem *_item;
};

const int inccW = 10;
bool CGraphicsItem::paintInteractBorderLine = true;
QPainterPath CGraphicsItem::getGraphicsItemShapePathByOrg(const QPainterPath &orgPath,
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

CGraphicsItem *CGraphicsItem::creatItemInstance(int itemType, const CGraphicsUnit &data)
{
    CGraphicsItem *item = nullptr;
    auto registerItems = CGraphicsItem_private::registedClassNameMap();
    auto itf = registerItems.find(itemType);
    if (itf != registerItems.end()) {
        item =  ClassObjectFactory::creatObject<CGraphicsItem>(itf.value());
    }

    if (item != nullptr)
        item->loadGraphicsUnit(data);

    if ((PictureType == itemType || itemType == PenType) && item != nullptr) {
        QPixmap pix = item->rasterSelf();
        auto jLay = new JDynamicLayer(pix.toImage());
        auto gemo = new JGeomeCommand(item->pos(), item->drawRotation(), item->zValue(), item->boundingRectTruly(), item->transform());
        gemo->setLayer(jLay);
        jLay->appendComand(gemo, true, false);

        jLay->setBlocked(false);
        if (itemType == PictureType) {
            jLay->setBlocked(true);
            jLay->setLayerType(JDynamicLayer::EImageType);
        }

        delete item;
        item = jLay;
    }

    return item;
}

CGraphicsItem *CGraphicsItem::zItem(const QList<CGraphicsItem *> &pBzItems, int wantZitemTp)
{
    if (pBzItems.isEmpty())
        return nullptr;
    CGraphicsItem *result = nullptr;

    QList<CGraphicsItem *> allBzItems;

    for (auto p : pBzItems) {
        if (p->isBzGroup()) {
            allBzItems.append(static_cast<CGraphicsItemGroup *>(p)->getBzItems(true));
        } else if (p->isBzItem()) {
            allBzItems.append(p);
            //qDebug() << "zzzzzz = " << p->zValue();
        }
    }

    if (wantZitemTp == -1) {
        result = PageScene::returnSortZItems(allBzItems, PageScene::EAesSort).first();
    } else if (wantZitemTp == -2) {
        result = PageScene::returnSortZItems(allBzItems, PageScene::EDesSort).first();
    }
    return result;
}


CGraphicsItem::CGraphicsItem(QGraphicsItem *parent)
    : QAbstractGraphicsShapeItem(parent)
    , _flipHorizontal(false) // 水平翻转
    , _flipVertical(false)  // 垂直翻转
{
    setAutoCache(false);
    _pPrivate = new CGraphicsItem_private(this);
}

CGraphicsItem::~CGraphicsItem()
{
    if (_cachePixmap != nullptr) {
        delete _cachePixmap;
        _cachePixmap = nullptr;
    }
    delete _pPrivate;
    _pPrivate = nullptr;
}

void CGraphicsItem::registerItem(const QString &classname, int classType)
{
    qWarning() << "classname = " << classname << "classType = " << classType;
    CGraphicsItem_private::registedClassNameMap().insert(classType, classname);
}

void CGraphicsItem::setScene(QGraphicsScene *scene, bool calZ)
{
    if (scene == nullptr) {
        auto pScene = this->scene();
        if (pScene == nullptr)
            return;
        else {
            auto pCScene = drawScene();
            if (pCScene != nullptr) {
                pCScene->removeCItem(this);
            } else {
                pScene->removeItem(this);
            }
        }
    } else {
        PageScene *pNewScene = qobject_cast<PageScene *>(scene);

        PageScene *pScene = this->drawScene();
        if (pScene == pNewScene)
            return;

        if (pScene != nullptr)
            pScene->removeCItem(this);

        if (pNewScene != nullptr) {
            pNewScene->addCItem(this, calZ);
        }
    }
}

DrawAttribution::SAttrisList CGraphicsItem::attributions()
{
    DrawAttribution::SAttrisList result;
    result << DrawAttribution::SAttri(DrawAttribution::EBrushColor, brush().color())
           << DrawAttribution::SAttri(DrawAttribution::EPenColor, pen().color())
           << DrawAttribution::SAttri(DrawAttribution::EBorderWidth,  pen().width());
    return result;
}

void CGraphicsItem::setAttributionVar(int attri, const QVariant &var, int phase)
{
    bool isPreview = (phase == EChangedBegin || phase == EChangedUpdate);
    switch (attri) {
    case DrawAttribution::EPenColor: {
        setPenColor(var.value<QColor>(), isPreview);
        break;
    }
    case DrawAttribution::EBrushColor: {
        setBrushColor(var.value<QColor>(), isPreview);
        break;
    }
    case DrawAttribution::EBorderWidth: {
        setPenWidth(var.toInt(), isPreview);
        break;
    }
    default:
        break;
    };
}

PageView *CGraphicsItem::curView() const
{
    PageView *parentView = nullptr;
    if (scene() != nullptr) {
        if (!scene()->views().isEmpty()) {
            parentView = dynamic_cast<PageView *>(scene()->views().first());
        }
    }
    return parentView;
}

PageScene *CGraphicsItem::drawScene() const
{
    if (scene() == nullptr)
        return nullptr;
    return qobject_cast<PageScene *>(scene());
}

Page *CGraphicsItem::page() const
{
    if (curView() != nullptr)
        return curView()->page();

    return nullptr;
}

void CGraphicsItem::setPenColor(const QColor &c, bool isPreview)
{
    if (isPreview) {
        m_penPreviewColor = c;
    } else {
        QPen p = pen();
        p.setColor(c);
        setPen(p);
    }
    m_isPreviewCom[0] = isPreview;

    // 重新获取缓存数据
    if (isCached() && !isPreview) {
        resetCachePixmap();
    }
    update();
}

void CGraphicsItem::setPenWidth(int w, bool isPreview)
{
    if (isPreview) {
        m_penWidth = w;
    } else {
        QPen p = pen();
        p.setWidth(w);
        p.setJoinStyle(Qt::MiterJoin);
        p.setStyle(Qt::SolidLine);
        p.setCapStyle(Qt::RoundCap);
        setPen(p);
    }
    m_isPreviewCom[1] = isPreview;

    updateShapeRecursion();
    // 重新获取缓存数据
    if (isCached() && !isPreview) {
        resetCachePixmap();
    }
    update();
}

void CGraphicsItem::setBrushColor(const QColor &c, bool isPreview)
{
    if (isPreview) {
        m_brPreviewColor = c;
    } else {
        QBrush br = brush();
        br.setStyle(Qt::SolidPattern);
        br.setColor(c);
        setBrush(br);
    }
    m_isPreviewCom[2] = isPreview;
    // 重新获取缓存数据
    if (isCached() && !isPreview) {
        resetCachePixmap();
    }
    update();
}

QBrush CGraphicsItem::paintBrush() const
{
    QBrush br = brush();
    if (m_isPreviewCom[2]) {
        br.setColor(m_brPreviewColor);
    }
    return br;
}

QPen CGraphicsItem::paintPen(Qt::PenJoinStyle style) const
{
    QPen p = pen();
    if (m_isPreviewCom[0]) {
        p.setColor(m_penPreviewColor);
    }
    if (m_isPreviewCom[1]) {
        p.setWidth(m_penWidth);
    }
    p.setJoinStyle(style);
    return p;
}

QColor CGraphicsItem::systemThemeColor() const
{
    if (drawScene() != nullptr) {
        return drawScene()->systemThemeColor();
    }
    return QColor();
}

bool CGraphicsItem::isMutiSelected() const
{
    auto parentGp = bzGroup(false);
    if (parentGp != nullptr && parentGp->groupType() == CGraphicsItemGroup::ESelectGroup) {
        return (parentGp->count() > 1);
    }
    return false;
}

bool CGraphicsItem::isSelected() const
{
    auto parentGp = bzGroup(false);
    if (parentGp != nullptr && parentGp->groupType() == CGraphicsItemGroup::ESelectGroup) {
        return true;
    }
    return false;
}

QPainterPath CGraphicsItem::getHighLightPath()
{
    return selfOrgShape();
}

void CGraphicsItem::loadHeadData(const SGraphicsUnitHead &head)
{
    this->setPen(head.pen);
    this->setBrush(head.brush);

    _roteAgnel = head.rotate;
    this->setPos(head.pos);
    this->setZValue(head.zValue);
    _blurInfos = head.blurInfos;
    this->setTransform(head.trans);

    updateShape();

    if (isCached()) {
        updateBlurPixmapBySelfBlurInfo();
        resetCachePixmap();
    }
}

void CGraphicsItem::updateHandlesGeometry()
{
}

int CGraphicsItem::type() const
{
    return Type;
}

bool CGraphicsItem::isBzItem() const
{
    return (this->type() > NoType && this->type() < MgrType);
}

bool CGraphicsItem::isSizeHandleExisted()
{
    return !m_handles.isEmpty();
}

void CGraphicsItem::doFilp(CPictureItem::EFilpDirect dir)
{
    if (dir == EFilpHor) {
        this->_flipHorizontal = !this->_flipHorizontal;
    } else if (dir == EFilpVer) {
        this->_flipVertical = !this->_flipVertical;
    }
    update();
}
void CGraphicsItem::setRotation90(bool leftOrRight)
{
    QPointF center = this->boundingRect().center();
    qreal angle = 0.0;

    // 左旋转减小 右旋转增大
    if (leftOrRight == true) {
        angle = - 90.0;
    } else {
        angle = 90.0;
    }

    // 矩阵变换
    QTransform trans;
    trans.translate(center.x(), center.y());
    trans.rotate(angle);
    trans.translate(-center.x(), -center.y());

    // 设置当前旋转角度
    setDrawRotatin(angle + drawRotation());
    setTransform(trans, true);

    updateShapeRecursion();
}

void CGraphicsItem::setFilpBaseOrg(CPictureItem::EFilpDirect dir, bool b)
{
    if (dir == EFilpHor) {
        if (this->_flipHorizontal != b) {
            doFilp(dir);
        }
    } else if (dir == EFilpVer) {
        if (this->_flipVertical != b) {
            doFilp(dir);
        }
    }
}

bool CGraphicsItem::isFilped(CPictureItem::EFilpDirect dir) const
{
    return (dir == EFilpHor ? this->_flipHorizontal : this->_flipVertical);
}

CSizeHandleRect::EDirection CGraphicsItem::hitTest(const QPointF &point) const
{
    const Handles::const_iterator hend =  m_handles.end();
    for (Handles::const_iterator it = m_handles.begin(); it != hend; ++it) {
        if ((*it)->hitTest(point)) {
            return (*it)->dir();
        }
    }
    //检测是否在矩形内
    QPointF pt = mapFromScene(point);
    if (this->shape().contains(pt)) {
        return CSizeHandleRect::InRect;
    }

    return CSizeHandleRect::None;
}

QPainterPath CGraphicsItem::getSelfOrgShape() const
{
    QPainterPath path;
    path.addRect(rect());
    path.closeSubpath();
    return path;
}

QPainterPath CGraphicsItem::getPenStrokerShape() const
{
    return getGraphicsItemShapePathByOrg(selfOrgShape(), pen(), true, this->incLength());
}

QPainterPath CGraphicsItem::getShape() const
{
    return getGraphicsItemShapePathByOrg(selfOrgShape(), pen(), false, this->incLength());
}

QPainterPath CGraphicsItem::getTrulyShape() const
{
    return getGraphicsItemShapePathByOrg(selfOrgShape(), pen(), true, 0, false);
}

QPixmap CGraphicsItem::getCachePixmap(bool baseOrg)
{
    qreal devecePixelRatio = curView() != nullptr ? curView()->devicePixelRatio() : qApp->devicePixelRatio();

    QSizeF size = boundingRectTruly().size();
    QPixmap pix((size * devecePixelRatio).toSize());

    pix.setDevicePixelRatio(devecePixelRatio);

    pix.fill(QColor(0, 0, 0, 0));

    QPainter painter(&pix);

    painter.translate(-boundingRectTruly().topLeft());

    painter.setRenderHint(QPainter::Antialiasing);

    paintItemSelf(&painter, &_curStyleOption, EPaintForCache);

    //qDebug() << "getCachePixmap---------------------- " << baseOrg;
    if (!baseOrg) {
        paintAllBlur(&painter);
    }

    return pix;
}

void CGraphicsItem::changeTransCenterTo(const QPointF &newCenter)
{
    setTransformOriginPoint(newCenter);
}

QRectF CGraphicsItem::boundingRect() const
{
    return m_boundingRect;
}

QRectF CGraphicsItem::selectedBoundingRect() const
{
    return rect();
}

QRectF CGraphicsItem::boundingRectTruly() const
{
    return m_boundingRectTrue;
}

QPainterPath CGraphicsItem::shape() const
{
    return m_boundingShape;
}

void CGraphicsItem::setCache(bool enable)
{
    _useCachePixmap = enable;
    if (_useCachePixmap) {
        if (_cachePixmap == nullptr) {
            _cachePixmap = new QPixmap;
            *_cachePixmap = getCachePixmap();
        }

    } else {
        if (_cachePixmap != nullptr) {
            setAutoCache(false);
            delete _cachePixmap;
            _cachePixmap = nullptr;
        }
    }
}

bool CGraphicsItem::isCached() const
{
    return (_useCachePixmap && _cachePixmap != nullptr);
}

void CGraphicsItem::setAutoCache(bool autoCache, int autoCacheMs)
{
    _autoCache = autoCache;
    _autoEplMs = autoCacheMs;
    update();
}

// Cppcheck检测函数没有使用到
//bool CGraphicsItem::isAutoCache() const
//{
//    return _autoCache;
//}

QPainterPath CGraphicsItem::selfOrgShape() const
{
    return m_selfOrgPathShape;
}

QPainterPath CGraphicsItem::penStrokerShape() const
{
    return m_penStroerPathShape;
}

QPointF CGraphicsItem::getCenter(CSizeHandleRect::EDirection dir)
{
    QPointF center;
    CGraphicsItem *pItem = this;
    QRectF rect = pItem->rect();
    switch (dir) {
    case CSizeHandleRect::LeftTop:
        center = rect.bottomRight();
        break;
    case CSizeHandleRect::Top:
        center = QPointF(rect.center().x(), rect.bottom());
        break;
    case CSizeHandleRect::RightTop:
        center = rect.bottomLeft();
        break;
    case CSizeHandleRect::Right:
        center = QPointF(rect.left(), rect.center().y());
        break;
    case CSizeHandleRect::RightBottom:
        center = rect.topLeft();
        break;
    case CSizeHandleRect::Bottom:
        center = QPointF(rect.center().x(), rect.top());
        break;
    case CSizeHandleRect::LeftBottom:
        center = rect.topRight();
        break;
    case CSizeHandleRect::Left:
        center = QPointF(rect.right(), rect.center().y());
        break;
    case CSizeHandleRect::Rotation:
        center = rect.center();
        break;
    default:
        center = rect.center();
        break;
    }
    return center;
}

CGraphicsLayer *CGraphicsItem::layer()
{
    //return dynamic_cast<CGraphicsLayer *>(parentItem());
    return _layer;
}

void CGraphicsItem::setLayer(CGraphicsLayer *layer)
{
    if (layer == _layer)
        return;

    if (_layer != nullptr) {
        _layer->removeCItem(this);
    }

    if (layer != nullptr) {
        layer->addCItem(this);
    }
    _layer = layer;
}

bool CGraphicsItem::isBzGroup(int *groupTp) const
{
    bool result = (this->type() == MgrType);
    if (result && groupTp != nullptr) {
        *groupTp = static_cast<const CGraphicsItemGroup *>(this)->groupType();
    }
    return result;
}

CGraphicsItemGroup *CGraphicsItem::bzGroup(bool onlyNormal) const
{
    if (onlyNormal) {
        if (PageScene::isNormalGroupItem(_pGroup)) {
            return _pGroup;
        }
        return nullptr;
    }
    return _pGroup;
}

CGraphicsItemGroup *CGraphicsItem::bzTopGroup(bool onlyNormal) const
{
    auto fPrecondition = [ = ](CGraphicsItemGroup * p) {
        if (!onlyNormal) {return p != nullptr;}
        return PageScene::isNormalGroupItem(p);
    };

    CGraphicsItemGroup *pTopGroup = bzGroup();

    if (fPrecondition(pTopGroup)) {
        while (true) {
            CGraphicsItemGroup *nextBzGroup = pTopGroup->bzGroup();
            if (!fPrecondition(nextBzGroup)) {
                break;
            }
            pTopGroup = pTopGroup->bzGroup();
        }
    } else {
        pTopGroup = nullptr;
    }

    return pTopGroup;
}

CGraphicsItem *CGraphicsItem::thisBzProxyItem(bool topleve)
{
    CGraphicsItemGroup *pTg = topleve ? bzTopGroup() : bzGroup();
    if (pTg != nullptr)
        return pTg;

    return this;
}

void CGraphicsItem::setBzGroup(CGraphicsItemGroup *pGroup)
{
    //防止添加自己引起循环
    if (pGroup == this)
        return;

    //判断是否相等可终结循环
    if (pGroup == _pGroup)
        return;

    //先从原来的组合中删除
    if (_pGroup != nullptr) {
        _pGroup->remove(this);
    }
    //再添加到新的组合中
    if (pGroup != nullptr) {
        pGroup->add(this);
    }
    _pGroup = pGroup;
}

qreal CGraphicsItem::drawZValue()
{
    return zValue();
}

bool CGraphicsItem::contains(const QPointF &point) const
{
    if (penStrokerShape().contains(point)) {
        return true;
    } else {
        qreal inLenth = this->incLength();
        bool isInInSide = selfOrgShape().intersects(QRectF(point - QPointF(inLenth, inLenth),
                                                           point + QPointF(inLenth, inLenth)));
        if (isInInSide)
            return true;
    }
    return false;
}

//void CGraphicsItem::setBzZValue(qreal z)
//{
//    this->setZValue(z);
//}

bool CGraphicsItem::isPosPenetrable(const QPointF &posLocal)
{
    if (curView() != nullptr) {
        int tool = drawScene()->page()->currentTool();
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

bool CGraphicsItem::isRectPenetrable(const QRectF &rectLocal)
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

int CGraphicsItem::operatingType()
{
    return m_operatingType;
}

void CGraphicsItem::operatingBegin(CGraphItemEvent *event)
{
    m_operatingType = event->toolEventType();
    event->setItem(this);
    switch (event->type()) {
    case CGraphItemEvent::EMove: {
        CGraphItemMoveEvent *movEvt = static_cast<CGraphItemMoveEvent *>(event);
        doMoveBegin(movEvt);
        break;
    }
    case CGraphItemEvent::EScal: {
        CGraphItemScalEvent *sclEvt = static_cast<CGraphItemScalEvent *>(event);
        doScalBegin(sclEvt);
        break;
    }
    case CGraphItemEvent::ERot: {
        CGraphItemRotEvent *rotEvt = static_cast<CGraphItemRotEvent *>(event);
        this->changeTransCenterTo(event->centerPos());
        doRotBegin(rotEvt);
        break;
    }
    case CGraphItemEvent::EBlur: {
        break;
    }
    default:
        break;
    }
}

void CGraphicsItem::operating(CGraphItemEvent *event)
{
    event->setItem(this);
    switch (event->type()) {
    case CGraphItemEvent::EMove: {
        CGraphItemMoveEvent *movEvt = static_cast<CGraphItemMoveEvent *>(event);
        doMoving(movEvt);
        break;
    }
    case CGraphItemEvent::EScal: {
        //qWarning() << "do scal------------------------";
        CGraphItemScalEvent *sclEvt = static_cast<CGraphItemScalEvent *>(event);
        doScaling(sclEvt);
        //刷新模糊路径特效
        if (!_blurInfos.isEmpty()) {
            QTransform trans = sclEvt->trans();
            foreach (SBlurInfo info, _blurInfos) {
                info.blurPath = trans.map(info.blurPath);
            }
            if (!isCached())
                updateBlurPixmapBySelfBlurInfo();
        }
        break;
    }
    case CGraphItemEvent::ERot: {
        CGraphItemRotEvent *rotEvt = static_cast<CGraphItemRotEvent *>(event);
        doRoting(rotEvt);
        break;
    }
    case CGraphItemEvent::EBlur: {
        break;
    }
    default:
        break;
    }
}

bool CGraphicsItem::testOpetating(CGraphItemEvent *event)
{
    event->setItem(this);
    bool accept = false;
    switch (event->type()) {
    case CGraphItemEvent::EMove: {
        CGraphItemMoveEvent *movEvt = static_cast<CGraphItemMoveEvent *>(event);
        accept = testMoving(movEvt);
        break;
    }
    case CGraphItemEvent::EScal: {
        CGraphItemScalEvent *sclEvt = static_cast<CGraphItemScalEvent *>(event);
        accept = testScaling(sclEvt);
        break;
    }
    case CGraphItemEvent::ERot: {
        CGraphItemRotEvent *rotEvt = static_cast<CGraphItemRotEvent *>(event);
        accept = testRoting(rotEvt);
        break;
    }
    case CGraphItemEvent::EBlur: {
        break;
    }
    default:
        break;
    }
    return accept;
}

void CGraphicsItem::operatingEnd(CGraphItemEvent *event)
{
    m_operatingType = -1;
    event->setItem(this);
    switch (event->type()) {
    case CGraphItemEvent::EMove: {
        CGraphItemMoveEvent *movEvt = static_cast<CGraphItemMoveEvent *>(event);
        doMoveEnd(movEvt);
        break;
    }
    case CGraphItemEvent::EScal: {
        CGraphItemScalEvent *sclEvt = static_cast<CGraphItemScalEvent *>(event);
        doScalEnd(sclEvt);
        updateBlurPixmapBySelfBlurInfo();
        if (isCached()) {
            *_cachePixmap = getCachePixmap();
        }
        break;
    }
    case CGraphItemEvent::ERot: {
        CGraphItemRotEvent *rotEvt = static_cast<CGraphItemRotEvent *>(event);
        doRotEnd(rotEvt);
        break;
    }
    case CGraphItemEvent::EBlur: {
        break;
    }
    default:
        break;
    }
}

void CGraphicsItem::doMoveBegin(CGraphItemMoveEvent *event)
{
    Q_UNUSED(event)
}

void CGraphicsItem::doMoving(CGraphItemMoveEvent *event)
{
    QPointF move = event->_scenePos - event->_oldScenePos;
    this->moveBy(move.x(), move.y());
}

bool CGraphicsItem::testMoving(CGraphItemMoveEvent *event)
{
    Q_UNUSED(event)
    return true;
}

void CGraphicsItem::doMoveEnd(CGraphItemMoveEvent *event)
{
    Q_UNUSED(event)
}

void CGraphicsItem::doScalBegin(CGraphItemScalEvent *event)
{
    Q_UNUSED(event);
}

void CGraphicsItem::doScaling(CGraphItemScalEvent *event)
{
    Q_UNUSED(event)
}

bool CGraphicsItem::testScaling(CGraphItemScalEvent *event)
{
    Q_UNUSED(event)
    return true;
}

void CGraphicsItem::doScalEnd(CGraphItemScalEvent *event)
{
    Q_UNUSED(event)
}

void CGraphicsItem::doRotBegin(CGraphItemRotEvent *event)
{
    Q_UNUSED(event);
}

void CGraphicsItem::doRoting(CGraphItemRotEvent *event)
{
    QPointF center = this->transformOriginPoint();
    QLineF l1 = QLineF(center, event->oldPos());
    QLineF l2 = QLineF(center, event->pos());
    qreal angle = l2.angle() - l1.angle();
    QTransform trans;
    trans.translate(center.x(), center.y());
    trans.rotate(-angle);
    trans.translate(-center.x(), -center.y());
    _roteAgnel += -angle;
    int n = int(_roteAgnel) / 360;
    _roteAgnel = _roteAgnel - n * 360;
    if (_roteAgnel < 0) {
        _roteAgnel += 360;
    }
    setTransform(trans, true);
}

bool CGraphicsItem::testRoting(CGraphItemRotEvent *event)
{
    Q_UNUSED(event)
    return true;
}

void CGraphicsItem::doRotEnd(CGraphItemRotEvent *event)
{
    Q_UNUSED(event)
}

void CGraphicsItem::loadGraphicsUnit(const CGraphicsUnit &data)
{
    loadHeadData(data.head);
}

qreal CGraphicsItem::incLength()const
{
    qreal scal = (curView() == nullptr) ? 1.0 : curView()->getScale();
    return inccW / scal;
}

void CGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    //qDebug() << "_useCachePixmap = " << _useCachePixmap << "_cachePixmap = " << _cachePixmap;
    if (isCached()) {
        beginCheckIns(painter);
        _curStyleOption = *option;
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setRenderHint(QPainter::SmoothPixmapTransform);

        painter->save();

        //初始化翻转信息
        painter->setTransform(getFilpTransform(), true);

        //绘制缓冲图
        painter->drawPixmap(boundingRectTruly(), *_cachePixmap, QRectF(0, 0, _cachePixmap->width(), _cachePixmap->height()));

        painter->restore();

        paintMutBoundingLine(painter, option);

        endCheckIns(painter);
    } else {
        QTime *time = nullptr;
        if (_autoCache) {
            time = new QTime;
            time->start();
        }
        //绘制图元
        paintItemSelf(painter, option, EPaintForNoCache);

        if (_autoCache) {
            int elp = time->elapsed();
            this->setCache(elp > _autoEplMs);
            delete time;
        }
    }
}


void CGraphicsItem::paintItemSelf(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                  EPaintReson paintReson)
{
    if (paintReson == EPaintForNoCache)
        beginCheckIns(painter);

    painter->save();

    //当是动态绘制(无缓冲绘制)绘制时才实现图片的翻转(缓冲绘制是绘制到Pixmap上,需要获取到原图)
    if (paintReson == EPaintForNoCache) {
        painter->setTransform(getFilpTransform(), true);
    }

    //绘制自身
    paintSelf(painter, option);

    if (paintReson == EPaintForNoCache) {
        //绘制模糊特效
        paintAllBlur(painter);
    }

    painter->restore();

    if (paintReson == EPaintForNoCache)
        endCheckIns(painter);

    if (paintReson == EPaintForNoCache)
        paintMutBoundingLine(painter, option);
}

void CGraphicsItem::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    beginCheckIns(painter);

    const QPen curPen = this->paintPen();
    painter->setPen(curPen.width() == 0 ? Qt::NoPen : curPen);
    painter->setBrush(this->paintBrush());
    painter->drawPath(selfOrgShape());

    endCheckIns(painter);

    paintMutBoundingLine(painter, option);
}


void CGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
}

void CGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
}

void CGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
}

void CGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
}

CGraphicsUnit CGraphicsItem::getGraphicsUnit(EDataReason reson) const
{
    Q_UNUSED(reson)
    return CGraphicsUnit();
}

void CGraphicsItem::move(QPointF beginPoint, QPointF movePoint)
{
    QPointF Pos = this->pos();
    this->setPos(Pos + movePoint - beginPoint);
}

void CGraphicsItem::updateShape()
{
    m_selfOrgPathShape   = getSelfOrgShape();
    m_penStroerPathShape = getPenStrokerShape();
    m_boundingShape      = getShape();
    m_boundingRect       = m_boundingShape.controlPointRect();

    m_boundingShapeTrue  = getTrulyShape();
    m_boundingRectTrue   = m_boundingShapeTrue.controlPointRect();

    if (drawScene() != nullptr)
        drawScene()->selectGroup()->updateBoundingRect();
}

//void CGraphicsItem::setSizeHandleRectFlag(CSizeHandleRect::EDirection dir, bool flag)
//{
//    foreach (CSizeHandleRect *sizeHandleRect, m_handles) {
//        if (sizeHandleRect->dir() == dir) {
//            sizeHandleRect->setVisible(flag);
//            break;
//        }
//    }
//}

void CGraphicsItem::setState(ESelectionHandleState st)
{
    const Handles::iterator hend =  m_handles.end();
    for (Handles::iterator it = m_handles.begin(); it != hend; ++it) {

        (*it)->setState(st);
    }
}

void CGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
}


QVariant CGraphicsItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (QGraphicsItem::ItemSceneHasChanged == change) {
        if (this->isBzItem()) {
            QGraphicsScene *pScene = qvariant_cast<QGraphicsScene *>(value);
            if (pScene == nullptr) {
                //1.清除节点
                clearHandle();
            } else {
                //添加到场景中的时候初始化节点
                initHandle();

                //刷新模糊图
                if (!_blurInfos.isEmpty()) {
                    updateBlurPixmapBySelfBlurInfo();
                    resetCachePixmap();
                }
            }
        }
    } else if (QGraphicsItem::ItemZValueHasChanged == change) {
        if (bzGroup(true) != nullptr) {
            bzGroup(true)->markZDirty();
        }
    } else if (QGraphicsItem::ItemSelectedHasChanged == change) {
        bool select = value.toBool();
        if (!select) {
            //this->rasterToSelfLayer();
        }
    }
    return value;
}

void CGraphicsItem::beginCheckIns(QPainter *painter)
{
    if (scene() == nullptr || !rect().isValid())
        return;

    painter->save();
    QRectF sceneRct = scene()->sceneRect();
    QRectF itemRct  = mapToScene(rect()).boundingRect();
    bool hasIntersects = sceneRct.intersects(itemRct);
    if (!hasIntersects) {
        painter->setOpacity(0.2);//透明度设置
    }
    painter->setClipping(hasIntersects);
}

void CGraphicsItem::endCheckIns(QPainter *painter)
{
    if (scene() == nullptr || !rect().isValid())
        return;

    painter->restore();
}

void CGraphicsItem::clearHandle()
{
    foreach (CSizeHandleRect *pItem, m_handles) {
        if (pItem->scene() != nullptr) {
            pItem->scene()->removeItem(pItem);
            delete pItem;
        }
    }
    m_handles.clear();
}

void CGraphicsItem::paintMutBoundingLine(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    if (!paintInteractBorderLine)
        return;

    if (isMutiSelected()) {

        painter->setClipping(false);
        QPen pen;

        painter->setRenderHint(QPainter::Antialiasing, true);

        pen.setWidthF(1 / option->levelOfDetailFromTransform(painter->worldTransform()));

        //选中标识颜色需要加深；目前的色值是#E0E0E0；UI提供色值：#BBBBBB
        pen.setColor(QColor("#BBBBBB"));

        painter->setPen(pen);
        painter->setBrush(QBrush(Qt::NoBrush));
        painter->drawRect(this->/*boundingRectTruly*/rect());
        painter->setClipping(true);
    }


    //test
#ifdef QT_DEBUG
//    QString testString;
//    if (this->isBzItem()) {
//        testString += QString("z = %1 ").arg(zValue());
//    }
//    if (this != drawScene()->selectGroup()) {
//        testString += QString("pos = (%1,%2) ").arg(sceneBoundingRect().x()).arg(sceneBoundingRect().y());
//        testString += QString("pos = (%1,%2) ").arg(rect().x()).arg(rect().y());
//    }
//    painter->setClipping(false);
//    QTextOption txtOption(bzGroup() == nullptr ? (Qt::AlignTop | Qt::AlignLeft) : (Qt::AlignTop | Qt::AlignRight));
//    txtOption.setWrapMode(QTextOption::NoWrap);
//    painter->drawText(boundingRect(), testString, txtOption);

#endif
}

void CGraphicsItem::blurBegin(const QPointF &pos)
{
//    _tempActiveBlurInfo.clear();

//    if (!isCached())
//        setCache(true);

//    //生成当前图源下的模糊图像(有发生过变化都要重新生成模糊)
//    EBlurEffect wantedEf = EBlurEffect(drawApp->defaultAttriVar(this->scene(), DrawAttribution::BlurPenEffect).toInt());
//    if (_blurPix[wantedEf].isNull() || _blurPix[wantedEf].size() != rect().size()) {
//        updateBlurPixmap(true);
//    }

//    _tempActiveBlurInfo.blurEfTp = wantedEf;

//    _tempActiveBlurPath = QPainterPath();
//    QPointF beginPos = getFilpTransform().map(pos);
//    _tempActiveBlurPath.moveTo(beginPos);
}

void CGraphicsItem::blurUpdate(const QPointF &pos, bool optm)
{
//    QPointF newPos = getFilpTransform().map(pos);
//    QPointF oldPos = _tempActiveBlurPath.currentPosition();
//    _tempActiveBlurPath.lineTo(newPos);
//    QPen p;
//    p.setCapStyle(Qt::RoundCap);
//    p.setWidth(drawApp->defaultAttriVar(this->scene(), DrawAttribution::BlurPenWidth).toInt());

//    SBlurInfo blur;

//    if (!optm) {
//        //0.将所有的路径进行重新计算
//        _tempActiveBlurInfo.blurPath = CGraphicsItem::getGraphicsItemShapePathByOrg(_tempActiveBlurPath, p, true, 0, false);
//        blur = _tempActiveBlurInfo;
//    } else {
//        //1.获取新增的模糊路径(通过仅绘制新增的路径进行优化加速)
//        QPainterPath path;
//        path.moveTo(oldPos);
//        path.lineTo(newPos);
//        blur.blurPath = CGraphicsItem::getGraphicsItemShapePathByOrg(path, p, true, 0, false);
//        blur.blurEfTp = _tempActiveBlurInfo.blurEfTp;
//    }

//    //将模糊绘制到缓冲图上
//    QPainter paintToCachePixmap;
//    paintToCachePixmap.begin(_cachePixmap);
//    paintBlur(&paintToCachePixmap, blur, -rect().topLeft());
//    paintToCachePixmap.end();
//    update();
}

void CGraphicsItem::blurEnd()
{
//    if (!_tempActiveBlurPath.isEmpty()) {
//        QPen p;
//        p.setCapStyle(Qt::RoundCap);
//        p.setJoinStyle(Qt::RoundJoin);
//        p.setWidth(drawApp->defaultAttriVar(this->scene(), DrawAttribution::BlurPenWidth).toInt());
//        _tempActiveBlurInfo.blurPath = CGraphicsItem::getGraphicsItemShapePathByOrg(_tempActiveBlurPath, p, true, 0, false);
//        _tempActiveBlurPath = QPainterPath();
//    }

//    if (_tempActiveBlurInfo.isValid())
//        addBlur(_tempActiveBlurInfo);

//    _tempActiveBlurInfo.clear();

//    update();
}

bool CGraphicsItem::isBlurActived()
{
    return !_tempActiveBlurPath.isEmpty();
}

void CGraphicsItem::setDrawRotatin(qreal angle)
{
    _roteAgnel = angle;
    // 进行角度取余数
    int n = int(_roteAgnel) / 360;
    _roteAgnel = _roteAgnel - n * 360;
    if (_roteAgnel < 0) {
        _roteAgnel += 360;
    }
}

void CGraphicsItem::updateShapeRecursion()
{
    updateShape();
    if (bzGroup(false) != nullptr) {
        bzGroup(false)->updateShapeRecursion();
    }
}

void CGraphicsItem::updateBlurPixmap(bool baseOrg, EBlurEffect effetTp)
{
    QPixmap pix = getCachePixmap(baseOrg);
    EBlurEffect blurEfTp = effetTp != UnknowEffect ? effetTp : EBlurEffect(static_cast<int>(drawScene()->pageContext()->defaultAttri(DrawAttribution::EBlurAttri).value<SBLurEffect>().type));
    _blurPix[blurEfTp] = NSBlur::blurPixmap(pix, 10, blurEfTp);
    update();
}

void CGraphicsItem::updateBlurPixmapBySelfBlurInfo()
{
    QSet<EBlurEffect> shuldUpadateEftSet;
    for (int i = 0; i < _blurInfos.size(); ++i) {
        shuldUpadateEftSet.insert(_blurInfos.at(i).blurEfTp);
    }
    for (auto it : shuldUpadateEftSet) {
        updateBlurPixmap(true, it);
    }
}

//void CGraphicsItem::addBlur(const SBlurInfo &sblurInfo)
//{
//    if (_blurInfos.isEmpty()) {
//        _blurInfos.append(sblurInfo);
//    } else {
//        SBlurInfo &last = _blurInfos.last();
//        if (last.blurEfTp == sblurInfo.blurEfTp) {
//            last.blurPath.addPath(sblurInfo.blurPath);
//        } else {
//            _blurInfos.append(sblurInfo);
//        }
//    }

//    update();
//}

void CGraphicsItem::paintAllBlur(QPainter *painter, const QPointF &translate)
{
    foreach (auto info, _blurInfos) {
        paintBlur(painter, info, translate);
    }
    paintBlur(painter, _tempActiveBlurInfo, translate);
}

void CGraphicsItem::paintBlur(QPainter *painter, const SBlurInfo &info, const QPointF &translate)
{
    if (!info.isValid())
        return;

    painter->save();
    painter->setClipRect(rect().translated(translate), Qt::IntersectClip);
    painter->setClipPath(info.blurPath.translated(translate), Qt::IntersectClip);
    if (_blurPix[info.blurEfTp].isNull()) {
        updateBlurPixmap(true, info.blurEfTp);
    }
    painter->drawPixmap(rect().topLeft() + translate, _blurPix[info.blurEfTp]);
    painter->restore();
}
void CGraphicsItem::initHandle()
{
    clearHandle();
    updateHandlesGeometry();
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setAcceptHoverEvents(true);
}

QVector<CSizeHandleRect *> CGraphicsItem::handleNodes()
{
    return m_handles;
}

CSizeHandleRect *CGraphicsItem::handleNode(CSizeHandleRect::EDirection direction)
{
    for (CSizeHandleRect *pNode : handleNodes()) {
        if (pNode->dir() == direction) {
            return pNode;
        }
    }
    return nullptr;
}

void CGraphicsItem::resetCachePixmap()
{
    if (_useCachePixmap && _cachePixmap != nullptr && m_operatingType == -1) {
        *_cachePixmap = getCachePixmap();
    }
}

QTransform CGraphicsItem::getFilpTransform() const
{
    QPointF center = boundingRect().center();
    QTransform trans(this->isFilped(CGraphicsItem::EFilpHor) ? -1 : 1, 0, 0,
                     0, this->isFilped(CGraphicsItem::EFilpVer)  ? -1 : 1, 0,
                     0, 0, 1);

    return (QTransform::fromTranslate(-center.x(), -center.y()) * trans * QTransform::fromTranslate(center.x(), center.y()));
}

QPixmap CGraphicsItem::rasterSelf()
{
    QPixmap pix = getCachePixmap();
    return pix;
}

void CGraphicsItem::rasterToSelfLayer(bool deleteSelf)
{
    QPixmap pix = this->rasterSelf();
    QTransform itemToLayer = this->itemTransform(this->layer());
    QPainter painter(&this->layer()->layerImage());

    //先保证painter的坐标系是layer，而不是layer的image
    painter.translate(-this->layer()->boundingRect().topLeft());

    //设置了从item到layer的矩阵变化，那么接下来的绘制可以假装是在item上绘制
    painter.setTransform(itemToLayer, true);

    painter.drawPixmap(this->boundingRectTruly().toRect(), pix);

    if (deleteSelf) {
        auto scene = drawScene();
        auto pItem = this;
        QMetaObject::invokeMethod(scene, [ = ]() {
            scene->removeCItem(pItem);
            //delete pItem;
        }, Qt::QueuedConnection);
    }
}

//QPointF CGraphicsItem::mapFromDrawScene(const QPointF &posInDScene) const
//{
////    //sceneTransform() is local item to scene,then sceneTransform().inverted() is scene to local item
////    auto trans = getFilpTransform() * sceneTransform().inverted()  /** getFilpTransform()*/;
////    auto result = trans.map(posInDScene);
////    return result;

//    return getFilpTransform().map(mapFromScene(posInDScene));
//}

//QPointF CGraphicsItem::mapToDrawScene(const QPointF &posInThis) const
//{
////    auto trans = sceneTransform() * getFilpTransform().inverted();
////    auto result = trans.map(posInThis);
////    return result;

//    return mapToScene(getFilpTransform().inverted().map(posInThis));
//}

void CGraphicsItem::updateViewport()
{
    if (curView() != nullptr) {
        curView()->viewport()->update();
    }
}

// Cppcheck检测函数没有使用到
//void CGraphicsItem::drawItem(QPainter *painter, const QStyleOptionGraphicsItem *option)
//{
//    paint(painter, option, nullptr);
//}


RegistItemHelper::RegistItemHelper(const QString &itemname, int type)
{
    CGraphicsItem::registerItem(itemname, type);
}
