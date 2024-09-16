// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CDRAWSCENE_H
#define CDRAWSCENE_H
#include "selecttool.h"
#include <QGraphicsScene>
#include <QPainterPath>
#include <QObject>

#include "globaldefine.h"
#include <pageobject.h>
#include "pageitem_p.h"
#include "itemgroup.h"
#include "rasteritem.h"

class QGraphicsSceneMouseEvent;
class QKeyEvent;
class RectBaseItem;
class PolygonItem;
class StarItem;
class PenItem_Compatible;
class LineItem;
class PageContext;
class PageView;
class Page;
class SelectionItem;
class UndoStack;

class DRAWLIB_EXPORT PageScene : public QGraphicsScene, public PageObject
{
    Q_OBJECT
public:
    /**
     * @brief DrawScene 构造函数
     * @param parent
     */
    explicit PageScene(PageContext *pageCxt);
    ~PageScene() override;

    PageContext *pageContext()const;
    Page *page() const;

    SAttrisList currentAttris() const;
    void  setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack);

    void addPageItem(PageItem *pItem);
    void addPageItem(PageItem *pItem, LayerItem *toLayer);
    void removePageItem(PageItem *pItem);

    void insertTopLayer(LayerItem *pLayer, int index = -1);
    void removeTopLayer(LayerItem *pLayer);
    void setCurrentTopLayer(LayerItem *pLayer);
    LayerItem *currentTopLayer()const;
    int topLayerCount()const;
    QList<LayerItem *> topLayers()const;
    void clearAll();

    //about select
    void selectPageItem(PageItem *item);
    void selectPageItem(const QList<PageItem *> &items);
    void notSelectPageItem(PageItem *item);
    void notSelectPageItem(const QList<PageItem *> &items);
    void selectAll();
    void clearSelections();
    void setPageItemSelected(PageItem *item, bool b);
    void selectItemsByRect(const QRectF &rect, bool replace = true);
    void blockSelect(bool b);
    QList<PageItem *> selectedPageItems() const;
    int selectedItemCount()const;
    QRectF selectionBoundingRect()const;
    void setSelectionsVisible(bool b);
    bool isSelectionsVisible()const;

    QColor bgColor() const;
    void setBgColor(const QColor &c);

    PageView *firstPageView() const;

    void setCursor(const QCursor &cursor);

    void blockAssignZValue(bool b);
    bool isAssignZBlocked()const;

    void blockScene(bool b);
    bool isSceneBlocked() const;

    static bool isPageItem(QGraphicsItem *pItem);
    static bool isHandleNode(QGraphicsItem *pItem);
    static bool isGroupItem(QGraphicsItem *pItem);
    static PageItem *associatedTopItem(QGraphicsItem *pItem);

    /**
     * @brief pageItems　获取一组图元中的基本业务图元(默认是按照z值降序)
     */
    QList<PageItem *> allPageItems(ESortItemTp tp = EDesSort)const;
    QList<PageItem *> allPageItems(const QPointF &pos, ESortItemTp tp = EDesSort)const;
    QList<PageItem *> allPageItems(const QRectF &rect, ESortItemTp tp = EDesSort)const;
    static QList<PageItem *> filterOutPageItems(const QList<QGraphicsItem *> &items);

    /**
     * @brief getRootItems　获取场景中所有的根图元(包括顶根组合图元),即bzGroup(true) == nullptr的图元
     * @param tp 表示排序类型
     */
    QList<PageItem *> allRootPageItems(ESortItemTp tp = EDesSort) const;
    QList<PageItem *> allRootPageItems(const QPointF &pos, ESortItemTp tp = EDesSort) const;
    QList<PageItem *> allRootPageItems(const QRectF &rect, ESortItemTp tp = EDesSort) const;
    static QList<PageItem *> filterOutRootPageItems(const QList<PageItem *> &items);

    /**
     * @brief moveBzItemsLayer 设置图元绘制的顺序
     * @param items表示要移动的基本业务图元
     * @param tp表示z要移动的方向
     * @param step要跳动的步数(-1表示到极限),tp为EDownLayer或EUpLayer时有效
     * @param pBaseInGroup 仅当tp为EToGroup才有效,以某一个图元z值为基准进行设置
     */
    static void movePageItemsZValue(const QList<PageItem *> &items,
                                    MoveItemZType tp, int step = 1, bool pushToUndoStack = false);

    /**
     * @brief isCurrentZMovable 判断当前是否可改变图层顺序
     */
    static bool isItemsZMovable(const QList<PageItem *> &items, MoveItemZType tp, int step = 1);

    /**
     * @brief topBzItem　获取某一点下的最顶层画板图元
     * @param pos 位置
     * @param penalgor 是否是考虑使用穿透算法过滤
     * @param IncW     以pos为中心作Incw半径矩形,将这个矩形范围内的图元也考虑进来
     */
    PageItem *topPageItem(const QPointF &pos, bool penalgor = true, int IncW = 0);

    /**
     * @brief topItem　获取一组图元中的第一个(最顶层)图元
     * @param pos 位置
     * @param items 某一组图元
     * @param isListDesSorted 该组图元items是否是按照z值降序排列好了
     * @param penalgor 是否是考虑使用穿透算法过滤
     * @param mustPageItem 返回的图元是否必须是基本业务图元
     * @param seeHandleNodeAsParentPageItem 是否items中的基本业务图元的节点图元会被当做是这个基本业务图元来看待
     * @param filterLayer 过滤掉图层图元
     * @param incW     以pos为中心作incW半径矩形,将这个矩形范围内的图元也考虑进来
     */
    QGraphicsItem *topItem(const QPointF &pos,
                           const QList<QGraphicsItem *> &items = QList<QGraphicsItem *>(),
                           bool isListDesSorted = true,
                           bool penalgor = false,
                           bool mustPageItem = false,
                           bool seeHandleNodeAsParentPageItem = false,
                           bool filterLayer = true,
                           int incW = 0);

    /**
     * @brief getMaxZValue　获取图元最大z值
     */
    qreal getMaxZValue();

    /**
     * @brief isGroupable 是否可以创建一个组合(默认是判断当前场景选中情况下是否可以进行组合)
     */
    bool isGroupable(const QList<PageItem *> &pageItems) const;

    /**
     * @brief isUnGroupable 框选的内容是否有组合图元可以进行撤销组合
     */
    bool isUnGroupable(const QList<PageItem *> &pBzItems) const;

    /**
     * @brief getManageGroup 获取到传入的图元的共同的顶层组合(如果不存在,那么返回空)
     * @param pBzItems 待获取的业务图元们
     * @param top为false有意义,如果图元们的组合都是nullptr,那么当sameNullCreatVirGroup为true时,会创建一个虚拟的组合进行统一管理,否则依然返回nullptr,用完记得删除!!!
     * @return 返回共同的组合图元,如果有不同的组合图元返回nullptr
     */
    GroupItem *getCommonGroup(const QList<PageItem *> &pBzItems, bool top = false);

    static bool isPageItemsSibling(const QList<PageItem *> &PageItems);

    static PageItem *samePageItemParent(const QList<PageItem *> &PageItems);

    /**
     * @brief creatGroup 基于多个图元创建一个组合(会将新创建的组合加入到这个场景中)
     * @param pBzItems 场景内的图元们(这个图元会被加入到新创建的组合中去)
     * @param pBzItemZBase 组合创建时,需要指定一个基础z值,以确定组合内基本图元的z值,为nullptr时取组合中z值最小的基本图元
     * @return 返回新创建的组合图元,失败返回nullptr(比如pBzItems的个数小于2,或者其中图元已经处于一个组合下了)
     */
    GroupItem *creatGroup(const QList<PageItem *> &pageItems, PageItem *base = nullptr);

    /**
     * @brief creatGroup 取消当前选中的组合
     */
    void cancelGroup(GroupItem *pGroup);

    QColor systemThemeColor()const;

    QImage renderToImage(const QColor &bgColor = Qt::transparent, const QSize &desImageSize = QSize());

    void loadData(const PageVariant &ut) final;
    PageVariant getData(int use)const final;
    void rotateSelectItems(qreal angle);
    QRectF currentTopLayerSceneRect();
signals:
    void selectionChanged(const QList<PageItem * > &children);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    bool event(QEvent *event) override;
    void drawItems(QPainter *painter, int numItems,
                   QGraphicsItem *items[],
                   const QStyleOptionGraphicsItem options[],
                   QWidget *widget = nullptr) override;

public:
    UnitTree_Comp getGroupTreeInfo() {return UnitTree_Comp();}

private:

    PRIVATECLASS(PageScene)
};
Q_DECLARE_METATYPE(PageScene *)

#endif // CDRAWSCENE_H
