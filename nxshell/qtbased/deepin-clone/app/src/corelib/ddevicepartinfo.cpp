// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "ddevicepartinfo.h"
#include "dpartinfo_p.h"
#include "helper.h"
#include "ddevicediskinfo.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QDebug>

class DDevicePartInfoPrivate : public DPartInfoPrivate
{
public:
    DDevicePartInfoPrivate(DDevicePartInfo *qq);

    void refresh() Q_DECL_OVERRIDE;
    qint64 getUsedSize() const Q_DECL_OVERRIDE;
    qint64 getFreeSize() const Q_DECL_OVERRIDE;
    bool getIsDeepinSystemRoot() const Q_DECL_OVERRIDE;
    void ensureSizeInfo();

    void init(const QJsonObject &obj);

    bool sizeInfoInitialized = false;
    bool isDeepinSystemRootInitialized = false;
};

DDevicePartInfoPrivate::DDevicePartInfoPrivate(DDevicePartInfo *qq)
    : DPartInfoPrivate(qq)
{

}

void DDevicePartInfoPrivate::refresh()
{
    *q = DDevicePartInfo(name);
}

qint64 DDevicePartInfoPrivate::getUsedSize() const
{
    const_cast<DDevicePartInfoPrivate*>(this)->ensureSizeInfo();

    return usedSize;
}

qint64 DDevicePartInfoPrivate::getFreeSize() const
{
    const_cast<DDevicePartInfoPrivate*>(this)->ensureSizeInfo();

    return freeSize;
}

bool DDevicePartInfoPrivate::getIsDeepinSystemRoot() const
{
    if (!isDeepinSystemRootInitialized) {
        const_cast<DDevicePartInfoPrivate*>(this)->isDeepinSystemRoot = Helper::isDeepinSystem(*q);
        const_cast<DDevicePartInfoPrivate*>(this)->isDeepinSystemRootInitialized = true;
    }

    return isDeepinSystemRoot;
}

void DDevicePartInfoPrivate::ensureSizeInfo()
{
    if (sizeInfoInitialized)
        return;

    if (fsType == DPartInfo::Invalid || fsType == DPartInfo::UnknowFS) {
        usedSize = size;
        freeSize = 0;
    } else if (!ThreadUtil::runInNewThread(&Helper::getPartitionSizeInfo, filePath, &usedSize, &freeSize, &blockSize)) {
        dCError("Get partition used sieze/free size info failed, device: %s", qPrintable(name));
    }

    sizeInfoInitialized = true;
}

void DDevicePartInfoPrivate::init(const QJsonObject &obj)
{
    name = obj.value("name").toString();
    filePath = name;
    kname = obj.value("kname").toString();
    parentDiskFilePath = obj.value("pkname").toString();
    size = Helper::getIntValue(obj.value("size"));
    fsTypeName = obj.value("fstype").toString();
    fsType = toType(fsTypeName);
    mountPoint = obj.value("mountpoint").toString();
    uuid = obj.value("uuid").toString();
    label = obj.value("label").toString();
    partLabel = obj.value("partlabel").toString();
    partTypeName = obj.value("parttype").toString();
    partType = DPartInfo::type(partTypeName);
    guidType = DPartInfo::guidType(partTypeName.toLatin1().toUpper());
    blockSize = obj.value("phy-sec").toInt(4096);
    readonly = Helper::getBoolValue(obj.value("ro")) || obj.value("type").toString() == "rom";
    removeable = Helper::getBoolValue(obj.value("rm"));
    transport = obj.value("tran").toString();
    partUUID = obj.value("partuuid").toString();

    if (obj.value("pkname").isNull()) {
        sizeStart = 0;
        sizeEnd = size - 1;
        index = 0;
    } else {
        int code = Helper::processExec("partx", {name, "-b", "-P", "-o", "START,END,SECTORS,SIZE,TYPE,NR,UUID"});

        if (code == 0) {
            const QByteArray &data = Helper::lastProcessStandardOutput();
            const QByteArrayList &list = data.split(' ');

            if (list.count() != 7) {
                dCError("Get partition START/END/SECTORS/SIZE/TYPE/NR/UUID info error by partx, device: %s", qPrintable(name));

                return;
            }

            qint64 start = list.first().split('"').at(1).toLongLong();
//            qint64 end = list.at(1).split('"').at(1).toLongLong();
            qint64 sectors = list.at(2).split('"').at(1).toLongLong();
            qint64 size = list.at(3).split('"').at(1).toLongLong();

            if (partTypeName.isEmpty()) {
                partTypeName = list.at(4).split('"').at(1);
                partType = DPartInfo::type(partTypeName);
            }

            index = list.at(5).split('"').at(1).toInt();

            if (partUUID.isEmpty()) {
                partUUID = list.at(6).split('"').at(1);
            }

            Q_ASSERT(sectors > 0);

            sizeStart = start * (size / sectors);
            sizeEnd = sizeStart + this->size - 1;
        } else {
            int number_start = 0;

            for (int i = name.size() - 1; i >= 0; --i) {
                if (!name.at(i).isDigit()) {
                    number_start = i + 1;
                    break;
                }
            }

            bool ok = false;

            index = name.mid(number_start).toInt(&ok);

            if (!ok)
                index = -1;
        }
    }
}

DDevicePartInfo::DDevicePartInfo()
    : DPartInfo(new DDevicePartInfoPrivate(this))
{

}

DDevicePartInfo::DDevicePartInfo(const QString &name)
    : DPartInfo(new DDevicePartInfoPrivate(this))
{
    const QJsonArray &block_devices = Helper::getBlockDevices({name});

    if (!block_devices.isEmpty()) {
        const QJsonObject &obj = block_devices.first().toObject();

        d_func()->init(obj);

        d->transport = DDeviceDiskInfo(d_func()->parentDiskFilePath).transport();
    }
}

QList<DDevicePartInfo> DDevicePartInfo::localePartList()
{
    const QJsonArray &block_devices = Helper::getBlockDevices();

    QList<DDevicePartInfo> list;

    for (const QJsonValue &value : block_devices) {
        const QJsonObject &obj = value.toObject();

        if (Global::disableLoopDevice && obj.value("type").toString() == "loop")
            continue;

        const QString &fstype = obj.value("fstype").toString();
        const QString &transport = obj.value("tran").toString();

        if (fstype.isEmpty()) {
            QStringList children_uuids;

            for (const QJsonValue &children : obj.value("children").toArray()) {
                const QJsonObject &obj = children.toObject();
                const QString &uuid = obj.value("partuuid").toString();

                if (!uuid.isEmpty() && children_uuids.contains(uuid))
                    continue;

                DDevicePartInfo info;

                info.d_func()->init(obj);

                if (info.isExtended())
                    continue;

                if (!info.partUUID().isEmpty() && children_uuids.contains(info.partUUID()))
                    continue;

                info.d->transport = transport;
                list << info;
                children_uuids << info.partUUID();
            }
        } else {
            DDevicePartInfo info;

            info.d_func()->init(obj);

            if (info.isExtended())
                continue;

            info.d->transport = transport;
            list << info;
        }
    }

    return list;
}

void DDevicePartInfo::init(const QJsonObject &obj)
{
    d_func()->init(obj);
}
