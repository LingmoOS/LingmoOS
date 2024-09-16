// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfproccessor_5_9_0_1.h"

DdfUnitProccessor_5_9_0_1::DdfUnitProccessor_5_9_0_1(QObject *parent): DdfUnitProccessor_5_8_0_84(parent)
{

}

int DdfUnitProccessor_5_9_0_1::version()
{
    return EDdf5_9_0_1_LATER;
}

void DdfUnitProccessor_5_9_0_1::serializationUnitHead(QDataStream &out, const UnitHead &head, DdfHander *hander)
{
    DdfUnitProccessor_5_8_0_84::serializationUnitHead(out, head, hander);
    out << head.trans;
}

UnitHead DdfUnitProccessor_5_9_0_1::deserializationUnitHead(QDataStream &in, DdfHander *hander)
{
    UnitHead head = DdfUnitProccessor_5_8_0_84::deserializationUnitHead(in, hander);
    in >> head.trans;
    return head;
}
