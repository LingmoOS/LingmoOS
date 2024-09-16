// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfproccessor_5_8_0_9.h"
#include "pagescene.h"

DdfUnitProccessor_5_8_0_9::DdfUnitProccessor_5_8_0_9(QObject *parent):
    DdfUnitProccessor_5_8_0_Compatibel(parent)
{

}

int DdfUnitProccessor_5_8_0_9::version()
{
    return EDdf5_8_0_9_1;
}

void DdfUnitProccessor_5_8_0_9::serializationDdfHead(QDataStream &out, const PageContextHead &ddfHead, DdfHander *hander)
{
    out << ddfHead.ddfFlag;
    out << ddfHead.version;
    out << ddfHead.unitCount;
    out << ddfHead.rect;
}

PageContextHead DdfUnitProccessor_5_8_0_9::deserializationDdfHead(QDataStream &in, DdfHander *hander)
{
    PageContextHead head;
    in >> head.ddfFlag;
    in >> head.version;
    in >> head.unitCount;
    in >> head.rect;
    return head;
}

void DdfUnitProccessor_5_8_0_9::serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander)
{
    if (type == RectType) {
        RectUnitData i = item.data.value<RectUnitData>();
        out << i.topLeft;
        out << i.bottomRight;
        out << i.xRedius;
        out << i.yRedius;
        return;
    }
    DdfUnitProccessor_5_8_0_Compatibel::serializationUnitData(out, type, item, hander);
}

UnitData DdfUnitProccessor_5_8_0_9::deserializationUnitData(QDataStream &in, int type, DdfHander *hander)
{
    if (type == RectType) {
        UnitData item;
        RectUnitData i;
        in >> i.topLeft;
        in >> i.bottomRight;
        in >> i.xRedius;
        in >> i.yRedius;
        item.data.setValue<RectUnitData>(i);
        return item;
    }
    return DdfUnitProccessor_5_8_0_Compatibel::deserializationUnitData(in, type, hander);
}
