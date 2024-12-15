// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddfproccessordrawboard.h"
#include "pagecontext.h"
#include "pagescene.h"
#include "layeritem.h"

#include <QFile>

DdfProccessorDrawBoard::DdfProccessorDrawBoard(QObject *parent): DdfUnitProccessor_5_10_0(parent)
{
    supTypes.insert(LayerItemType);
}

int DdfProccessorDrawBoard::version()
{
    return EDdf_DrawBorad;
}

bool DdfProccessorDrawBoard::saveTo(const PageContextData &data, const QString &file, DdfHander *hander)
{
    //return DdfUnitProccessor_5_10_0::saveTo(data, file, hander);
    int needBytesCount = calTotalBytes(data, hander);

    if (!isVolumeSpaceAvailabel(file, needBytesCount)) {
        hander->setError(DataHander::EInsufficientPartitionSpace, spaceNotEnoughMessage());
    } else {
        QFile writeFile(file);
        if (writeFile.open(QIODevice::WriteOnly)) {

            QDataStream out(&writeFile);

            serializationDdfHead(out, data.head, hander);
            out << data.units.count();
            serializationUnits(out, data.units, hander);

            writeFile.close();
        } else {
            hander->setError(writeFile.error(), writeFile.errorString());
        }
    }

    saveMd5ToFile(file);

    return hander->error() == 0;
}

PageContextData DdfProccessorDrawBoard::loadFrom(const QString &file, DdfHander *hander)
{
    //return DdfUnitProccessor_5_10_0::loadFrom(file, hander);
    if (checkMd5BeforeLoad(file)) {
        //int ret = 0;
        QString resultInfo;
        PageContextData data;
        QFile readFile(file);
        if (readFile.open(QIODevice::ReadOnly)) {
            QDataStream in(&readFile);
            data.head      = deserializationDdfHead(in, hander);
            bool stop      = false;
            int count = 0;
            in >> count;
            data.units = deserializationUnits(in, count, stop, resultInfo, hander);
            readFile.close();
            if (!stop) {
            } else {
                //ret = 1;
            }
            return data;
        }
    }
    messageFileBroken(file, hander);
    return PageContextData();
}

PageContextData DdfProccessorDrawBoard::pageContextToData(PageContext *context)
{
    PageContext *pContext = context;

    PageContextData data;
    auto layers = pContext->scene()->topLayers();
    foreach (auto ly, layers) {
        data.units.append(ly->getItemUnit(UnitUsage_Save));
    }
//    int count = 0;
//    foreach (auto it, data.units) {
//        if (isSupportedSaveType(it.head.dataType))
//            ++count;
//    }
    int count = std::count_if(data.units.begin(), data.units.end(), [ = ](const Unit & it) {return isSupportedSaveType(it.head.dataType);});

    data.head.version = version();
    data.head.unitCount = count;
    data.head.rect = pContext->pageRect();

    return data;
}

void DdfProccessorDrawBoard::pageContextLoadData(PageContext *pContext, const PageContextData &data)
{
    pContext->loadUnits(data.units, true);
}

qint64 DdfProccessorDrawBoard::calTotalBytes(const PageContextData &data, DdfHander *hander)
{
    QSignalBlocker blocker(hander);

    QByteArray allBytes;

    QDataStream out(&allBytes, QIODevice::WriteOnly);

    serializationDdfHead(out, data.head, hander);

    serializationUnits(out, data.units, hander);

    return allBytes.size();
}

void DdfProccessorDrawBoard::serializationUnits(QDataStream &out, const QList<Unit> &units,
                                                DdfHander *hander)
{
    foreach (auto ut, units) {
        auto ty = ut.head.dataType;
        if (isSupportedSaveType(ty)) {
            out << ut.chidren.count();
            serializationUnits(out, ut.chidren, hander);
            serializationUnit(out, ut, hander);
        }
    }
}

QList<Unit> DdfProccessorDrawBoard::deserializationUnits(QDataStream &in, int count, bool &stop,
                                                         QString &stopReason, DdfHander *hander)
{
    QList<Unit> units;
    for (int i = 0; i < count; ++i) {
        emit hander->progressChanged(i, count, "");

        int childCount = 0;
        in >> childCount;
        auto children = deserializationUnits(in, childCount, stop, stopReason, hander);

        Unit ut = deserializationUnit(in, hander);
        ut.chidren = children;
        if (isSupportedSaveType(ut.head.dataType)) {
            units.append(ut);
        }
    }
    return units;
}
Unit DdfProccessorDrawBoard::deserializationUnit(QDataStream &in, DdfHander *hander)
{
    //使用默认的解析三步
    Unit unit(UnitUsage_Save);

    unit.head = deserializationUnitHead(in, hander);

    unit.data = deserializationUnitData(in, unit.head.dataType, hander);

    unit.tail = deserializationUnitTail(in, hander);

    return unit;
}


void DdfProccessorDrawBoard::serializationUnitData(QDataStream &out, int type, const UnitData &item, DdfHander *hander)
{
    if (type == LayerItemType) {
        LayerUnitData i = item.data.value<LayerUnitData>();
        UnitData rasterTemp;
        rasterTemp.setValue<RasterUnitData>(i.rasterData);
        DdfProccessorDrawBoard::serializationUnitData(out, RasterItemType, rasterTemp, hander);
        return;
    } else if (type == RasterItemType) {
        RasterUnitData i = item.data.value<RasterUnitData>();
        out << i.baseImg;
        out << i.baseRect;
        out << i.blocked;
        out << i.layerType;
        out << i.commands.count();
        foreach (auto cmd, i.commands) {
            out << cmd->cmdType();
            cmd->serialization(out);
        }
        return;
    } else if (type == RectType) {
        RectUnitData i = item.data.value<RectUnitData>();
        out << i.topLeft;
        out << i.bottomRight;
        out << i.xRedius;
        out << i.yRedius;
        out << i.bSameRadius;
        out << i.leftRadius;
        out << i.rightRadius;
        out << i.leftBottomRadius;
        out << i.rightBottomRadius;
        return;
    }
    DdfUnitProccessor_5_10_0::serializationUnitData(out, type, item, hander);
}

UnitData DdfProccessorDrawBoard::deserializationUnitData(QDataStream &in, int type, DdfHander *hander)
{
    if (type == LayerItemType) {
        LayerUnitData data;
        data.rasterData = DdfProccessorDrawBoard::deserializationUnitData(in, RasterItemType, hander).data.value<RasterUnitData>();
        UnitData result;
        result.data.setValue(data);
        return result;
    } else if (type == RasterItemType) {
        RasterUnitData iData;
        in >> iData.baseImg;
        in >> iData.baseRect;
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
    } else if (type == RectType) {
        RectUnitData rData;
        in >> rData.topLeft;
        in >> rData.bottomRight;
        in >> rData.xRedius;
        in >> rData.yRedius;
        in >> rData.bSameRadius;
        in >> rData.leftRadius;
        in >> rData.rightRadius;
        in >> rData.leftBottomRadius;
        in >> rData.rightBottomRadius;
        UnitData rItem;
        rItem.data.setValue<RectUnitData>(rData);
        return rItem;
    }
    return DdfUnitProccessor_5_10_0::deserializationUnitData(in, type, hander);
}
