// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfproccessor_5_8_0_10_1.h"

DdfUnitProccessor_5_8_0_10_1::DdfUnitProccessor_5_8_0_10_1(QObject *parent):
    DdfUnitProccessor_5_8_0_9(parent)
{

}

int DdfUnitProccessor_5_8_0_10_1::version()
{
    return EDdf5_8_0_10_1;
}

void DdfUnitProccessor_5_8_0_10_1::serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander)
{
    if (LineType == type) {
        LineUnitData i = item.data.value<LineUnitData>();
        out << i.point1;
        out << i.point2;
        out << qint32(i.start_type);
        out << qint32(i.end_type);
        return;
    }
    DdfUnitProccessor_5_8_0_9::serializationUnitData(out, type, item, hander);
}

UnitData DdfUnitProccessor_5_8_0_10_1::deserializationUnitData(QDataStream &in, int type, DdfHander *hander)
{
    if (LineType == type) {
        LineUnitData i;
        in >> i.point1;
        in >> i.point2;
        qint32 startTp, endTp;
        in >> startTp;
        in >> endTp;
        i.start_type = ELinePosStyle(startTp);
        i.end_type   = ELinePosStyle(endTp);
        UnitData item;
        item.data.setValue<LineUnitData>(i);
        return item;
    }
    return DdfUnitProccessor_5_8_0_9::deserializationUnitData(in, type, hander);
}
