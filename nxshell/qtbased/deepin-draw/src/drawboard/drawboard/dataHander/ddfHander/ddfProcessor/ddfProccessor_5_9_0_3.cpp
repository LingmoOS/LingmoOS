// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfProccessor_5_9_0_3.h"

#include "pagescene.h"
#include "pagecontext.h"


#include <QStack>

DdfUnitProccessor_5_9_0_3::DdfUnitProccessor_5_9_0_3(QObject *parent): DdfUnitProccessor_5_9_0_1(parent)
{
    supTypes.insert(GroupItemType);
}

int DdfUnitProccessor_5_9_0_3::version()
{
    return EDdf5_9_0_3_LATER;
}

PageContextData DdfUnitProccessor_5_9_0_3::pageContextToData(PageContext *context)
{
    PageContext *pContext = context;

    PageContextData data;
    data.units_comp = pContext->scene()->getGroupTreeInfo(/*nullptr, ESaveToDDf*/);

    int count = 0;
    QStack<UnitTree_Comp> stacks;
    stacks.append(data.units_comp);
    while (!stacks.isEmpty()) {
        UnitTree_Comp tree = stacks.pop();
        if (isSupportedSaveType(tree.treeUnit.head.dataType)) {
            ++count;
        }
        for (int i = 0; i < tree.childTrees.count(); ++i) {
            auto childTree = tree.childTrees.at(i);
            stacks.push(childTree);
        }
        foreach (auto it, tree.bzItems) {
            if (isSupportedSaveType(it.head.dataType))
                ++count;
        }
    }
    data.head.version = version();
    data.head.unitCount = count;
    data.head.rect = pContext->pageRect();

    return data;
}

void DdfUnitProccessor_5_9_0_3::serializationUnitTree(QDataStream &out, const UnitTree_Comp &tree, int count, DdfHander *hander)
{
    int progress = 0;
    serializationUnitTree_Loop(out, tree, count, progress, hander);
}

UnitTree_Comp DdfUnitProccessor_5_9_0_3::deserializationUnitTree(QDataStream &in, int count, bool &stop, QString &stopReason, DdfHander *hander)
{
    int progress = 0;
    return deserializationUnitTree_Loop(in, count, progress, stop, stopReason, hander);
}

void DdfUnitProccessor_5_9_0_3::serializationUnitHead(QDataStream &out, const UnitHead &head, DdfHander *hander)
{
    DdfUnitProccessor_5_9_0_1::serializationUnitHead(out, head, hander);
    out << head.rect;
}


UnitHead DdfUnitProccessor_5_9_0_3::deserializationUnitHead(QDataStream &in, DdfHander *hander)
{
    UnitHead head = DdfUnitProccessor_5_9_0_1::deserializationUnitHead(in, hander);
    in >> head.rect;
    return head;
}

void DdfUnitProccessor_5_9_0_3::serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander)
{
    if (type == GroupItemType) {
        GroupUnitData i = item.data.value<GroupUnitData>();
        out << i.isCancelAble;
        if (!i.name.isEmpty()) {
            QByteArray arry = i.name.toUtf8();
            int count = arry.size();
            char *p = arry.data();
            out << count;
            out.writeRawData(p, count);
        } else {
            out << int(0);
        }
        out << i.groupType;

        return;
    }
    DdfUnitProccessor_5_9_0_1::serializationUnitData(out, type, item, hander);
}

UnitData DdfUnitProccessor_5_9_0_3::deserializationUnitData(QDataStream &in, int type, DdfHander *hander)
{
    if (type == GroupItemType) {
        UnitData item;
        GroupUnitData i;
        in >> i.isCancelAble;
        int count = 0;
        in >> count;
        if (count > 0) {
            in >> i.nameByteSz;
            if (i.nameByteSz > 0) {
                char *nameTemp = new char[unsigned(i.nameByteSz)];
                in.readRawData(nameTemp, int(i.nameByteSz));
                i.name = QString::fromUtf8(nameTemp, i.nameByteSz);
            }
        }
        in >> i.groupType;
        item.data.setValue<GroupUnitData>(i);
        return item;
    }
    return DdfUnitProccessor_5_9_0_1::deserializationUnitData(in, type, hander);
}

void DdfUnitProccessor_5_9_0_3::serializationUnitTree_Loop(QDataStream &out, const UnitTree_Comp &tree, int count, int &currentProgress, DdfHander *hander)
{
    qWarning() << "tree item count = " << tree.bzItems.count() << "group count = " << tree.childTrees.count();
    out << tree.childTrees.count();

    for (int i = 0; i < tree.childTrees.count(); ++i) {
        UnitTree_Comp childTree = tree.childTrees.at(i);
        serializationUnitTree_Loop(out, childTree, count, currentProgress, hander);
    }

    out << tree.bzItems.count();
    foreach (auto unit, tree.bzItems) {
        //emit serializationProgress(++currentProgress, count, "save");
        emit hander->progressChanged(++currentProgress, count, "");
        if (isSupportedSaveType(unit.head.dataType)) {
            serializationUnit(out, unit, hander);
        }
    }
    //emit serializationProgress(++currentProgress, count, "save");
    emit hander->progressChanged(++currentProgress, count, "");
    serializationUnit(out, tree.treeUnit, hander);
}

UnitTree_Comp DdfUnitProccessor_5_9_0_3::deserializationUnitTree_Loop(QDataStream &in, int count, int &currentProgress, bool &stop, QString &stopReason, DdfHander *hander)
{
    UnitTree_Comp tree;
    int childTreeCount = 0;
    in >> childTreeCount;
    for (int i = 0; i < childTreeCount; ++i) {
        tree.childTrees.append(deserializationUnitTree_Loop(in, count, currentProgress, stop, stopReason, hander));
    }

    int childItemCount = 0;
    in >> childItemCount;
    for (int i = 0; i < childItemCount; ++i) {
        emit hander->progressChanged(++currentProgress, count, "");
        Unit unit = deserializationUnit(in, hander);
        if (isSupportedSaveType(unit.head.dataType)) {
            tree.bzItems.append(unit);
        }
    }

    emit hander->progressChanged(++currentProgress, count, "");
    tree.treeUnit = deserializationUnit(in, hander);

    return tree;
}

//bool DdfUnitProccessor_5_9_0_3::isSupportedSaveType(int type, DdfHander *hander)
//{
//    bool b = DdfUnitProccessor_5_9_0_1::isSupportedSaveType(type, hander);
//    if (!b) {
//        return type == MgrType;
//    }
//    return b;
//}

UnitTree_Comp DdfUnitProccessor_5_9_0_3_Compatibel::deserializationUnitTree_Loop(QDataStream &in, int count, int &currentProgress, bool &stop, QString &stopReason, DdfHander *hander)
{
    UnitTree_Comp tree;
    int childTreeCount = 0;
    in >> childTreeCount;
    for (int i = 0; i < childTreeCount; ++i) {
        tree.childTrees.append(deserializationUnitTree_Loop(in, count, currentProgress, stop, stopReason, hander));
        if (stop) {
            return tree;
        }
    }

    int childItemCount = 0;
    in >> childItemCount;
    for (int i = 0; i < childItemCount; ++i) {
        emit hander->progressChanged(++currentProgress, count, "");
        Unit unit = deserializationUnit(in, hander);
        if (isSupportedSaveType(unit.head.dataType)) {
            if (unit.head.dataType == PenType) {
                if (_handerMeetPen.contains(hander)) {
                    _handerMeetPen.remove(hander);
                    int ret = waitForHowToHanderOldPenUnit(hander);
                    if (ret != 0) {
                        stop = true;
                        stopReason = "because refuse old pen unit.";
                        hander->setError(DdfHander::EUserCancelLoad_OldPen, stopReason);
                        return tree;
                    }
                }
            }
            tree.bzItems.append(unit);
        }
    }

    emit hander->progressChanged(++currentProgress, count, "");
    tree.treeUnit = deserializationUnit(in, hander);

    return tree;
}
