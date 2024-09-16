// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSITEM_H
#define CGRAPHICSITEM_H

#include "../../utils/globaldefine.h"
#include "../../service/dyncreatobject.h"
#include "csizehandlerect.h"
#include "pageobject.h"
#include "pagecontext.h"

#include <QAbstractGraphicsShapeItem>
#include <QPainterPath>
#include <QCursor>
#include <QStyleOptionGraphicsItem>

class PageView;
class PageScene;

class GroupItem;
class LayerItem;

class PageItemEvent;
class PageItemMoveEvent;
class PageItemScalEvent;
class PageItemRotEvent;

class DRAWLIB_EXPORT RegistItemHelper
{
public:
    RegistItemHelper(const QString &itemname, int type);
};

#define REGISTITEM_DECLARE(classname,type)\
    REGISTCLASS_DECLARE(classname)\
    class classname##RegistItemHelper1{static RegistItemHelper helper;};

#define REGISTITEM_DEFINE(classname,type)\
    REGISTCLASS_DEFINE(classname)\
    RegistItemHelper classname##RegistItemHelper1::helper = RegistItemHelper(#classname,type);

#define REGISTITEMCLASS(classname,type)\
    REGISTITEM_DECLARE(classname,type) \
    REGISTITEM_DEFINE(classname,type)


class DRAWLIB_EXPORT PageItem : public QGraphicsItem, public PageObject
{
    //Q_OBJECT
public:
    enum EFilpDirect {EFilpHor, EFilpVer};
    enum EPageItemChange {
        PageItemSceneChanged = ItemSceneHasChanged,
        PageItemParentChanged = /*ItemTransformOriginPointHasChanged + 100*/ItemParentHasChanged,
        PageItemGroupChanged = PageItemParentChanged,
        PageItemChildAdded = ItemChildAddedChange,
        PageItemChildRemoved = ItemChildRemovedChange,
        PageItemZChanged = ItemZValueHasChanged,

        PageItemRectChanged = ItemTransformOriginPointHasChanged + 100,
        PageItemSelectionChanged,
        PageItemMulSelectionChanged,
        PageItemLayerChanged
    };

    explicit PageItem(PageItem *parent = nullptr);

    ~PageItem();

    static void registerItem(const QString &classname, int classType);
    static PageItem *creatItemInstance(int itemType, const Unit &data = Unit());
    static QList<int> registedItemTypes();

    bool isLayer()const;
    LayerItem *layer() const;
    void setLayer(LayerItem *layer);
    LayerItem *topLayer()const;
    bool isTopLayer()const;

    void setPageScene(PageScene *scene);
    void setParentPageItem(PageItem *parent);
    PageItem *parentPageItem() const;
    PageItem *rootParentPageItem()const;
    bool isRootPageItem()const;
    void removeChild(PageItem *child);
    void addChild(PageItem *child);
    QList<PageItem *> childPageItems() const;

    //base pageGroup == parentPageItem
    bool isPageGroup() const;
    GroupItem *pageGroup() const;
    void setPageGroup(GroupItem *pGroup);
    bool isTopPageGroup()const;
    GroupItem *topPageGroup() const;
    PageItem *pageProxyItem(bool topleve = true);

    virtual SAttrisList attributions();
    virtual void  setAttributionVar(int attri, const QVariant &var, int phase);

    enum {Type = UserType};
    int  type() const override;

    PageView *pageView()const;
    PageScene *pageScene() const;
    Page *page()const;
    bool isIdle() const;
    void deleteSafe();

    virtual QRectF       itemRect() const = 0;
    virtual QPainterPath itemShape() const;

    inline virtual QRectF       orgRect() const {return itemRect();}
    inline virtual QPainterPath orgShape() const {return itemShape();}

    virtual QRectF selectedRectangle() const;
    virtual QPainterPath highLightPath() const;

    bool contains(const QPointF &point) const override;
    virtual bool isPosPenetrable(const QPointF &posLocal);
    virtual bool isRectPenetrable(const QRectF &rectLocal);

    qreal pageZValue() const;
    void  setPageZValue(qreal z);

    void loadItemUnit(const Unit &ut) ;
    Unit getItemUnit(int use)const ;

    void loadData(const PageVariant &ut) final;
    PageVariant getData(int use)const final;

    bool isBzItem() const;

    virtual void doFilp(EFilpDirect dir = EFilpHor);
    void setFilpBaseOrg(EFilpDirect dir, bool b);
    bool isFilped(EFilpDirect dir) const;

    void rot90Angle(bool leftOrRight);

    void move(QPointF beginPoint, QPointF movePoint);

    virtual void updateShape();

    int  operatingType()const;
    bool isOperating() const;

    bool isItemSelected() const;
    void setItemSelected(bool b);

    bool isSingleSelected() const;
    bool isMutiSelected() const;

    void addHandleNode(HandleNode *node);
    void removeHandleNode(HandleNode *node);
    int  nodeCount() const;
    Handles handleNodes() const;
    HandleNode *handleNode(int direction = HandleNode::Rotation)const;

    void setSingleSelectionBorderVisible(bool b);
    bool isSingleSelectionBorderVisible() const;

    QTransform getFilpTransform() const;

    QPointF mapFromPageScene(const QPointF &posInDScene) const;
    QPointF mapToPageScene(const QPointF &posInThis) const;

    static void setPaintSelectingBorder(bool b);
    static bool isPaintSelectingBorder();

    void updateViewport();

    virtual void operatingBegin(PageItemEvent *event);
    virtual void operating(PageItemEvent *event);
    virtual bool testOpetating(PageItemEvent *event);
    virtual void operatingEnd(PageItemEvent *event);

    bool installChangeFilter(PageItem *filter);
    void removeChangeFilter(PageItem *filter);

    void render(QPainter *painter);

    QImage renderToImage();

    //static ExtendAttribution *getExtendObject();
protected:
    QRectF boundingRect() const final;
    QPainterPath shape() const final;

    void preparePageItemGeometryChange();

protected:
    virtual void doMoveBegin(PageItemMoveEvent *event);
    virtual void doMoving(PageItemMoveEvent *event);
    virtual bool testMoving(PageItemMoveEvent *event);
    virtual void doMoveEnd(PageItemMoveEvent *event);

    virtual void doScalBegin(PageItemScalEvent *event);
    virtual void doScaling(PageItemScalEvent *event);
    virtual bool testScaling(PageItemScalEvent *event);
    virtual void doScalEnd(PageItemScalEvent *event);

    virtual void doRotBegin(PageItemRotEvent *event);
    virtual void doRoting(PageItemRotEvent *event);
    virtual bool testRoting(PageItemRotEvent *event);
    virtual void doRotEnd(PageItemRotEvent *event);

    virtual void loadUnit(const Unit &ut);
    virtual Unit getUnit(int use) const;
public:
    void loadHeadData(const UnitHead &head);
    UnitHead getHeadData()const;

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    void notifyChanged(int changeType, const QVariant &value);
    QVariant itemChange(GraphicsItemChange doChange, const QVariant &value) final;
    virtual QVariant pageItemChange(int changeType, const QVariant &value);
    virtual bool pageItemChangeFilter(PageItem *item, int changeType, QVariant &value);

    static void paintItemHelper(PageItem *item, QPainter *painter, const QStyleOptionGraphicsItem *option,
                                QWidget *widget);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    void paintChidren(QPainter *painter, const QList<PageItem *> &childList, QWidget *widget = nullptr);

    void beginCheckIns(QPainter *painter);
    void endCheckIns(QPainter *painter);
    void paintMutBoundingLine(QPainter *painter, const QStyleOptionGraphicsItem *option);

    virtual void paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option);

    void mousePressEvent(QGraphicsSceneMouseEvent *event) final;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) final;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) final;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) final;

protected:
    void  changeTransCenterTo(const QPointF &newCenter);
    void  updateHandle();

public:
    void  setDrawRotatin(qreal angle);
    qreal drawRotation()const;

protected:
    PRIVATECLASS(PageItem)
    friend class PageScene;
};
Q_DECLARE_METATYPE(PageItem *)

#endif // CGRAPHICSITEM_H
