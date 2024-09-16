// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cshapemimedata.h"
#include "cgraphicsitem.h"
#include "cgraphicsrectitem.h"
#include "cgraphicsellipseitem.h"
#include "cgraphicslineitem.h"
#include "cgraphicstriangleitem.h"
#include "cgraphicspolygonitem.h"
#include "cgraphicspolygonalstaritem.h"
#include "cgraphicstextitem.h"
#include "cgraphicspenitem.h"
#include "cpictureitem.h"
#include "cdrawscene.h"

//CShapeMimeData::CShapeMimeData(QList<QGraphicsItem *> items)
//{
//    m_itemList.clear();
//    foreach (QGraphicsItem *item, items) {
//        CGraphicsItem *pBzItem = dynamic_cast<CGraphicsItem *>(item);
//        if (pBzItem != nullptr && pBzItem->isBzItem()) {
//            CGraphicsItem *pNew = pBzItem->creatSameItem();
//            if (pNew != nullptr) {
//                m_itemList.append(pNew);
//            }
//        }
//    }
//}

CShapeMimeData::CShapeMimeData(const CGroupBzItemsTreeInfo &itemsTreeInfo):
    QMimeData(), treeInfo(itemsTreeInfo)
{
}
CShapeMimeData::~CShapeMimeData()
{
    PageScene::releaseBzItemsTreeInfo(treeInfo);
}

bool CShapeMimeData::hasFormat(const QString &mimetype) const
{
    if (mimetype == "drawItems") {
        return true;
    }
    return false;
}

//QList<CGraphicsItem *> CShapeMimeData::itemList() const
//{
//    return m_itemList;
//}

//QList<CGraphicsItem *> CShapeMimeData::creatCopyItems() const
//{
//    QList<CGraphicsItem *> retList;
//    foreach (QGraphicsItem *item, m_itemList) {
//        CGraphicsItem *pBzItem = dynamic_cast<CGraphicsItem *>(item);
//        if (pBzItem != nullptr && pBzItem->isBzItem()) {
//            CGraphicsItem *pNew = pBzItem->creatSameItem();
//            if (pNew != nullptr) {
//                retList.append(pNew);
//            }
//        }
//    }
//    return retList;
//}

CGroupBzItemsTreeInfo CShapeMimeData::itemsTreeInfo()
{
    return treeInfo;
}
