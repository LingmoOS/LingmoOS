// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGEITEM_P_H
#define PAGEITEM_P_H
#include "pageitem.h"

class ExtendAttribution;
class SelectionItem;
class PageItem::PageItem_private
{
public:
    PageItem_private(PageItem *qp);

    static QMap<int, QString> &registedClassNameMap();
    static QList<int> registedItemTypes() ;


    void removeChildForChildList(PageItem *child);
    void addChildToChildList(PageItem *child);

    void operatingBegin_helper(PageItemEvent *event);
    void operating_helper(PageItemEvent *event);
    bool testOpetating_helper(PageItemEvent *event);
    void operatingEnd_helper(PageItemEvent *event);

    void setLayer_helper(LayerItem *layer);
    void notifyItemAndChildrenLayerChanged(LayerItem *layer);

    void setPageScene_helper(PageScene *scene);
    void notifyItemAndChildrenPageSceneChanged(PageScene *scene);

    void setSelected_helper(bool b);

    QVariant notifyItemChange(int doChange, const QVariant &value);

    void initStyleOption(QStyleOptionGraphicsItem *option, const QTransform &worldTransform,
                         const QRegion &exposedRegion, bool allItems) const;


    int  maxZ()const;
    //void setZ_Helper(int zz);

    PageItem *q;

    Handles m_handles;

    int m_operatingType = -1;

    bool isSelected    = false;
    int  isMulSelected = false;
    bool isSingleSelectionBorderVisible = true;

    GroupItem *_pGroup = nullptr;

    /* 关于镜像翻转 */
    bool _flipHorizontal = false;      // 水平翻转
    bool _flipVertical   = false;      // 垂直翻转
    bool blockNotify = false;

    qreal  _roteAgnel = 0;             //图元的旋转角度

    LayerItem *_layer = nullptr;

    QList<PageItem *> childrens;

    PageItem *parentItem = nullptr;

    //我的事件变化时要先转交给的过滤器items(过滤执行者)
    QList<PageItem *> filters;

    //将事件变化委托给我的items(过滤委托者)
    QSet<PageItem *>   filteDelegators;

    int z = 0;
    bool blockZAgin = false;

    //int  childMaxZ = 0;
    bool childrenZIsDirty = false;

    static bool paintSelectedBorderLine;
};

/**
 * @brief zItem 返回z值最小或最低或等于某一z值的item
 * @param pBzItems 被查找的items
 * @param wantZitemTp 想要的类型(-1返回z值最小的item,-2返回z值最大的item,z>=0返回等于该值的item)
 * @return 返回的图元
 */
DRAWLIB_EXPORT PageItem *zEndingItem(const QList<PageItem *> &pBzItems, int wantZitemTp = -1);


extern bool zValueSortDES(QGraphicsItem *info1, QGraphicsItem *info2);
extern bool zValueSortASC(QGraphicsItem *info1, QGraphicsItem *info2);


/**
 * @brief sortZ　通过z值对一组图元进行排序
 * @param list 某一组图元(即是入参也是出参)
 * @param tp 排序方式(升序还是降序)
 */

template<typename T>
static void sortZ(QList<T *> &list, ESortItemTp tp = EDesSort)
{
    auto f = (tp == EAesSort ? zValueSortASC : zValueSortDES);
    // 调用了过时的函数“ qSort”,改用'std :: sort'。
    std::sort(list.begin(), list.end(), f);
}

/**
 * @brief returnSortZItems　按照某一升降顺序进行排序
 */
template<typename T>
static QList<T *> returnSortZItems(const QList<T *> &list, ESortItemTp tp = EDesSort)
{
    QList<T *> sorts = list;
    sortZ(sorts, tp);
    return sorts;
}
template<typename I, class O>
static QList<O *> switchListClassTo(const QList<I *> &list)
{
    QList<O *> result;
    foreach (auto f, list) {
        auto it = dynamic_cast<O *>(f);
        if (it != nullptr)
            result.append(it);
    }
    return result;
}

template<typename T = QGraphicsItem *, typename K = PageItem *>
static QList<T> returnList(const QList<K> &list)
{
    QList<T> result;
    for (auto p : list) {
        result.append(p);
    }
    return result;
}

class PageItemNotifyBlocker
{
public:
    PageItemNotifyBlocker(PageItem *item, bool notifyChangs = false);
    ~PageItemNotifyBlocker();

private:
    bool valid = true;

    PageItem *it;
    bool blocked;

    PageItem  *parent;
    GroupItem *group;
    LayerItem *layer;
    PageScene *scene;

    bool doNotifyChanges = false;

    static QSet<PageItem *> inBlockerItems;
};

#endif // PAGEITEM_P_H
