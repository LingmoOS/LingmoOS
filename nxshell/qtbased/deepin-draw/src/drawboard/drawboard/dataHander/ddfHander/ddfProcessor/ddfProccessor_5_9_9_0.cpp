// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfProccessor_5_9_9_0.h"

#include "pagescene.h"
#include "pagecontext.h"

void compatibelRasterUnitData(UnitHead &head, RasterUnitData &iData)
{
    RasterItem *rasterItem = new RasterItem;

    rasterItem->setImage(iData.baseImg);
    foreach (auto cmd, iData.commands) {
        cmd.data()->doCommandFor(rasterItem);
    }
    //iData.image = rasterItem->image();
    head.pos = rasterItem->pos();
    head.rect = rasterItem->itemRect();
    head.trans = rasterItem->transform();
    head.rotate = rasterItem->drawRotation();
    head.zValue = rasterItem->zValue();
}

DdfUnitProccessor_5_9_9_0::DdfUnitProccessor_5_9_9_0(QObject *parent): DdfUnitProccessor_5_9_0_3_Compatibel(parent)
{
    supTypes.insert(RasterItemType);
    supTypes.remove(PenType);
}

int DdfUnitProccessor_5_9_9_0::version()
{
    return EDdf5_9_9_0_LATER;
}

Unit DdfUnitProccessor_5_9_9_0::deserializationUnit(QDataStream &in, DdfHander *hander)
{
    Unit unit = DdfUnitProccessor_5_9_0_3_Compatibel::deserializationUnit(in, hander);

    if (unit.head.dataType == RasterItemType) {
        RasterUnitData i = unit.data.data.value<RasterUnitData>();
        //非手绘的图层（图像图层）
        if (!i.baseImg.isNull()) {
            RasterGeomeCmd *com = new RasterGeomeCmd(unit.head.pos.toPoint(), 0, unit.head.zValue, unit.head.rect, unit.head.trans);
            i.commands.push_front(QSharedPointer<RasterCommandBase>(com));
            unit.data.data.setValue<RasterUnitData>(i);
        }
        //compatibelRasterUnitData(unit.head, i);
        unit.setValue<RasterUnitData>(i);
    }
    return unit;
}

void DdfUnitProccessor_5_9_9_0::serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander)
{
    switch (type) {
    case RasterItemType: {
        RasterUnitData i = item.data.value<RasterUnitData>();
        out << i.baseImg;
        out << i.blocked;
        out << i.commands.count();
        foreach (auto cmd, i.commands) {
            out << cmd->cmdType();
            cmd->serialization(out);
        }
        break;
    }
    default: {
        DdfUnitProccessor_5_9_0_3_Compatibel::serializationUnitData(out, type, item, hander);
        break;
    }
    }
}

UnitData DdfUnitProccessor_5_9_9_0::deserializationUnitData(QDataStream &in, int type, DdfHander *hander)
{
    if (type == RasterItemType) {
        RasterUnitData iData;
        in >> iData.baseImg;
        in >> iData.blocked;
        int count = 0;
        in >> count;
        for (int i = 0; i < count; ++i) {
            int tp = 0;
            in >> tp;
            auto cmd = RasterCommandBase::creatCmd(tp);
            if (cmd != nullptr) {
                cmd->deserialization(in);
                QSharedPointer<RasterCommandBase> p(cmd);
                iData.commands.append(p);
            }
        }

        //0 mean is pen draw,1 mean is one imag;

        iData.layerType = iData.baseImg.isNull() ? 0 : 1;
        UnitData item;
        item.data.setValue<RasterUnitData>(iData);
        return item;
    }
    return DdfUnitProccessor_5_9_0_3_Compatibel::deserializationUnitData(in, type, hander);
}

DdfUnitProccessor_5_10_0::DdfUnitProccessor_5_10_0(QObject *parent): /*DdfUnitProccessor_5_9_0_3_Compatibel*/DdfUnitProccessor_5_9_9_0(parent)
{
    supTypes.insert(RasterItemType);
    supTypes.remove(PenType);
}

int DdfUnitProccessor_5_10_0::version()
{
    return EDdf5_10_2_7_LATER;
}

Unit DdfUnitProccessor_5_10_0::deserializationUnit(QDataStream &in, DdfHander *hander)
{
    //这儿就是要调用DdfUnitProccessor_5_9_0_3_Compatibel的函数，绕开DdfUnitProccessor_5_9_9_0中对RasterItemType的处理
    Unit unit = DdfUnitProccessor_5_9_0_3_Compatibel::deserializationUnit(in, hander);
    if (unit.head.dataType == RasterItemType) {
        RasterUnitData i = unit.value<RasterUnitData>();
        compatibelRasterUnitData(unit.head, i);
        unit.setValue<RasterUnitData>(i);
    }
    return unit;
}

void DdfUnitProccessor_5_10_0::serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander)
{
    switch (type) {
    case RasterItemType: {
        RasterUnitData i = item.data.value<RasterUnitData>();
        out << i.baseImg;
        out << i.blocked;
        out << i.layerType;
        out << i.commands.count();
        foreach (auto cmd, i.commands) {
            out << cmd->cmdType();
            cmd->serialization(out);
        }
        break;
    }
    default: {
        DdfUnitProccessor_5_9_9_0::serializationUnitData(out, type, item, hander);
        break;
    }
    }
}

UnitData DdfUnitProccessor_5_10_0::deserializationUnitData(QDataStream &in, int type, DdfHander *hander)
{
    if (type == RasterItemType) {
        RasterUnitData iData;
        in >> iData.baseImg;
        in >> iData.blocked;
        in >> iData.layerType;

        int count = 0;
        in >> count;
        for (int i = 0; i < count; ++i) {
            int tp = 0;
            in >> tp;
            auto cmd = RasterCommandBase::creatCmd(tp);
            if (cmd != nullptr) {
                cmd->deserialization(in);
                QSharedPointer<RasterCommandBase> p(cmd);
                iData.commands.append(p);
            }
        }
        UnitData item;
        item.data.setValue<RasterUnitData>(iData);
        return item;
    }
    return DdfUnitProccessor_5_9_9_0::deserializationUnitData(in, type, hander);
}
