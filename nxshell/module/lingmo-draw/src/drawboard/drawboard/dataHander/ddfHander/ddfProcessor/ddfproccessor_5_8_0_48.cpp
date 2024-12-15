// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfproccessor_5_8_0_48.h"
DdfUnitProccessor_5_8_0_48::DdfUnitProccessor_5_8_0_48(QObject *parent): DdfUnitProccessor_5_8_0_20(parent)
{

}

int DdfUnitProccessor_5_8_0_48::version()
{
    return EDdf5_8_0_48_LATER;
}

void DdfUnitProccessor_5_8_0_48::serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander)
{
    if (type == PictureType) {
        ImageUnitData_Comp i = item.data.value<ImageUnitData_Comp>();
        UnitData rectTemp;
        rectTemp.data.setValue<RectUnitData>(i.rect);
        serializationUnitData(out, RectType, rectTemp, hander);
        //out << i.rect;
        out << i.srcByteArry;
        out << i.flipHorizontal;
        out << i.flipVertical;
        return ;
    }
    DdfUnitProccessor_5_8_0_20::serializationUnitData(out, type, item, hander);
}

UnitData DdfUnitProccessor_5_8_0_48::deserializationUnitData(QDataStream &in, int type, DdfHander *hander)
{
    if (type == PictureType) {
        ImageUnitData_Comp i;
        auto rect = deserializationUnitData(in, RectType, hander);
        i.rect = rect.data.value<RectUnitData>();
//        in >> i.rect;
        QByteArray arryData;
        in >> arryData;
        i.srcByteArry = arryData;
        i.image = QImage::fromData(arryData);
        in >> i.flipHorizontal;
        in >> i.flipVertical;
        UnitData item;
        item.data.setValue<ImageUnitData_Comp>(i);
        return item;
    }
    return DdfUnitProccessor_5_8_0_20::deserializationUnitData(in, type, hander);
}

