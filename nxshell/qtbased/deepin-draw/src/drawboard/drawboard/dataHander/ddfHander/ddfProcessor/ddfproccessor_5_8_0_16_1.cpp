// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfproccessor_5_8_0_16_1.h"

DdfUnitProccessor_5_8_0_16_1::DdfUnitProccessor_5_8_0_16_1(QObject *parent): DdfUnitProccessor_5_8_0_10_2(parent)
{

}

int DdfUnitProccessor_5_8_0_16_1::version()
{
    return EDdf5_8_0_16_1;
}

void DdfUnitProccessor_5_8_0_16_1::serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander)
{
    if (type == PictureType) {
        ImageUnitData_Comp i = item.data.value<ImageUnitData_Comp>();
        UnitData rectUnit;
        rectUnit.data.setValue<RectUnitData>(i.rect);
        serializationUnitData(out, RectType, rectUnit, hander);
        //out << i.rect;
        out << i.srcByteArry;
        return ;
    }
    DdfUnitProccessor_5_8_0_10_2::serializationUnitData(out, type, item, hander);
}

UnitData DdfUnitProccessor_5_8_0_16_1::deserializationUnitData(QDataStream &in, int type, DdfHander *hander)
{
    if (type == PictureType) {
        ImageUnitData_Comp i;
        auto rect = deserializationUnitData(in, RectType, hander);
        i.rect = rect.data.value<RectUnitData>();
        //in >> i.rect;
        QByteArray arryData;
        in >> arryData;
        i.srcByteArry = arryData;
        i.image = QImage::fromData(arryData);
        UnitData item;
        item.data.setValue<ImageUnitData_Comp>(i);
        return item;
    }
    return DdfUnitProccessor_5_8_0_10_2::deserializationUnitData(in, type, hander);
}

