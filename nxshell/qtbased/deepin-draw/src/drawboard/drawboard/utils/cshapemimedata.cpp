// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cshapemimedata.h"

//CShapeMimeData::CShapeMimeData(const UnitTree_Comp &itemsTreeInfo):
//    QMimeData(), treeInfo(itemsTreeInfo)
//{
//}
DrawBoardMimeData::DrawBoardMimeData(const UnitList &u): QMimeData(), ut(u)
{

}

DrawBoardMimeData::~DrawBoardMimeData()
{

}

bool DrawBoardMimeData::hasFormat(const QString &mimetype) const
{
    if (mimetype == "pageItems") {
        return true;
    }
    return false;
}

UnitList DrawBoardMimeData::unit() const
{
    return ut;
}
