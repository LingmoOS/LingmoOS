// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOPTICALDISCINFO_P_H
#define DOPTICALDISCINFO_P_H

#include <memory>

#include <QSharedData>

#include "dopticaldiscinfo.h"
#include "dxorrisoengine.h"

DBURN_BEGIN_NAMESPACE

class DOpticalDiscInfoPrivate : public QSharedData
{
public:
    inline explicit DOpticalDiscInfoPrivate(const QString &dev)
        : isoEngine(std::make_unique<DXorrisoEngine>()), devid(dev)
    {
        iniData();
    }

    inline DOpticalDiscInfoPrivate(const DOpticalDiscInfoPrivate &copy)
        : QSharedData(copy)
    {
    }

    inline ~DOpticalDiscInfoPrivate() {}

    void iniData();

    std::unique_ptr<DXorrisoEngine> const isoEngine;
    bool formatted {};   //!< @~english True when the media in device is blank or false otherwise.
    MediaType media;   //!< @~english Type of media currently in the device.
    quint64 data {};   //!< @~english Size of on-disc data in bytes.
    quint64 avail {};   //!< @~english Available size in bytes.
    quint64 datablocks {};   //!< @~english Size of on-disc data in number of blocks.
    QStringList writespeed {};   //!< @~english List of write speeds supported, e.g. "10.0x".
    QString devid {};   //!< @~english Device identifier. Empty if the device property is invalid.
    QString volid {};   //!< @~english Volume name of the disc.
};

DBURN_END_NAMESPACE

#endif   // DOPTICALDISCINFO_P_H
