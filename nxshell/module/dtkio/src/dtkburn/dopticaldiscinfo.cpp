// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dopticaldiscinfo_p.h"

DBURN_BEGIN_NAMESPACE

void DOpticalDiscInfoPrivate::iniData()
{
    if (!isoEngine->acquireDevice(devid)) {
        qWarning() << "[dtkburn]: Init data failed, cannot acquire device";
        devid = "";
        return;
    }
    media = isoEngine->mediaTypeProperty();
    isoEngine->mediaStorageProperty(&data, &avail, &datablocks);
    formatted = isoEngine->mediaFormattedProperty();
    volid = isoEngine->mediaVolIdProperty();
    writespeed = isoEngine->mediaSpeedProperty();
    isoEngine->clearResult();
    isoEngine->releaseDevice();
}

DOpticalDiscInfo::DOpticalDiscInfo(const DOpticalDiscInfo &info)
    : d_ptr(info.d_ptr)
{
}

DOpticalDiscInfo &DOpticalDiscInfo::operator=(const DOpticalDiscInfo &info)
{
    if (this == &info)
        return *this;

    d_ptr = info.d_ptr;
    return *this;
}

DOpticalDiscInfo::~DOpticalDiscInfo()
{
}

bool DOpticalDiscInfo::blank() const
{
    return d_ptr->formatted;
}

QString DOpticalDiscInfo::device() const
{
    return d_ptr->devid;
}

QString DOpticalDiscInfo::volumeName() const
{
    return d_ptr->volid;
}

quint64 DOpticalDiscInfo::usedSize() const
{
    return d_ptr->data;
}

quint64 DOpticalDiscInfo::availableSize() const
{
    return d_ptr->avail;
}

quint64 DOpticalDiscInfo::totalSize() const
{
    return usedSize() + availableSize();
}

quint64 DOpticalDiscInfo::dataBlocks() const
{
    return d_ptr->datablocks;
}

MediaType DOpticalDiscInfo::mediaType() const
{
    return d_ptr->media;
}

QStringList DOpticalDiscInfo::writeSpeed() const
{
    return d_ptr->writespeed;
}

DOpticalDiscInfo::DOpticalDiscInfo(const QString &dev, QObject *parent)
    : QObject(parent), d_ptr(new DOpticalDiscInfoPrivate(dev))
{
}

DBURN_END_NAMESPACE
