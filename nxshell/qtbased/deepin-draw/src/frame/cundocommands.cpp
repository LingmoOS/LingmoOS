// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cundocommands.h"

#include "globaldefine.h"
#include "cgraphicslineitem.h"
#include "cgraphicsrectitem.h"
#include "cgraphicspolygonitem.h"
#include "cgraphicspolygonalstaritem.h"
#include "cgraphicspenitem.h"
#include "cdrawparamsigleton.h"
#include "cdrawscene.h"
#include "cgraphicslineitem.h"
//#include "cgraphicsmasicoitem.h"
#include "cgraphicsitemselectedmgr.h"
#include "cgraphicstextitem.h"
#include "cpictureitem.h"

#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "cgraphicsitemselectedmgr.h"
#include "widgets/ctextedit.h"

#include <QUndoCommand>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QBrush>
#include <QDebug>

//升序排列用
//bool zValueSortASC(QGraphicsItem *info1, QGraphicsItem *info2)
//{
//    return info1->zValue() <= info2->zValue();
//}
////降序排列用
//bool zValueSortDES(QGraphicsItem *info1, QGraphicsItem *info2)
//{
//    return info1->zValue() >= info2->zValue();
//}

//COneLayerUpCommand::COneLayerUpCommand(CDrawScene *scene, const QList<QGraphicsItem *> &items, QUndoCommand *parent)
//    : QUndoCommand(parent)
//{
//    myGraphicsScene = scene;
//    QList<QGraphicsItem *> curItems;
//    for (auto item : scene->items(Qt::AscendingOrder)) {
//        if (item->type() > QGraphicsItem::UserType && item->type() < EGraphicUserType::MgrType) {
//            if (curItems.isEmpty()) {
//                if (items.contains(item)) {
//                    curItems.append(item);
//                }
//            } else {
//                if (items.contains(item)) {
//                    curItems.append(item);
//                } else {
//                    m_items.append(qMakePair<QGraphicsItem *, QList<QGraphicsItem *>>(item, curItems));
//                    curItems.clear();
//                }
//            }
//        }
//    }
//    m_oldItemZValue.clear();
//    m_selectItems = items;
//    m_isUndoExcuteSuccess = true;
//    m_isRedoExcuteSuccess = false;
//}

//void COneLayerUpCommand::undo()
//{
////    qDebug() << "COneLayerUpCommand undo";
////    if (!m_isRedoExcuteSuccess) {
////        return;
////    }

////    QList<QGraphicsItem *> itemList = myGraphicsScene->items();

////    m_isRedoExcuteSuccess = false;

////    int count = m_oldItemZValue.count();
////    for (int i = 0; i < count; i++) {
////        QGraphicsItem *item = m_oldItemZValue.keys().at(i);
////        item->setZValue(m_oldItemZValue[item]);
////        //myGraphicsScene->updateBlurItem(item);
////        m_isRedoExcuteSuccess = true;
////    }
////    //重置保存的最大z值
////    QList<QGraphicsItem *> allItems = myGraphicsScene->items();
////    for (int i = allItems.size() - 1; i >= 0; i--) {
////        QGraphicsItem *allItem = allItems.at(i);
////        if (allItem->type() <= QGraphicsItem::UserType || allItem->type() >= EGraphicUserType::MgrType) {
////            continue;
////        }
////        if (allItem->zValue() > myGraphicsScene->getMaxZValue()) {
////            myGraphicsScene->setMaxZValue(allItem->zValue());
////        }
////    }

////    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->selectGroup()->updateBoundingRect();
////    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->refreshLook();

////    if (m_isRedoExcuteSuccess) {
////        myGraphicsScene->update();
////        myGraphicsScene->setModify(true);
////    }
////    myGraphicsScene->updateBlurItem();
//}

//void COneLayerUpCommand::redo()
//{
////    qDebug() << "COneLayerUpCommand redo";
////    if (!m_isUndoExcuteSuccess) {
////        return;
////    }

////    m_isRedoExcuteSuccess = false;
////    //移除z值为0的和多选管理图元
////    QList<QGraphicsItem *> allItems = myGraphicsScene->items();
////    for (int i = allItems.size() - 1; i >= 0; i--) {
////        if (allItems.at(i)->zValue() == 0.0) {
////            allItems.removeAt(i);
////            continue;
////        }
////        if (allItems[i]->type() <= QGraphicsItem::UserType || allItems[i]->type() >= EGraphicUserType::MgrType) {
////            allItems.removeAt(i);
////            continue;
////        }
////    }
////    //升序排列
////    qSort(m_selectItems.begin(), m_selectItems.end(), zValueSortASC);
////    for (int selectIndex = 0; selectIndex < m_selectItems.size(); selectIndex++) {
////        QGraphicsItem *selectItem = m_selectItems.at(selectIndex);
////        QGraphicsItem *selectItemNext = nullptr;
////        if ((selectIndex + 1) < m_selectItems.size()) {
////            selectItemNext = m_selectItems.at(selectIndex + 1);
////        }
////        m_isRedoExcuteSuccess = true;
////        qSort(allItems.begin(), allItems.end(), zValueSortASC);
////        for (int allItemIndex = 0; allItemIndex < allItems.size(); allItemIndex++) {
////            QGraphicsItem *allItem = allItems.at(allItemIndex);
////            if (allItem->zValue() > selectItem->zValue()) {
////                if (allItem == selectItemNext) {
////                    break;
////                }
////                //记录原来z值
////                m_oldItemZValue[allItem] = allItem->zValue();
////                m_oldItemZValue[selectItem] = selectItem->zValue();

////                qreal tmpZValue = selectItem->zValue();
////                selectItem->setZValue(allItem->zValue());
////                allItem->setZValue(tmpZValue);
////                break;
////            }
////        }
////    }
////    //重置保存的最大z值
////    allItems = myGraphicsScene->items();
////    for (int i = allItems.size() - 1; i >= 0; i--) {
////        QGraphicsItem *allItem = allItems.at(i);
////        if (allItem->type() <= QGraphicsItem::UserType || allItem->type() >= EGraphicUserType::MgrType) {
////            continue;
////        }
////        if (allItem->zValue() > myGraphicsScene->getMaxZValue()) {
////            myGraphicsScene->setMaxZValue(allItem->zValue());
////        }
////    }

////    /* 刷新属性展示和多选的大小 */
////    CGraphicsItemGroup *pMgr = CManageViewSigleton::GetInstance()->getCurView()->drawScene()->selectGroup();
////    pMgr->updateAttributes();
////    pMgr->updateBoundingRect();

////    /* 刷新鼠标和高亮 */
////    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->refreshLook();

////    if (m_isRedoExcuteSuccess) {
////        myGraphicsScene->update();
////        myGraphicsScene->setModify(true);
////    }
////    myGraphicsScene->updateBlurItem();
//}

//COneLayerDownCommand::COneLayerDownCommand(CDrawScene *scene, const QList<QGraphicsItem *> &items, QUndoCommand *parent)
//    : QUndoCommand(parent)
//{
//    myGraphicsScene = scene;
//    QList<QGraphicsItem *> curItems;
//    QGraphicsItem *preItem = nullptr;
//    for (auto item : scene->items(Qt::AscendingOrder)) {
//        if (item->type() > QGraphicsItem::UserType && item->type() < EGraphicUserType::MgrType) {
//            if (items.contains(item)) {
//                if (preItem != nullptr) {
//                    curItems.append(item);
//                }
//            } else {
//                if (!curItems.isEmpty()) {
//                    m_items.append(qMakePair<QGraphicsItem *, QList<QGraphicsItem *>>(preItem, curItems));
//                    curItems.clear();
//                }
//                preItem = item;
//            }
//        }
//    }
//    if (preItem != nullptr && !curItems.isEmpty()) {
//        m_items.append(qMakePair<QGraphicsItem *, QList<QGraphicsItem *>>(preItem, curItems));
//    }
//    m_oldItemZValue.clear();
//    m_selectItems = items;
//    m_isUndoExcuteSuccess = true;
//    m_isRedoExcuteSuccess = false;
//}

//COneLayerDownCommand::~COneLayerDownCommand()
//{
//}

//void COneLayerDownCommand::undo()
//{
////    qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!downUndo";
////    if (!m_isRedoExcuteSuccess) {
////        return;
////    }

////    m_isRedoExcuteSuccess = false;

////    int count = m_oldItemZValue.count();
////    for (int i = 0; i < count; i++) {
////        QGraphicsItem *item = m_oldItemZValue.keys().at(i);
////        item->setZValue(m_oldItemZValue[item]);
////        //myGraphicsScene->updateBlurItem(item);
////        m_isRedoExcuteSuccess = true;
////    }
////    //重置保存的最大z值
////    QList<QGraphicsItem *> allItems = myGraphicsScene->items();
////    for (int i = allItems.size() - 1; i >= 0; i--) {
////        QGraphicsItem *allItem = allItems.at(i);
////        if (allItem->type() <= QGraphicsItem::UserType || allItem->type() >= EGraphicUserType::MgrType) {
////            continue;
////        }
////        if (allItem->zValue() > myGraphicsScene->getMaxZValue()) {
////            myGraphicsScene->setMaxZValue(allItem->zValue());
////        }
////    }

////    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->selectGroup()->updateBoundingRect();
////    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->refreshLook();

////    if (m_isRedoExcuteSuccess) {
////        myGraphicsScene->update();
////        myGraphicsScene->setModify(true);
////    }
////    myGraphicsScene->updateBlurItem();
//}

//void COneLayerDownCommand::redo()
//{
////    qDebug() << "COneLayerDownCommand::redo";
////    if (!m_isUndoExcuteSuccess) {
////        return;
////    }

////    m_isRedoExcuteSuccess = false;
////    //移除z值为0的和多选管理图元
////    QList<QGraphicsItem *> allItems = myGraphicsScene->items();
////    for (int i = allItems.size() - 1; i >= 0; i--) {
////        if (allItems.at(i)->zValue() == 0.0) {
////            allItems.removeAt(i);
////            continue;
////        }
////        if (allItems[i]->type() <= QGraphicsItem::UserType || allItems[i]->type() >= EGraphicUserType::MgrType) {
////            allItems.removeAt(i);
////            continue;
////        }
////    }
////    qSort(m_selectItems.begin(), m_selectItems.end(), zValueSortDES);
////    QGraphicsItem *selectItemNext = nullptr;
////    for (int selectIndex = 0; selectIndex < m_selectItems.size(); selectIndex++) {
////        QGraphicsItem *selectItem = m_selectItems.at(selectIndex);
////        selectItemNext = nullptr;
////        if ((selectIndex + 1) < m_selectItems.size()) {
////            selectItemNext = m_selectItems.at(selectIndex + 1);
////        }
////        m_isRedoExcuteSuccess = true;
////        qSort(allItems.begin(), allItems.end(), zValueSortDES);
////        for (int allItemIndex = 0; allItemIndex < allItems.size(); allItemIndex++) {
////            QGraphicsItem *allItem = allItems.at(allItemIndex);
////            if (allItem->zValue() < selectItem->zValue()) {
////                if (allItem == selectItemNext) {
////                    break;
////                }
////                //记录原来z值
////                m_oldItemZValue[allItem] = allItem->zValue();
////                m_oldItemZValue[selectItem] = selectItem->zValue();

////                qreal tmpZValue = selectItem->zValue();
////                selectItem->setZValue(allItem->zValue());
////                allItem->setZValue(tmpZValue);
////                break;
////            }
////        }
////    }
////    //重置保存的最大z值
////    allItems = myGraphicsScene->items();
////    for (int i = allItems.size() - 1; i >= 0; i--) {
////        QGraphicsItem *allItem = allItems.at(i);
////        if (allItem->type() <= QGraphicsItem::UserType || allItem->type() >= EGraphicUserType::MgrType) {
////            continue;
////        }
////        if (allItem->zValue() > myGraphicsScene->getMaxZValue()) {
////            myGraphicsScene->setMaxZValue(allItem->zValue());
////        }
////    }

////    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->selectGroup()->updateBoundingRect();
////    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->refreshLook();

////    if (m_isRedoExcuteSuccess) {
////        myGraphicsScene->update();
////        myGraphicsScene->setModify(true);
////    }

////    myGraphicsScene->updateBlurItem();
//}

//CBringToFrontCommand::CBringToFrontCommand(CDrawScene *scene, const QList<QGraphicsItem *> &items, QUndoCommand *parent)
//    : QUndoCommand(parent)
//{
////    myGraphicsScene = scene;
////    for (auto item : scene->items()) {
////        if (item->type() > QGraphicsItem::UserType && item->type() < EGraphicUserType::MgrType) {
////            if (items.contains(item)) {
////                m_items.append(item);
////            }
////        }
////    }
////    QList<QGraphicsItem *> curItems;
////    for (auto item : scene->items(Qt::AscendingOrder)) {
////        if (item->type() > QGraphicsItem::UserType && item->type() < EGraphicUserType::MgrType) {
////            if (curItems.isEmpty()) {
////                if (items.contains(item)) {
////                    curItems.append(item);
////                }
////            } else {
////                if (items.contains(item)) {
////                    curItems.append(item);
////                } else {
////                    m_changedItems.append(qMakePair<QGraphicsItem *, QList<QGraphicsItem *>>(item, curItems));
////                    curItems.clear();
////                }
////            }
////        }
////    }
////    for (auto item : items) {
////        if (item->type() > QGraphicsItem::UserType && item->type() < EGraphicUserType::MgrType) {
////            m_oldItemZValue[item] = item->zValue();
////        }
////    }
////    m_selectItems = items;
//}

//void CBringToFrontCommand::undo()
//{
////    bool modifyFlag = false;
////    int count = m_oldItemZValue.count();
////    for (int i = 0; i < count; i++) {
////        QGraphicsItem *item = m_oldItemZValue.keys().at(i);
////        item->setZValue(m_oldItemZValue[item]);
////        modifyFlag = true;
////    }
////    //重置保存的最大z值
////    QList<QGraphicsItem *> allItems = myGraphicsScene->items();
////    for (int i = allItems.size() - 1; i >= 0; i--) {
////        QGraphicsItem *allItem = allItems.at(i);
////        if (allItem->type() <= QGraphicsItem::UserType || allItem->type() >= EGraphicUserType::MgrType) {
////            continue;
////        }
////        if (allItem->zValue() > myGraphicsScene->getMaxZValue()) {
////            myGraphicsScene->setMaxZValue(allItem->zValue());
////        }
////    }

////    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->selectGroup()->updateBoundingRect();
////    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->refreshLook();

////    if (modifyFlag) {
////        myGraphicsScene->update();
////        myGraphicsScene->setModify(true);
////    }

////    myGraphicsScene->updateBlurItem();
//}

//void CBringToFrontCommand::redo()
//{
////    bool modifyFlag = false;

////    qSort(m_selectItems.begin(), m_selectItems.end(), zValueSortASC);
////    qreal maxZValue = myGraphicsScene->getMaxZValue();
////    for (int selectIndex = 0; selectIndex < m_selectItems.size(); selectIndex++) {
////        QGraphicsItem *selectItem = m_selectItems.at(selectIndex);
////        maxZValue = myGraphicsScene->getMaxZValue();
////        selectItem->setZValue(maxZValue + 1);
////        myGraphicsScene->setMaxZValue(maxZValue + 1);
////        modifyFlag = true;
////    }

////    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->selectGroup()->updateBoundingRect();
////    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->refreshLook();

////    if (modifyFlag) {
////        myGraphicsScene->update();
////        myGraphicsScene->setModify(true);
////    }
////    myGraphicsScene->updateBlurItem();
//}

//CSendToBackCommand::CSendToBackCommand(CDrawScene *scene, const QList<QGraphicsItem *> &items, QUndoCommand *parent)
//    : QUndoCommand(parent)
//{
//    myGraphicsScene = scene;
//    for (auto item : scene->items()) {
//        if (item->type() > QGraphicsItem::UserType && item->type() < EGraphicUserType::MgrType) {
//            if (items.contains(item)) {
//                m_items.append(item);
//            }
//        }
//    }
//    QList<QGraphicsItem *> curItems;
//    QGraphicsItem *preItem = nullptr;
//    for (auto item : scene->items(Qt::AscendingOrder)) {
//        if (item->type() > QGraphicsItem::UserType && item->type() < EGraphicUserType::MgrType) {
//            if (items.contains(item)) {
//                if (preItem != nullptr) {
//                    curItems.append(item);
//                }
//            } else {
//                if (!curItems.isEmpty()) {
//                    m_changedItems.append(qMakePair<QGraphicsItem *, QList<QGraphicsItem *>>(preItem, curItems));
//                    curItems.clear();
//                }
//                preItem = item;
//            }
//        }
//    }
//    if (preItem != nullptr && !curItems.isEmpty()) {
//        m_changedItems.append(qMakePair<QGraphicsItem *, QList<QGraphicsItem *>>(preItem, curItems));
//    }
//    m_oldItemZValue.clear();
//    for (auto item : items) {
//        if (item->type() > QGraphicsItem::UserType && item->type() < EGraphicUserType::MgrType) {
//            m_oldItemZValue[item] = item->zValue();
//        }
//    }
//    m_selectItems = items;
//}

//CSendToBackCommand::~CSendToBackCommand()
//{
//}

//void CSendToBackCommand::undo()
//{
//    qDebug() << "CSendToBackCommand::undo";

//    bool modifyFlag = false;
//    int count = m_oldItemZValue.count();
//    for (int i = 0; i < count; i++) {
//        QGraphicsItem *item = m_oldItemZValue.keys().at(i);
//        item->setZValue(m_oldItemZValue[item]);
//        //myGraphicsScene->updateBlurItem(item);
//        modifyFlag = true;
//    }
//    //重置保存的最大z值
//    QList<QGraphicsItem *> allItems = myGraphicsScene->items();
//    qreal maxZvalue = 0;
//    for (int i = allItems.size() - 1; i >= 0; i--) {
//        QGraphicsItem *allItem = allItems.at(i);
//        if (allItem->type() <= QGraphicsItem::UserType || allItem->type() >= EGraphicUserType::MgrType) {
//            continue;
//        }
//        if (allItem->zValue() > maxZvalue) {
//            maxZvalue = allItem->zValue();
//        }
//    }
//    myGraphicsScene->setMaxZValue(maxZvalue);

//    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->selectGroup()->updateBoundingRect();
//    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->refreshLook();

//    if (modifyFlag) {
//        myGraphicsScene->update();
//        myGraphicsScene->setModify(true);
//    }
//    myGraphicsScene->updateBlurItem();
//}

//void CSendToBackCommand::redo()
//{
//    qDebug() << "CSendToBackCommand::redo";

//    bool modifyFlag = false;

//    QList<QGraphicsItem *> allItems = myGraphicsScene->items();
//    for (int i = allItems.size() - 1; i >= 0; i--) {
//        QGraphicsItem *allItem = allItems.at(i);
//        if (allItems.at(i)->zValue() == 0.0) {
//            allItems.removeAt(i);
//            continue;
//        }
//        if (allItem->type() <= QGraphicsItem::UserType || allItem->type() >= EGraphicUserType::MgrType) {
//            allItems.removeAt(i);
//            continue;
//        }
//        for (int j = 0; j < m_selectItems.size(); j++) {
//            QGraphicsItem *selectItem = m_selectItems.at(j);
//            if (allItem == selectItem) {
//                allItems.removeAt(i);
//                break;
//            }
//        }
//    }

//    qSort(m_selectItems.begin(), m_selectItems.end(), zValueSortASC);
//    myGraphicsScene->setMaxZValue(m_selectItems.last()->zValue());
//    qSort(allItems.begin(), allItems.end(), zValueSortASC);
//    for (int allItemIndex = 0; allItemIndex < allItems.size(); allItemIndex++) {
//        QGraphicsItem *allItem = allItems.at(allItemIndex);
//        m_oldItemZValue[allItem] = allItem->zValue();
//        allItem->setZValue(myGraphicsScene->getMaxZValue() + 1);
//        myGraphicsScene->setMaxZValue(myGraphicsScene->getMaxZValue() + 1);

//        modifyFlag = true;
//    }

//    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->selectGroup()->updateBoundingRect();
//    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->refreshLook();

//    if (modifyFlag) {
//        myGraphicsScene->update();
//        myGraphicsScene->setModify(true);
//    }
//    myGraphicsScene->updateBlurItem();
//}

CSceneCutCommand::CSceneCutCommand(PageScene *scene, QRectF rect, QUndoCommand *parent, CGraphicsItem *item)
    : QUndoCommand(parent)
    , m_newRect(rect)
    , m_oldRect(scene->sceneRect())
{
    myGraphicsScene = scene;
    m_item = item;
}

CSceneCutCommand::~CSceneCutCommand()
{
}

void CSceneCutCommand::undo()
{
    myGraphicsScene->setSceneRect(m_oldRect);
    myGraphicsScene->update();

    //myGraphicsScene->updateBlurItem();
}

void CSceneCutCommand::redo()
{
    myGraphicsScene->setSceneRect(m_newRect);
    myGraphicsScene->update();

    //myGraphicsScene->updateBlurItem();
}

CItemsAlignCommand::CItemsAlignCommand(PageScene *scene, QMap<CGraphicsItem *, QPointF> startPos, QMap<CGraphicsItem *, QPointF> endPos)
    : myGraphicsScene(scene)
    , m_itemsStartPos(startPos)
    , m_itemsEndPos(endPos)
    , m_isMoved(true)
{
}

void CItemsAlignCommand::undo()
{
    qDebug() << "CItemsAlignCommand::undo";
    if (m_isMoved) {
        QList<CGraphicsItem *> allItems = m_itemsStartPos.keys();
        QList<QPointF> itemsStartPos = m_itemsStartPos.values();
        QList<QPointF> itemsEndPos = m_itemsEndPos.values();

        for (int i = 0; i < allItems.size(); i++) {
            allItems.at(i)->move(itemsEndPos.at(i), itemsStartPos.at(i));
        }
    }
    m_isMoved = false;

    // 手动刷新重做后的多选框线
    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->selectGroup()->updateBoundingRect();
    //CManageViewSigleton::GetInstance()->getCurView()->drawScene()->refreshLook();
}

void CItemsAlignCommand::redo()
{
    qDebug() << "CItemsAlignCommand::redo";
    if (!m_isMoved) {
        QList<CGraphicsItem *> allItems = m_itemsStartPos.keys();
        QList<QPointF> itemsStartPos = m_itemsStartPos.values();
        QList<QPointF> itemsEndPos = m_itemsEndPos.values();

        for (int i = 0; i < allItems.size(); i++) {
            allItems.at(i)->move(itemsStartPos.at(i), itemsEndPos.at(i));
        }
    }
    m_isMoved = true;

    // 手动刷新重做后的多选框线
    CManageViewSigleton::GetInstance()->getCurView()->drawScene()->selectGroup()->updateBoundingRect();
    //CManageViewSigleton::GetInstance()->getCurView()->drawScene()->refreshLook();
}
