// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cdrawscene.h"
#include "cgraphicsrectitem.h"
#include "idrawtool.h"
#include "cdrawtoolmanagersigleton.h"
#include "cdrawparamsigleton.h"
#include "globaldefine.h"
#include "cgraphicspolygonitem.h"
#include "cgraphicspolygonalstaritem.h"
#include "cgraphicspenitem.h"
//#include "frame/cpicturewidget.h"
#include "cgraphicstextitem.h"
#include "ccuttool.h"
#include "cgraphicstextitem.h"
#include "cgraphicsproxywidget.h"
#include "cgraphicslineitem.h"
#include "cpictureitem.h"
#include "cgraphicsitemselectedmgr.h"
#include "cpictureitem.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "widgets/ctextedit.h"
#include "application.h"
#include "cundoredocommand.h"
#include "clefttoolbar.h"
#include "cgraphicsellipseitem.h"
#include "cgraphicstriangleitem.h"
#include "toptoolbar.h"
#include "cattributemanagerwgt.h"
#include "ccentralwidget.h"

#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QRect>
#include <QGraphicsView>
#include <QPainterPath>
#include <QtMath>
#include <DApplication>
#include <QScrollBar>
#include <QWindow>

DWIDGET_USE_NAMESPACE

PageScene::PageScene(PageContext *pageCxt)
    : QGraphicsScene(pageCxt)
    , m_pSelGroupItem(nullptr)
{
    resetScene();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this,
    [ = ](DGuiApplicationHelper::ColorType themeType) {
        resetSceneBackgroundBrush();
    });
}

PageScene::~PageScene()
{
    foreach (auto p, m_notInSceneItems) {
        delete p;
    }
    m_notInSceneItems.clear();
}

PageContext *PageScene::pageContext() const
{
    return qobject_cast<PageContext *>(parent());
}

Page *PageScene::page() const
{
    if (this->drawView() == nullptr)
        return nullptr;

    return this->drawView()->page();
}

SAttrisList PageScene::currentAttris() const
{
    DrawAttribution::SAttrisList attris = selectGroup()->attributions();
    if (selectGroup()->allCount() > 1/*!= 0*/) {
        if (!attris.isEmpty()) {
            attris << DrawAttribution::SAttri(ESeparator);
        }

        QList<QVariant> couple; couple << isGroupable() << isUnGroupable();
        attris << DrawAttribution::SAttri(DrawAttribution::EGroupWgt, couple);
    }
    return attris;
}

void PageScene::insertLayer(CGraphicsLayer *pLayer, int index)
{
    if (pLayer == nullptr)
        return;

    if (m_layers.contains(pLayer)) {
        return;
    }
    m_layers.insert(index, pLayer);
    this->addItem(pLayer);
}

//void PageScene::removeLayer(CGraphicsLayer *pLayer)
//{
//    if (m_layers.contains(pLayer)) {
//        m_layers.removeOne(pLayer);
//        this->removeItem(pLayer);
//        if (m_currentLayer == pLayer) {
//            m_currentLayer = m_layers.isEmpty() ? nullptr : m_layers.last();
//        }
//    }
//}

void PageScene::setCurrentLayer(CGraphicsLayer *pLayer)
{
    m_currentLayer = pLayer;
}

//QList<CGraphicsLayer *> PageScene::graphicsLayers()
//{
//    return m_layers;
//}

void PageScene::resetScene()
{
    clear();
    resetSceneBackgroundBrush();
    m_pSelGroupItem = nullptr;
    if (m_pSelGroupItem == nullptr) {
        m_pSelGroupItem = new CGraphicsItemGroup(CGraphicsItemGroup::ESelectGroup);

        this->addItem(m_pSelGroupItem);
        m_pSelGroupItem->setZValue(INT_MAX);

        connect(m_pSelGroupItem, &CGraphicsItemGroup::childrenChanged, this, [ = ](const QList<CGraphicsItem * > &children) {
            Q_UNUSED(children)
            CGraphicsItemGroup *pSeleGp = m_pSelGroupItem;
            if (pSeleGp == nullptr || pSeleGp->drawScene() == nullptr)
                return;
            if (pSeleGp->page() == nullptr)
                return;

            if (pSeleGp->page()->isActivedPage())
                updateAttribution();

            emit selectionChanged(children);
        });
    }

    CGraphicsLayer *pLayer = new CGraphicsLayer;
    this->insertLayer(pLayer);
    setCurrentLayer(pLayer);
    connect(this, &QGraphicsScene::sceneRectChanged, this, [ = ](const QRectF & rect) {
        m_currentLayer->setRect(rect);
    });
}

void PageScene::setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack)
{
    //qWarning() << "autoCmdStack ==== " << autoCmdStack << "phase  = " << phase ;
    CCmdBlock block((autoCmdStack  ? selectGroup() : nullptr), EChangedPhase(phase));
    selectGroup()->setAttributionVar(attri, var, phase);
}

void PageScene::updateAttribution()
{
    attributeDirty = true;
    update();
}

QColor PageScene::bgColor() const
{
    return _bgColor;
}

void PageScene::setBgColor(const QColor &c)
{
    _bgColor = c;
    update();
}

PageView *PageScene::drawView() const
{
    return (views().isEmpty() ? nullptr : qobject_cast<PageView *>(views().first()));
}

void PageScene::mouseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    switch (mouseEvent->type()) {
    case QEvent::GraphicsSceneMousePress:
        qDebug() << "qt to do SceneMousePress-----";
        QGraphicsScene::mousePressEvent(mouseEvent);
        break;
    case QEvent::GraphicsSceneMouseMove:
        //qDebug() << "qt to do SceneMouseMove----- press = " << mouseEvent->buttons();
        QGraphicsScene::mouseMoveEvent(mouseEvent);
        break;
    case QEvent::GraphicsSceneMouseRelease:
        qDebug() << "qt to do SceneMouseRelease-----";
        QGraphicsScene::mouseReleaseEvent(mouseEvent);
        break;
    case QEvent::GraphicsSceneMouseDoubleClick:
        qDebug() << "qt to do SceneMouseDoubleClick-----";
        QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
        break;
    default:
        break;
    }
}

void PageScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    ensureAttribution();

    //resetSceneBackgroundBrush();

    QGraphicsScene::drawBackground(painter, rect);

    painter->fillRect(sceneRect(), bgColor());
}

void PageScene::resetSceneBackgroundBrush()
{
    int themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == 1) {
        this->setBackgroundBrush(QColor(248, 248, 251));
    } else if (themeType == 2) {
        this->setBackgroundBrush(QColor(35, 35, 35));
    }
    this->setBgColor(Qt::white);
}

void PageScene::setCursor(const QCursor &cursor)
{
    if (page() != nullptr) {
        page()->setDrawCursor(cursor);
    }
}

void PageScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    IDrawTool *pTool =  page()->currentTool_p();
    if (nullptr != pTool) {
        pTool->mousePressEvent(mouseEvent, this);
    }
}

void PageScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (isBlockMouseMoveEvent())
        return;

    IDrawTool *pTool =  page()->currentTool_p();
    if (nullptr != pTool) {
        pTool->mouseMoveEvent(mouseEvent, this);
    }
}

void PageScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    IDrawTool *pTool =  page()->currentTool_p();
    if (nullptr != pTool) {
        if (pTool->isWorking() && mouseEvent->button() != Qt::LeftButton) {
            return;
        }
        pTool->mouseReleaseEvent(mouseEvent, this);
    }
}

void PageScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    IDrawTool *pTool =  page()->currentTool_p();
    if (nullptr != pTool) {
        pTool->mouseDoubleClickEvent(mouseEvent, this);
    }
}

void PageScene::doLeave()
{
    IDrawTool *pTool =  page()->currentTool_p();

    if (pTool != nullptr) {
        if (pTool->isWorking()) {
            QMetaObject::invokeMethod(this, [ = ]() {
                pTool->interrupt();
            }, Qt::QueuedConnection);
        }
    }
}

bool PageScene::event(QEvent *event)
{
    QEvent::Type evType = event->type();
    if (evType == QEvent::TouchBegin || evType == QEvent::TouchUpdate || evType == QEvent::TouchEnd) {

        QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event);

        QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();

        IDrawTool *pTool =  page()->currentTool_p();

        if (nullptr == pTool || (touchPoints.count() > 1 && pTool->getDrawToolMode() == selection)) {
            //是一个手势，那么中断当前的updating操作
            if (pTool != nullptr)
                pTool->interrupt();
            return QGraphicsScene::event(event);
        }

        //解决触屏后鼠标隐藏但还可能鼠标的位置还是高亮的问题
        if (evType == QEvent::TouchBegin) {
            QAbstractButton *pBtn = pTool->toolButton();
            if (pBtn != nullptr) {
                pBtn->setAttribute(Qt::WA_UnderMouse, false);
                pBtn->update();
            }
        }

        bool accept = true;
        foreach (const QTouchEvent::TouchPoint tp, touchPoints) {
            CDrawToolEvent e = CDrawToolEvent::fromTouchPoint(tp, this, touchEvent);
            switch (tp.state()) {
            case Qt::TouchPointPressed:
                //表示触碰按下
                pTool->toolDoStart(&e);
                break;
            case Qt::TouchPointMoved:
                //触碰移动
                pTool->toolDoUpdate(&e);
                break;
            case Qt::TouchPointReleased:
                //触碰离开
                pTool->toolDoFinish(&e);
                break;
            default:
                break;
            }
            if (!e.isAccepted()) {
                accept = false;
            }
        }
        event->accept();

        if (evType == QEvent::TouchEnd)
            pTool->clearITE();

        if (accept)
            return true;

    } else if (event->type() == QEvent::Gesture) {
        int currentMode = page()->currentTool();

        if (currentMode == selection) {
            return drawView()->gestureEvent(static_cast<QGestureEvent *>(event));
        }
    }
    return QGraphicsScene::event(event);
}

void PageScene::drawItems(QPainter *painter, int numItems, QGraphicsItem *items[], const QStyleOptionGraphicsItem options[], QWidget *widget)
{
    painter->setClipping(true);
    painter->setClipRect(sceneRect());

    QGraphicsScene::drawItems(painter, numItems, items, options, widget);
}

void PageScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    //禁止绘制时返回
    if (!bDrawForeground)
        return;

    //绘制额外的前景显示，如框选等

    IDrawTool *pTool = page()->currentTool_p();

    if (pTool != nullptr) {
        pTool->drawMore(painter, rect, this);
    }

    QGraphicsScene::drawForeground(painter, rect);
}

PageScene::CGroupBzItemsTree PageScene::getGroupTree(CGraphicsItemGroup *pGroup)
{
    CGroupBzItemsTree info;

    //为nullptr表示收集当前场景下的组合情况
    if (pGroup == nullptr) {
        for (auto pGroup : m_pGroups) {
            if (pGroup->isTopBzGroup()) {
                info.childGroups.append(getGroupTree(pGroup));
            }
        }
        info.setGroupType(CGraphicsItemGroup::EVirRootGroup);
        return info;
    }

    info.pGroup = pGroup;
    info.setGroupType(pGroup->groupType());
    CGraphicsUnit tempGroupData = pGroup->getGraphicsUnit(EDuplicate);
    CGraphicsUnit::deepCopy(info.data, tempGroupData);
    tempGroupData.release();

    QList<CGraphicsItem *> items = pGroup->items();
    for (auto it : items) {
        if (isNormalGroupItem(it)) {
            info.childGroups.append(getGroupTree(static_cast<CGraphicsItemGroup *>(it)));
        } else {
            info.bzItems.append(it);
        }
    }
    return info;
}

CGroupBzItemsTreeInfo PageScene::getGroupTreeInfo(CGraphicsItemGroup *pGroup, EDataReason reson,
                                                  bool processEvent)
{
    CGroupBzItemsTreeInfo info;

    //为nullptr表示收集当前场景下的组合情况
    if (pGroup == nullptr) {
        for (auto pGroup : m_pGroups) {
            if (processEvent)
                qApp->processEvents();
            if (pGroup->isTopBzGroup()) {
                info.childGroups.append(getGroupTreeInfo(pGroup));
            }
        }
        if (reson == ESaveToDDf) {
            auto bzItems = getBzItems();
            for (auto p : bzItems) {
                if (processEvent)
                    qApp->processEvents();
                if (p->bzGroup(true) == nullptr) {
                    info.bzItems.append(p->getGraphicsUnit(reson));
                }
            }
        }
        info.setGroupType(CGraphicsItemGroup::EVirRootGroup);
        info.data.head.dataType = MgrType;
        return info;
    }
//    if (pGroup == nullptr)
//        return info;

    CGraphicsUnit tempGroupData = pGroup->getGraphicsUnit(reson);
    CGraphicsUnit::deepCopy(info.data, tempGroupData);
    tempGroupData.release();

    //只允许有一个选择组合图元
    if (info.groupType() == CGraphicsItemGroup::ESelectGroup) {
        info.setGroupType(CGraphicsItemGroup::EVirRootGroup);
    }

    //按照升序进行保存,这样在被加载回来的时候z值可以从低到高正确
    auto groups  = pGroup->getGroups();
    auto fSortGroup = [ = ](CGraphicsItemGroup * gp1, CGraphicsItemGroup * gp2) {
        return gp1->getMinZ() <= gp2->getMinZ();
    };
    // 调用了过时的函数“ qSort”，改用'std :: sort'
    std::sort(groups.begin(), groups.end(), fSortGroup);

    auto bzItems = pGroup->getBzItems();
    auto fSortItems = [ = ](CGraphicsItem * it1, CGraphicsItem * it2) {
        return it1->zValue() <= it2->zValue();
    };
    std::sort(bzItems.begin(), bzItems.end(), fSortItems);

    foreach (auto gp, groups) {
        if (processEvent)
            qApp->processEvents();
        info.childGroups.append(getGroupTreeInfo(gp));
    }
    foreach (auto it, bzItems) {
        if (processEvent)
            qApp->processEvents();
        info.bzItems.append(it->getGraphicsUnit(reson));
    }
    return info;
}

CGraphicsItemGroup *PageScene::loadGroupTree(const PageScene::CGroupBzItemsTree &info)
{
    //如果是顶层根管理类型(顶层) 那么先清空当前场景内的组合情况
    if (info.groupType() == CGraphicsItemGroup::EVirRootGroup)
        destoryAllGroup();

    CGraphicsItemGroup *pGroup = (info.pGroup == nullptr ? nullptr : static_cast<CGraphicsItemGroup *>(info.pGroup));

    QList<CGraphicsItem *> items = info.bzItems;

    foreach (auto it, info.childGroups) {
        CGraphicsItemGroup *pChildGroup = loadGroupTree(it);
        if (pChildGroup != nullptr)
            items.append(pChildGroup);
    }

    if (!items.isEmpty()) {
        if (pGroup == nullptr)
            pGroup = creatGroup(items, info.groupType());
        else {
            foreach (auto p, items) {
                pGroup->add(p, false, false);
            }
            pGroup->setRecursiveScene(this);
            pGroup->updateBoundingRect();
            if (!m_pGroups.contains(pGroup))
                m_pGroups.append(pGroup);
            selectItem(pGroup);
        }
        pGroup->loadGraphicsUnit(info.data);
    }
    return pGroup;
}

CGraphicsItemGroup *PageScene::loadGroupTreeInfo(const CGroupBzItemsTreeInfo &info, bool notClear)
{
    //如果是非常规组合 那么先清空当前场景内的组合情况
    if (info.groupType() != CGraphicsItemGroup::ENormalGroup && !notClear)
        destoryAllGroup();

    CGraphicsItemGroup *pGroup = nullptr;

    QList<CGraphicsUnit> utItems = info.bzItems;
    QList<CGraphicsItem *> items;
    foreach (auto ut, utItems) {
        CGraphicsItem *pItem = CGraphicsItem::creatItemInstance(ut.head.dataType, ut);
        if (pItem != nullptr)
            items.append(pItem);
    }

    foreach (auto it, info.childGroups) {
        CGraphicsItemGroup *pChildGroup = loadGroupTreeInfo(it);
        if (pChildGroup != nullptr)
            items.append(pChildGroup);
    }

    if (!items.isEmpty()) {
        pGroup = creatGroup(items, info.groupType());
        pGroup->loadGraphicsUnit(info.data);
    }

    return pGroup;
}

void PageScene::releaseBzItemsTreeInfo(CGroupBzItemsTreeInfo info)
{
    for (int i = 0; i < info.childGroups.size(); ++i) {
        CGroupBzItemsTreeInfo &childInfo = info.childGroups[i];
        releaseBzItemsTreeInfo(childInfo);
    }
    for (int i = 0; i < info.bzItems.size(); ++i) {
        info.bzItems[i].release();
    }
}

void PageScene::releaseBzItemsTreeInfo(CGroupBzItemsTree info)
{
    for (int i = 0; i < info.childGroups.size(); ++i) {
        CGroupBzItemsTree &childInfo = info.childGroups[i];
        releaseBzItemsTreeInfo(childInfo);
    }
    for (int i = 0; i < info.bzItems.size(); ++i) {
        auto pBzItem = info.bzItems[i];

        if (pBzItem->scene() == nullptr) {
            delete pBzItem;
        }
    }

    if (info.pGroup != nullptr) {
        auto p = static_cast<CGraphicsItemGroup *>(info.pGroup);
        if (p->scene() == nullptr) {
            p->deleteLater();
        }
        info.pGroup = nullptr;
    }
}

void assignmentZ(const QList<CGraphicsItem *> &items, qreal &beginZ, PageScene::ESortItemTp listSortedTp)
{
    int beginIndex = listSortedTp == PageScene::EDesSort ? items.size() - 1 : 0;
    int endIndex   = listSortedTp == PageScene::EDesSort ? 0 : items.size();
    int inc = listSortedTp == PageScene::EDesSort ? -1 : 1;
    for (int i = beginIndex; listSortedTp == PageScene::EDesSort ? i >= endIndex : i < endIndex; i += inc) {
        CGraphicsItem *p = items.at(i);
        if (p->isBzItem()) {
            p->setZValue(beginZ);
            auto dynamicLayer = dynamic_cast<JDynamicLayer *>(p);
            //动态图元移动了需要记录命令 用于保存和二次打开
            if (nullptr != dynamicLayer) {
                auto cmd = new JGeomeCommand(dynamicLayer);
                dynamicLayer->appendComand(cmd, false, false);
            }
            ++beginZ;
        } else if (p->isBzGroup()) {
            auto pGroup = static_cast<CGraphicsItemGroup *>(p);
            assignmentZ(PageScene::returnSortZItems(pGroup->items(), listSortedTp), beginZ, listSortedTp);
        }
    }
}

void PageScene::moveItemsZDown(const QList<CGraphicsItem *> &items, int step)
{
    //1.首先判断图元们是否是一个组合的,要移动的图元需要是处于一个组合中的(或者都不处于组合)
    CGraphicsItemGroup *pSameGroup = getSameGroup(items, false, false, true);
    if (pSameGroup == nullptr)
        return;

    //2.获取会受到影响的图元(按照z降序排列,即从z值大到小排序)
    qreal minZ = 0;
    QList<CGraphicsItem *> invokedItems;
    if (pSameGroup->groupType() == CGraphicsItemGroup::ENormalGroup) {
        invokedItems = pSameGroup->items();
        sortZ(invokedItems, EDesSort);
        minZ = invokedItems.last()->zValue();
    } else {
        invokedItems = getRootItems(EDesSort);
        minZ = 0;
    }
    //3.进行排序
    bool toLimitFirst = true;
    //按照z值从低到高进行遍历,遇到想移动的图元见按照步数在invokedItems链表向后移动
    for (int i = invokedItems.size() - 1; i >= 0; --i) {
        CGraphicsItem *p = invokedItems.at(i);
        if (items.contains(p)) {
            //向下移动step个图层
            int index = (i + step + 1);
            if (step == -1) {
                if (toLimitFirst) {
                    index = invokedItems.size();
                    toLimitFirst = false;
                } else {
                    index = invokedItems.size() - 1;
                }
            }
            //边界检查
            if (index > invokedItems.size()) {
                index = invokedItems.size();
            }
            //先插入再删除以实现将当前的图元移动到正确的位置
            invokedItems.insert(index, p);
            invokedItems.removeAt(i);
        }
    }

    //4.循环遍历给于新的z值
    assignmentZ(invokedItems, minZ, EDesSort);

    if (pSameGroup->groupType() == CGraphicsItemGroup::EVirRootGroup) {
        //delete pSameGroup;
        pSameGroup->clear();
        pSameGroup->deleteLater();
    }
}

void PageScene::moveItemsZUp(const QList<CGraphicsItem *> &items, int step)
{
    //1.首先判断图元们是否是一个组合的,要移动的图元需要是处于一个组合中的(或者都不处于组合)
    CGraphicsItemGroup *pSameGroup = getSameGroup(items, false, false, true);
    if (pSameGroup == nullptr) {
        return;
    }

    //2.获取会受到影响的图元(按照z升序排列,即从z值小到大排序)
    qreal minZ = 0;
    QList<CGraphicsItem *> invokedItems;
    if (pSameGroup->groupType() == CGraphicsItemGroup::ENormalGroup) {
        invokedItems = pSameGroup->items();
        sortZ(invokedItems, EAesSort);
        minZ = invokedItems.first()->zValue();
    } else {
        invokedItems = getRootItems(EAesSort);
        minZ = 0;
    }
    //3.进行排序
    bool toLimitFirst = true;
    //按照z值从高到低进行遍历,遇到想移动的图元见按照步数在invokedItems链表向后动
    for (int i = invokedItems.size() - 1; i >= 0; --i) {
        CGraphicsItem *p = invokedItems.at(i);
        if (items.contains(p)) {
            int index = (i + step + 1);
            if (step == -1) {
                if (toLimitFirst) {
                    index = invokedItems.size();
                    toLimitFirst = false;
                } else {
                    index = invokedItems.size() - 1;
                }
            }
            //边界检查
            if (index > invokedItems.size()) {
                index = invokedItems.size();
            }
            //先插入再删除以实现将当前的图元移动到正确的位置
            invokedItems.insert(index, p);
            invokedItems.removeAt(i);
        }
    }

    //4.循环遍历给于新的z值
    assignmentZ(invokedItems, minZ, EAesSort);

    if (pSameGroup->groupType() == CGraphicsItemGroup::EVirRootGroup) {
        //delete pSameGroup;
        pSameGroup->clear();
        pSameGroup->deleteLater();
    }
}

void PageScene::sortZBaseOneBzItem(const QList<CGraphicsItem *> &items, CGraphicsItem *pBaseItem)
{
    //0.必须保证基准图元时在被操作的图元中的
    assert(items.contains(pBaseItem));

    //场景下所有的基本业务图元
    auto scenBzItems = this->getBzItems();
    int  tempZ = scenBzItems.size() - 1;

    QList<QList<CGraphicsItem *>> tempListLists;

    QList<CGraphicsItem *> temp;
    int baszZIndex = 0;
    QList<int> groupListIndexList;

    bool nextListIsBase = false;

    bool preIemInGroup = false;
    bool frst = true;
    for (auto p : scenBzItems) {
        CGraphicsItem *pBz = static_cast<CGraphicsItem *>(p);
        bool isInGroup = items.contains(pBz);
        if (isInGroup == preIemInGroup || frst) {
        } else {
            if (nextListIsBase) {
                baszZIndex = tempListLists.size();
                nextListIsBase = false;
            }
            if (preIemInGroup) {
                groupListIndexList.append(tempListLists.size());
            }
            tempListLists.append(temp);
            temp.clear();
        }
        temp.append(pBz);
        if (isInGroup && pBz == pBaseItem) {
            nextListIsBase = true;
        }
        preIemInGroup = isInGroup;
        frst = false;
    }

    //再判断一次,进行边界收尾
    if (nextListIsBase) {
        baszZIndex = tempListLists.size();
    }
    if (preIemInGroup) {
        groupListIndexList.append(tempListLists.size());
    }
    tempListLists.append(temp);
    temp.clear();

    for (int i = 0; i < tempListLists.size(); ++i) {
        if (i == baszZIndex) {
            for (auto index : groupListIndexList) {
                auto list = tempListLists.at(index);
                for (int j = 0; j < list.size(); ++j) {
                    list.at(j)->setZValue(tempZ);
                    --tempZ;
                }
            }

        } else {
            if (!groupListIndexList.contains(i)) {
                auto list = tempListLists.at(i);
                for (int j = 0; j < list.size(); ++j) {
                    list.at(j)->setZValue(tempZ);
                    --tempZ;
                }
            }
        }
    }

    //弄脏z值,获取到组合的正确z值
    foreach (auto pG, m_pGroups) {
        pG->markZDirty();
    }
#ifdef QT_DEBUG
    //打印测试
    {
        qDebug() << "begin print z:";
        auto itemsggg = this->getBzItems();
        qDebug() << "scenBzItems count = " << itemsggg.count();
        foreach (auto p, itemsggg) {
            qDebug() << "-------z = " << p->zValue();
        }
        qDebug() << "end   print z:";
    }
#endif
}

void PageScene::ensureAttribution()
{
//    if (attributeDirty) {
//        auto currentTool = page()->currentTool_p()->getDrawToolMode();
//        if (currentTool == selection) {
//            if (page() != nullptr && page()->borad() != nullptr && page()->borad()->attributionWidget() != nullptr) {
//                DrawAttribution::SAttrisList attris = selectGroup()->attributions();
//                if (selectGroup()->count() != 0) {
//                    QList<QVariant> couple; couple << isGroupable() << isUnGroupable();
//                    attris << DrawAttribution::SAttri(DrawAttribution::EGroupWgt, couple);
//                } else {
//                    attris << DrawAttribution::SAttri(DrawAttribution::ETitle, page()->title());
//                }
//                page()->borad()->attributionWidget()->setAttributions(attris);
//            }
//        }
//        attributeDirty = false;
//    }



    if (attributeDirty) {
        pageContext()->update();
        attributeDirty = false;
    }
}

//void PageScene::setDrawForeground(bool b)
//{
//    bDrawForeground = b;
//}

//void PageScene::setItemsActive(bool canSelecte)
//{
//    //让其他图元不可选中
//    QList<QGraphicsItem *> itemList = this->items();
//    foreach (QGraphicsItem *item, itemList) {
//        if (item->type() > QGraphicsItem::UserType) {
//            item->setFlag(QGraphicsItem::ItemIsMovable, canSelecte);
//            item->setFlag(QGraphicsItem::ItemIsSelectable, canSelecte);
//        }
//    }
//}

//void PageScene::switchTheme(int type)
//{
//    Q_UNUSED(type);
////    QList<QGraphicsItem *> items = this->items();//this->collidingItems();
////    //QList<QGraphicsItem *> items = this->collidingItems();
////    for (int i = items.size() - 1; i >= 0; i--) {
////        CGraphicsItem *pItem = dynamic_cast<CGraphicsItem *>(items[i]);
////        if (pItem != nullptr) {
////            if (pItem->type() == BlurType) {
////                static_cast<CGraphicsMasicoItem *>(items[i])->updateMasicPixmap();
////            }
////        }
////    }
//}

CGraphicsItemGroup *PageScene::selectGroup() const
{
    return m_pSelGroupItem;
}

void PageScene::addCItem(QGraphicsItem *pItem, bool calZ, bool record)
{
    if (pItem == nullptr)
        return;

    CCmdBlock blocker((record ? this : nullptr), CSceneUndoRedoCommand::EItemAdded, QList<QGraphicsItem *>() << pItem);

    if (calZ && isBussizeItem(pItem) && !blockZAssign) {
        qreal curMax = getMaxZValue();
        qreal z = curMax + 1;
        pItem->setZValue(z);
    }

    this->addItem(pItem);
    m_notInSceneItems.remove(pItem);
    if (isBussizeItem(pItem))
        m_currentLayer->addCItem(dynamic_cast<CGraphicsItem *>(pItem));
}

void PageScene::removeCItem(QGraphicsItem *pItem, bool del, bool record)
{
    if (pItem == nullptr)
        return;

    if (pItem->scene() != this)
        return;

    CCmdBlock blocker(del ? nullptr : (record ? this : nullptr), CSceneUndoRedoCommand::EItemRemoved, QList<QGraphicsItem *>() << pItem);

    if (this->isBussizeItem(pItem) || pItem->type() == MgrType) {
        auto p = static_cast<CGraphicsItem *>(pItem);
        p->setBzGroup(nullptr);

    }

    this->removeItem(pItem);

    if (isBussizeItem(pItem))
        m_currentLayer->removeCItem(dynamic_cast<CGraphicsItem *>(pItem));

    if (del) {
        delete pItem;
    } else {
        bool isMgrVirRootItem = (pItem->type() == MgrType && static_cast<CGraphicsItemGroup *>(pItem)->groupType() == CGraphicsItemGroup::EVirRootGroup);
        bool isCutItem = (pItem->type() == CutType);
        if ((!isMgrVirRootItem) && (!isCutItem))
            m_notInSceneItems.insert(pItem);
    }
}

void PageScene::blockAssignZValue(bool b)
{
    blockZAssign = b;
}

void PageScene::blockSelectionStyle(bool b)
{
    if (b) {
        selectGroup()->setNoContent(true, false);
        CGraphicsItem::paintInteractBorderLine = false;
    } else {
        selectGroup()->setNoContent(false, false);
        CGraphicsItem::paintInteractBorderLine = true;
    }
    this->update();
}

bool PageScene::isBussizeItem(QGraphicsItem *pItem)
{
    if (pItem == nullptr)
        return false;

    return (pItem->type() >= RectType && pItem->type() < MgrType/*<= BlurType*/);
}

bool PageScene::isBussizeHandleNodeItem(QGraphicsItem *pItem)
{
    if (pItem == nullptr)
        return false;
    //CSizeHandleRect的父类QGraphicsSvgItem的类型就是13
    if (pItem->type() == QGraphicsSvgItem::Type) {
        CSizeHandleRect *pHandleItem = dynamic_cast<CSizeHandleRect *>(pItem);
        if (pHandleItem != nullptr) {
            return true;
        }
    }

    return false;
}

//bool CDrawScene::isBzAssicaitedItem(QGraphicsItem *pItem)
//{
//    return (isBussizeItem(pItem) || isBussizeHandleNodeItem(pItem));
//}

bool PageScene::isNormalGroupItem(QGraphicsItem *pItem)
{
    if (pItem == nullptr)
        return false;

    if (pItem->type() != MgrType) {
        return false;
    }
    return ((static_cast<CGraphicsItemGroup *>(pItem))->groupType() == CGraphicsItemGroup::ENormalGroup);
}

bool PageScene::isDrawItem(QGraphicsItem *pItem)
{
    return (isNormalGroupItem(pItem) || isBussizeItem(pItem));
}

CGraphicsItem *PageScene::getAssociatedBzItem(QGraphicsItem *pItem)
{
    if (pItem == nullptr)
        return nullptr;

    if (isBussizeItem(pItem)) {
        return dynamic_cast<CGraphicsItem *>(pItem);
    }

    if (isBussizeHandleNodeItem(pItem)) {
        CSizeHandleRect *pHandle = dynamic_cast<CSizeHandleRect *>(pItem);
        return dynamic_cast<CGraphicsItem *>(pHandle->parentItem());
    }
    return nullptr;
}

void PageScene::clearSelectGroup()
{
    clearSelection();
    m_pSelGroupItem->clear();
}

void PageScene::selectItem(QGraphicsItem *pItem, bool onlyBzItem, bool updateAttri, bool updateRect)
{
    if (blockSel)
        return;

    if (pItem == nullptr || pItem->scene() == nullptr)
        return;

    if ((onlyBzItem && isBussizeItem(pItem)) || isNormalGroupItem(pItem)) {
        pItem = static_cast<CGraphicsItem *>(pItem)->thisBzProxyItem(true);
        pItem->setSelected(true);
        m_pSelGroupItem->add(dynamic_cast<CGraphicsItem *>(pItem), updateAttri, updateRect);
    } else {
        pItem->setSelected(true);
    }
}

void PageScene::notSelectItem(QGraphicsItem *pItem, bool updateAttri, bool updateRect)
{
    pItem->setSelected(false);

    if (isBussizeItem(pItem) || isNormalGroupItem(pItem)) {
        m_pSelGroupItem->remove(dynamic_cast<CGraphicsItem *>(pItem), updateAttri, updateRect);
    }
}

void PageScene::blockSelect(bool b)
{
    blockSel = b;
}

void PageScene::selectItemsByRect(const QRectF &rect, bool replace, bool onlyBzItem)
{
    if (blockSel)
        return;

    if (replace)
        clearSelectGroup();

    QList<QGraphicsItem *> itemlists = this->items(rect);

    for (QGraphicsItem *pItem : itemlists) {
        if (onlyBzItem && isBussizeItem(pItem)) {

            CGraphicsItem *pCItem = dynamic_cast<CGraphicsItem *>(pItem)->thisBzProxyItem(true);

            pCItem->setSelected(true);

            // 此处可以不用刷新属性,但是文字图元修改为不同样式后导入画板进行框选,显示的属性不对,后续进行改进
            m_pSelGroupItem->add(pCItem, false, false);
        }
    }
    m_pSelGroupItem->updateAttributes();
    m_pSelGroupItem->updateBoundingRect();

    m_pSelGroupItem->setAddType(CGraphicsItemGroup::ERectSelect);
}

void PageScene::updateMrItemBoundingRect()
{
    m_pSelGroupItem->updateBoundingRect();
}

QList<CGraphicsItem *> PageScene::getBzItems(const QList<QGraphicsItem *> &items, ESortItemTp tp)
{
    bool isSort = false;
    QList<QGraphicsItem *> lists = items;
    if (lists.isEmpty()) {
        lists = this->items(tp == EDesSort ? Qt::DescendingOrder : Qt::AscendingOrder);
        isSort = true;
    }

    QList<CGraphicsItem *> result;
    for (int i = 0; i < lists.count(); ++i) {
        QGraphicsItem *pItem = lists[i];
        if (isBussizeItem(pItem)) {
            result.append(static_cast<CGraphicsItem *>(pItem));
        }
    }
    if (!isSort) {
        return returnSortZItems(result, tp);
    }
    return result;
}

QList<CGraphicsItem *> PageScene::getRootItems(PageScene::ESortItemTp tp)
{
    QList<QGraphicsItem *> lists = items(tp == EDesSort ? Qt::DescendingOrder : Qt::AscendingOrder);
    QList<CGraphicsItem *> result;
    for (auto p : lists) {
        if (isBussizeItem(p) || isNormalGroupItem(p)) {
            CGraphicsItem *pC = static_cast<CGraphicsItem *>(p);
            if (pC->bzGroup(true) == nullptr) {
                result.append(pC);
            }
        }
    }
    sortZ(result, tp);
    return result;
}

void PageScene::moveBzItemsLayer(const QList<CGraphicsItem *> &items,
                                 EZMoveType tp, int step,
                                 CGraphicsItem *pBaseInGroup,
                                 bool pushToStack)
{
    if (items.isEmpty())
        return;

    if (pushToStack) {
        //图元被加入组合时z值会发生变化所以记录z值信息
        auto list = getBzItems();
        QList<CGraphicsItem *> shouldNotFocusItems;
        for (int i = 0; i < list.count(); ++i) {
            auto p = list.at(i);
            if (!items.contains(p)) {
                shouldNotFocusItems.append(p);
            }
        }
        this->recordItemsInfoToCmd(list, UndoVar, true, shouldNotFocusItems);
    }

    switch (tp) {
    case EDownLayer: {
        moveItemsZDown(items, step);
        break;
    }
    case EUpLayer: {
        //sortZOnItemsMove(items, tp, step);
        moveItemsZUp(items, step);
        break;
    }
    case EToGroup: {
        sortZBaseOneBzItem(items, pBaseInGroup);
        break;
    }
    }

    if (pushToStack) {
        //图元被加入组合时z值会发生变化所以记录z值信息
        auto list = getBzItems();
        QList<CGraphicsItem *> shouldNotFocusItems;
        for (int i = 0; i < list.count(); ++i) {
            auto p = list.at(i);
            if (!items.contains(p)) {
                shouldNotFocusItems.append(p);
            }
        }
        this->recordItemsInfoToCmd(list, RedoVar, false, shouldNotFocusItems);
        this->finishRecord();
    }
}

//bool CDrawScene::isZMovable(const QList<CGraphicsItem *> &items,
//                            EZMoveType tp,
//                            int step,
//                            CGraphicsItem *pBaseInGroup)
//{
//    if (items.isEmpty())
//        return false;

//    bool result = false;

//    switch (tp) {
//    case EDownLayer:

//    case EUpLayer: {

//        Q_UNUSED(step)

//        CGraphicsItemGroup *pSameGroup = getSameGroup(items, false, false, true);

//        //不支持不处于同一组合下的图层操作
//        result = (pSameGroup != nullptr);

//        if (pSameGroup != nullptr && pSameGroup->groupType() == CGraphicsItemGroup::EVirRootGroup) {
//            delete pSameGroup;
//        }

//        break;
//    }
//    case EToGroup: {
//        result = items.contains(pBaseInGroup);
//        break;
//    }
//    }

//    return result;
//}

bool PageScene::isCurrentZMovable(EZMoveType tp, int step, CGraphicsItem *pBaseInGroup)
{
    Q_UNUSED(step)
    Q_UNUSED(pBaseInGroup)

    bool selectcCount = getBzItems().count() >= 2 &&  selectGroup()->count() > 0 &&  selectGroup()->getBzItems(true).count() < getBzItems().count();
    if (!selectcCount) {
        return  false;
    }

    bool result = false;

    // 获取框选所有图元且进行降序排列
    QList<CGraphicsItem *> resultSort = selectGroup()->getBzItems(true);
    auto allBzItemInSelectGroup = returnSortZItems(resultSort, ESortItemTp::EDesSort);

    switch (tp) {
    case EDownLayer: {
        if (allBzItemInSelectGroup.isEmpty())
            return  false;

        CGraphicsItem *minItems = nullptr;
        minItems  = allBzItemInSelectGroup.last();
        if (minItems != nullptr) {
            qreal min = minItems->zValue();
            if (static_cast<int >(min) <= static_cast<int >(getBzItems().last()->zValue()))
                result = false;
            else
                result = true;
        }

        break;
    }

    case EUpLayer: {
        if (allBzItemInSelectGroup.isEmpty())
            return  false;

        CGraphicsItem *maxItems = nullptr;
        maxItems  = allBzItemInSelectGroup.first();
        if (maxItems != nullptr) {
            qreal max = maxItems->zValue();
            if (static_cast<int >(max) >= static_cast<int >(getBzItems().first()->zValue()))
                result = false;
            else
                result = true;
        }

        break;
    }
    case EToGroup:
        break;
    }

    return result;
}

qreal PageScene::getMaxZValue()
{
    auto bzItems = getBzItems();
    return bzItems.isEmpty() ? -1 : bzItems.first()->zValue();
}

//降序排列用
bool zValueSortDES(QGraphicsItem *info1, QGraphicsItem *info2)
{
    if (PageScene::isDrawItem(info1) && PageScene::isDrawItem(info2))
        return static_cast<CGraphicsItem *>(info1)->drawZValue() >= static_cast<CGraphicsItem *>(info2)->drawZValue();
    return info1->zValue() >= info2->zValue();
}
//升序排列用
bool zValueSortASC(QGraphicsItem *info1, QGraphicsItem *info2)
{
    if (PageScene::isDrawItem(info1) && PageScene::isDrawItem(info2))
        return static_cast<CGraphicsItem *>(info1)->drawZValue() <= static_cast<CGraphicsItem *>(info2)->drawZValue();
    return info1->zValue() <= info2->zValue();
}

CGraphicsItem *PageScene::topBzItem(const QPointF &pos, bool penalgor, int IncW)
{
    return dynamic_cast<CGraphicsItem *>(firstItem(pos, QList<QGraphicsItem *>(),
                                                   true, penalgor, true, true, true, IncW));
}

QGraphicsItem *PageScene::firstItem(const QPointF &pos,
                                    const QList<QGraphicsItem *> &itemsCus,
                                    bool isListDesSorted,
                                    bool penalgor,
                                    bool isBzItem,
                                    bool seeNodeAsBzItem,
                                    bool filterMrAndHightLight, int incW)
{
    QList<QGraphicsItem *> items = itemsCus;

    if (itemsCus.isEmpty()) {
        if (incW == 0) {
            items = this->items(pos);
        } else {
            items = this->items(QRectF(pos - QPoint(incW, incW), 2 * QSize(incW, incW)), Qt::IntersectsItemShape);
        }
    }

    //先去掉多选图元和高亮图元
    if (filterMrAndHightLight) {
        for (int i = 0; i < items.count();) {
            QGraphicsItem *pItem = items[i];
            bool isAsscMgr = false;
            if (pItem->type() == MgrType || pItem->type() == CGraphicsProxyWidget::Type) {
                //isAsscMgr = true;
                items.removeAt(i);
                continue;
            } else if (isBussizeHandleNodeItem(pItem)) {
                if (seeNodeAsBzItem) {
                    pItem = getAssociatedBzItem(pItem);
                    isAsscMgr = (pItem != nullptr && pItem->type() == MgrType);
                }
            }

            if (isAsscMgr) {
                CGraphicsItemGroup *pSelctMrItem = dynamic_cast<CGraphicsItemGroup *>(pItem);
                QList<CGraphicsItem *> selects = pSelctMrItem->items();
                if (selects.count() == 1) {
                    items.replace(i, selects.first());
                } else {
                    items.removeAt(i);
                    continue;
                }
            }

            ++i;
        }
    }

    //如果list没有排好序那么要进行先排序
    if (!isListDesSorted) {
        sortZ(items);
    }

    if (items.isEmpty())
        return nullptr;

    if (penalgor) {
        QGraphicsItem *pResultItem = nullptr;
        QList<CGraphicsItem *> allPenetrable;
        for (int i = 0; i < items.count(); ++i) {
            QGraphicsItem *pItem = items[i];
            if (isBussizeItem(pItem)) {
                CGraphicsItem *pBzItem = dynamic_cast<CGraphicsItem *>(pItem);
                bool isPenetrable = pBzItem->isPosPenetrable(pBzItem->mapFromScene(pos));
                if (incW != 0) {
                    QRectF rect(pBzItem->mapFromScene(pos) - QPoint(incW, incW), 2 * QSize(incW, incW));
                    isPenetrable = pBzItem->isRectPenetrable(rect);
                }
                if (!isPenetrable) {
                    //在该位置不透明,判定完成
                    pResultItem = pBzItem;
                    break;
                } else {
                    allPenetrable.append(pBzItem);
                }
            } else {
                if (pItem->flags() & QGraphicsItem::ItemHasNoContents) {
                    continue;
                }
                //非业务图元无认识不透明的 ，那么就证明找到了，判定完成
                pResultItem = pItem;

                //将非业务图元的节点图元看作业务图元时的情况
                if (isBussizeHandleNodeItem(pItem)) {
                    if (seeNodeAsBzItem) {
                        CSizeHandleRect *pHandelItem = dynamic_cast<CSizeHandleRect *>(pItem);
                        CGraphicsItem *pBzItem = dynamic_cast<CGraphicsItem *>(pHandelItem->parentItem());
                        pResultItem = pBzItem;
                    }
                }

                //必须返回业务图元的情况
                if (isBzItem && !isBussizeItem(pResultItem)) {
                    pResultItem = nullptr;
                }

                break;
            }
        }
        if (pResultItem == nullptr) {
            if (!allPenetrable.isEmpty()) {
                QRectF ins = allPenetrable.first()->mapToScene(allPenetrable.first()->boundingRect()).boundingRect();
                for (auto it : allPenetrable) {
                    QRectF rect = it->mapToScene(it->boundingRect()).boundingRect();
                    ins = ins.intersected(rect);
                }
                qreal maxRadio = 0;
                for (auto it : allPenetrable) {
                    QRectF rect = it->mapToScene(it->boundingRect()).boundingRect();
                    qreal radio = (ins.width() * ins.height()) / (rect.width() * rect.height());
                    if (radio > maxRadio) {
                        maxRadio = radio;
                        pResultItem = it;
                    }
                }
            }
        }
        return pResultItem;
    }
    QGraphicsItem *pRetItem = nullptr;
    if (isBzItem) {
        for (auto item : items) {
            if (isBussizeItem(item)) {
                pRetItem = item;
                break;
            } else {
                if (seeNodeAsBzItem && isBussizeHandleNodeItem(item)) {
                    pRetItem = getAssociatedBzItem(item);
                    break;
                }
            }
        }
    } else {
        pRetItem = items.isEmpty() ? nullptr : items.first();
    }
    return pRetItem;
}

// Cppcheck检测函数没有使用到
//QList<CGraphicsItem *> CDrawScene::findBzItems(const QPointF &pos,
//                                               bool seeNodeAsBzItem,
//                                               bool filterGroup,
//                                               int incW)
//{
//    QList<QGraphicsItem *> items;
//    if (incW == 0) {
//        items = this->items(pos);
//    } else {
//        items = this->items(QRectF(pos - QPoint(incW, incW), 2 * QSize(incW, incW)), Qt::IntersectsItemShape);
//    }
//    QList<CGraphicsItem *> resultItems;
//    for (auto p : items) {
//        if (isDrawItem(p)) {
//            if (filterGroup) {
//                if (!isNormalGroupItem(p)) {
//                    resultItems.append(static_cast<CGraphicsItem *>(p));
//                }
//            } else {
//                resultItems.append(static_cast<CGraphicsItem *>(p));
//            }
//        } else {
//            if (seeNodeAsBzItem && isBussizeHandleNodeItem(p)) {
//                auto pItem = getAssociatedBzItem(p);
//                if (filterGroup) {
//                    if (!isNormalGroupItem(pItem)) {
//                        resultItems.append(pItem);
//                    }
//                } else {
//                    resultItems.append(pItem);
//                }
//            }
//        }
//    }
//    return resultItems;
//}

void PageScene::blockMouseMoveEvent(bool b)
{
    blockMouseMoveEventFlag = b;
}

bool PageScene::isBlockMouseMoveEvent()
{
    return blockMouseMoveEventFlag;
}

void PageScene::recordSecenInfoToCmd(int exptype, EVarUndoOrRedo varFor, const QList<CGraphicsItem *> &items)
{
    QList<QVariant> vars;
    vars << reinterpret_cast<long long>(this);

    QVariant var;
    CSceneUndoRedoCommand::EChangedType tp = CSceneUndoRedoCommand::EChangedType(exptype);

    //当前仅支持EGroupChanged的场景快照还原
    switch (tp) {
    case CSceneUndoRedoCommand::EGroupChanged: {
        var.setValue<CGroupBzItemsTree>(getGroupTree());
        vars << var;

        //真正的变动图元,可以用于撤销还原后的选中操作
        for (QGraphicsItem *pItem : items) {
            vars << reinterpret_cast<long long>(pItem);
        }

        break;
    }
    default:
        return;
    }

    if (varFor == UndoVar) {
        CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::ESceneChangedCmd, exptype,
                                            vars, true, false);
    } else if (varFor == RedoVar) {
        CUndoRedoCommand::recordRedoCommand(CUndoRedoCommand::ESceneChangedCmd, exptype,
                                            vars);
    }
}

void PageScene::recordItemsInfoToCmd(const QList<CGraphicsItem *> &items, EVarUndoOrRedo varFor, bool clearInfo,
                                     const QList<CGraphicsItem *> &shoudNotSelectItems)
{
    //qWarning() << "recordItemsInfoToCmd ============ " << int(varFor);
    for (int i = 0; i < items.size(); ++i) {
        CGraphicsItem *pItem = items[i];
        QList<QVariant> vars;
        vars << reinterpret_cast<long long>(pItem);
        QVariant varInfo;
        varInfo.setValue(pItem->getGraphicsUnit(EUndoRedo));
        vars << varInfo;
        if (shoudNotSelectItems.contains(pItem)) {
            vars << true;
        }

        if (varFor == UndoVar) {
            CUndoRedoCommand::recordUndoCommand(CUndoRedoCommand::EItemChangedCmd,
                                                CItemUndoRedoCommand::EAllChanged, vars, clearInfo && (i == 0), false);
        } else {
            CUndoRedoCommand::recordRedoCommand(CUndoRedoCommand::EItemChangedCmd,
                                                CItemUndoRedoCommand::EAllChanged, vars);
        }
    }
}

void PageScene::finishRecord(bool doRedoCmd)
{
    CUndoRedoCommand::finishRecord(doRedoCmd);
}

bool PageScene::isGroupable(const QList<CGraphicsItem *> &pBzItems) const
{
    QList<CGraphicsItem *> bzItems = pBzItems;
    if (pBzItems.isEmpty()) {
        bzItems = selectGroup()->items();
    }

    //组合个数必须是大于1的
    if (bzItems.count() <= 1)
        return false;

    return true;
}

bool PageScene::isUnGroupable(const QList<CGraphicsItem *> &pBzItems) const
{
    QList<CGraphicsItem *> bzItems = pBzItems;
    if (pBzItems.isEmpty()) {
        bzItems = selectGroup()->items();
    }

    for (auto pItem : bzItems) {
        pItem = pItem->thisBzProxyItem(true);
        if (isNormalGroupItem(pItem))
            return  true;
    }

    return false;
}

CGraphicsItemGroup *PageScene::getSameGroup(const QList<CGraphicsItem *> &pBzItems, bool top,
                                            bool onlyNormal,
                                            bool sameNullCreatVirGroup)
{
    if (pBzItems.isEmpty())
        return nullptr;

    CGraphicsItemGroup *pGroup = top ? pBzItems.first()->bzTopGroup(onlyNormal) : pBzItems.first()->bzGroup(onlyNormal);
    bool isSameNullptr = (pGroup == nullptr);
    for (int i = 1; i < pBzItems.size(); ++i) {
        CGraphicsItem *p = pBzItems[i];
        CGraphicsItemGroup *pTopGroupTemp = top ? p->bzTopGroup(onlyNormal) : p->bzGroup(onlyNormal);
        if (pGroup != pTopGroupTemp) {
            pGroup = nullptr;
            isSameNullptr = false;
            break;
        }
    }

    if (isSameNullptr && !top && sameNullCreatVirGroup) {
        //创建一个虚拟管理组合,用完记得删除
        CGraphicsItemGroup *pNewGroup = new CGraphicsItemGroup;
        for (auto p : pBzItems) {
            pNewGroup->add(p, false, false);
        }
        pNewGroup->setGroupType(CGraphicsItemGroup::EVirRootGroup);
        return pNewGroup;
    }

    return /*nullptr*/pGroup;
}

CGraphicsItemGroup *PageScene::creatGroup(const QList<CGraphicsItem *> &pBzItems,
                                          int groupType,
                                          bool pushUndo,
                                          CGraphicsItem *pBzItemZBase, bool getMinZItemIfNull)
{
    QList<CGraphicsItem *> bzItems = pBzItems;
    if (pBzItems.isEmpty()) {
        bzItems = selectGroup()->items();
    }
    if (bzItems.count() == 0)
        return nullptr;

    //普通组合个数必须是大于1的,其他特殊的组合没有此限制
    if (groupType == CGraphicsItemGroup::ENormalGroup && bzItems.count() <= 1)
        return nullptr;

    //pBzItems中不能有图元在某个组合下
    for (auto p : pBzItems) {
        if (p->bzGroup() != nullptr)
            return nullptr;
    }

    CCmdBlock block(pushUndo ? this : nullptr, CSceneUndoRedoCommand::EGroupChanged,
                    CGraphicsItem::returnList(bzItems));
    if (pushUndo) {
        //图元被加入组合时z值会发生变化所以记录z值信息
        this->recordItemsInfoToCmd(getBzItems(), UndoVar, false);
    }
    //CGraphicsItemGroup *pNewGroup = m_pCachGroups.isEmpty() ? new CGraphicsItemGroup : m_pCachGroups.takeFirst();
    CGraphicsItemGroup *pNewGroup = new CGraphicsItemGroup;

    pNewGroup->setGroupType(CGraphicsItemGroup::EGroupType(groupType));

    for (auto it : bzItems) {
        it->setBzGroup(pNewGroup);
    }

    pNewGroup->setRecursiveScene(this);

    pNewGroup->updateBoundingRect();

    if (!m_pGroups.contains(pNewGroup))
        m_pGroups.append(pNewGroup);

    selectItem(pNewGroup);

    qDebug() << "in used groups count = " << m_pGroups.count() << "cached groups = " << m_pCachGroups.count();

    //排序z
    if (pBzItemZBase == nullptr && getMinZItemIfNull) {
        pBzItemZBase = CGraphicsItem::zItem(bzItems);
    }
    if (pBzItemZBase != nullptr) {
        sortZBaseOneBzItem(pNewGroup->getBzItems(true), pBzItemZBase);
    }

    if (pushUndo) {
        //图元被加入组合时z值会发生变化所以记录z值信息
        this->recordItemsInfoToCmd(getBzItems(), RedoVar, false);
    }

    return pNewGroup;
}

CGraphicsItemGroup *PageScene::copyCreatGroup(CGraphicsItemGroup *pGroup)
{
    if (pGroup == nullptr || pGroup->drawScene() == nullptr)
        return nullptr;

    PageScene *pScene = pGroup->drawScene();
    CGroupBzItemsTreeInfo itemsTreeInfo = pScene->getGroupTreeInfo(pGroup);
    CGraphicsItemGroup *pNewGroup = pScene->loadGroupTreeInfo(itemsTreeInfo, true);
    releaseBzItemsTreeInfo(itemsTreeInfo);
    return pNewGroup;
}

void PageScene::cancelGroup(CGraphicsItemGroup *pGroup, bool pushUndo)
{
    QList<CGraphicsItem *> itemlists;
    if (pGroup == nullptr) {
        QList<CGraphicsItem *> bzItems = selectGroup()->items();
        QList<CGraphicsItemGroup *> pGroups;
        for (auto pItem : bzItems) {
            pItem = pItem->thisBzProxyItem(true);
            if (isNormalGroupItem(pItem)) {
                pGroup = static_cast<CGraphicsItemGroup *>(pItem);
                if (pGroup != nullptr && pGroup->isCancelable()) {
                    itemlists.append(pGroup->items());
                    pGroups.append(pGroup);
                }

            }
        }
        CCmdBlock block(pushUndo ? this : nullptr, CSceneUndoRedoCommand::EGroupChanged,
                        CGraphicsItem::returnList(itemlists));

        for (auto pGroup : pGroups) {
            destoryGroup(pGroup, false, false);
        }
        qDebug() << "in used groups count = " << m_pGroups.count() << "cached groups = " << m_pCachGroups.count();
    } else {
        if (pGroup->isCancelable()) {
            itemlists = pGroup->items();
            destoryGroup(pGroup, pGroup->groupType() == CGraphicsItemGroup::EVirRootGroup, pushUndo);
        }
    }

    if (!itemlists.isEmpty())
        clearSelectGroup();
    // 取消组合需要还原框选状态
    foreach (CGraphicsItem *pCItem, itemlists) {
        selectItem(pCItem, true, false, false);
    }
    // 更新边界矩形框
    m_pSelGroupItem->updateAttributes();
    m_pSelGroupItem->updateBoundingRect();
    m_pSelGroupItem->setAddType(CGraphicsItemGroup::ERectSelect);
}

void PageScene::destoryGroup(CGraphicsItemGroup *pGroup, bool deleteIt, bool pushUndo)
{
    if (pGroup == nullptr)
        return;

    //保证组合是这个场景的
    assert(pGroup->scene() == this);

    CCmdBlock block(pushUndo ? this : nullptr, CSceneUndoRedoCommand::EGroupChanged, CGraphicsItem::returnList(pGroup->items()));

    if (pGroup->isSelected())
        notSelectItem(pGroup);

    this->removeCItem(pGroup);

    m_pGroups.removeOne(pGroup);

    pGroup->clear();

    pGroup->setGroupType(CGraphicsItemGroup::ENormalGroup);

    Q_UNUSED(deleteIt)
    /*if (deleteIt) {
        m_pGroups.removeOne(pGroup);
        delete pGroup;
    } else */{
        pGroup->resetTransform();
        m_pCachGroups.append(pGroup);
    }
}

void PageScene::destoryAllGroup(bool deleteIt, bool pushUndo)
{
    CCmdBlock block(pushUndo ? this : nullptr, CSceneUndoRedoCommand::EGroupChanged);
    while (!m_pGroups.isEmpty()) {
        destoryGroup(m_pGroups.takeFirst(), deleteIt);
    }
}

QList<CGraphicsItemGroup *> PageScene::bzGroups()
{
    return m_pGroups;
}

QColor PageScene::systemThemeColor() const
{
    DPalette pa = this->palette();
    QBrush selectBrush = pa.brush(QPalette::Active, DPalette::Highlight);
    return selectBrush.color();
}

//QImage &PageScene::sceneExImage()
//{
//    return m_currentLayer->layerImage();
//}

QImage PageScene::renderToImage(const QColor &bgColor, const QSize &desImageSize)
{
    QImage image;

    auto scene = this;

    //render前屏蔽掉多选框和选中的边线显示(之后恢复)
    this->blockSelectionStyle(true);

    auto cachedColor = this->bgColor();
    auto cachedBrush = this->backgroundBrush();

    this->setBgColor(bgColor);
    this->setBackgroundBrush(Qt::transparent);

    image = QImage(desImageSize.isValid() ? desImageSize : scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    this->render(&painter, image.rect(), scene->sceneRect(), Qt::IgnoreAspectRatio);

    //render后恢复屏蔽掉的多选框和选中的边线显示
    this->blockSelectionStyle(false);

    this->setBgColor(cachedColor);
    this->setBackgroundBrush(cachedBrush);

    return image;
}
