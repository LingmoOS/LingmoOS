// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfproccessor_5_8_0_10_2.h"

DdfUnitProccessor_5_8_0_10_2::DdfUnitProccessor_5_8_0_10_2(QObject *parent): DdfUnitProccessor_5_8_0_10_1(parent)
{

}

int DdfUnitProccessor_5_8_0_10_2::version()
{
    return EDdf5_8_0_10_2;
}

void DdfUnitProccessor_5_8_0_10_2::serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander)
{
    if (type == PenType) {
        PenUnitData_Comp i = item.data.value<PenUnitData_Comp>();
        out << qint32(i.start_type);
        out << i.path;
        out << qint32(i.end_type);
        return;
    }
    DdfUnitProccessor_5_8_0_10_1::serializationUnitData(out, type, item, hander);
}

UnitData DdfUnitProccessor_5_8_0_10_2::deserializationUnitData(QDataStream &in, int type, DdfHander *hander)
{
    if (type == PenType) {
        PenUnitData_Comp i;
        qint32 startTp, endTp;
        QPainterPath pathTemp;
        in >> startTp;
        in >> pathTemp;
        in >> endTp;
        i.start_type = ELinePosStyle(startTp);
        i.path       = pathTemp;
        i.end_type   = ELinePosStyle(endTp);

        UnitData item;
        item.data.setValue<PenUnitData_Comp>(i);
        return item;
    }
    return DdfUnitProccessor_5_8_0_10_1::deserializationUnitData(in, type, hander);
}
