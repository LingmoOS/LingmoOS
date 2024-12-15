// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfproccessor_5_8_0_84.h"

#include "pagescene.h"

DdfUnitProccessor_5_8_0_84::DdfUnitProccessor_5_8_0_84(QObject *parent): DdfUnitProccessor_5_8_0_48(parent)
{
    supTypes.remove(BlurType);
}

int DdfUnitProccessor_5_8_0_84::version()
{
    return EDdf5_8_0_84_LATER;
}

void DdfUnitProccessor_5_8_0_84::serializationUnitHead(QDataStream &out, const UnitHead &head, DdfHander *hander)
{
    DdfUnitProccessor_5_8_0_48::serializationUnitHead(out, head, hander);
    out << head.blurInfos.count();
    for (int i = 0; i < head.blurInfos.count(); ++i) {
        SBlurInfo blurInfo = head.blurInfos.at(i);
        out << blurInfo.blurPath;
        out << int(blurInfo.blurEfTp);
    }
}

UnitHead DdfUnitProccessor_5_8_0_84::deserializationUnitHead(QDataStream &in, DdfHander *hander)
{
    UnitHead head = DdfUnitProccessor_5_8_0_48::deserializationUnitHead(in, hander);
    in >> head.blurCount;
    for (int i = 0; i < head.blurCount; ++i) {
        SBlurInfo blurInfo;
        in >> blurInfo.blurPath;
        int bluref = UnknowEffect;
        in >> bluref;
        blurInfo.blurEfTp = EBlurEffect(bluref);

        head.blurInfos.append(blurInfo);
    }
    return head;
}

//bool DdfUnitProccessor_5_8_0_84::isSupportedSaveType(int type, DdfHander *hander)
//{
//    bool b = DdfUnitProccessor_5_8_0_48::isSupportedSaveType(type, hander);
//    if (b) {
//        return type != BlurType;
//    }
//    return false;
//}

