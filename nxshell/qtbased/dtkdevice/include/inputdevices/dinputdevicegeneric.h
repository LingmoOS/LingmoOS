// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICEGENERIC_H
#define DINPUTDEVICEGENERIC_H

#include "dtkdevice_global.h"

#include <DExpected>
#include <DObject>

#include <QObject>
#include <QSharedPointer>

#include "dtkinputdevices_types.h"

DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DObject;
class DInputDeviceGenericPrivate;
class LIBDTKDEVICESHARED_EXPORT DInputDeviceGeneric : public QObject, public DObject
{
    Q_OBJECT
    Q_PROPERTY(quint32 id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(DeviceType type READ type CONSTANT)
    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)

public:
    virtual quint32 id() const;
    virtual QString name() const;
    virtual DeviceType type() const;
    virtual bool enabled() const;

    using Ptr = QSharedPointer<DInputDeviceGeneric>;
    ~DInputDeviceGeneric() override;

Q_SIGNALS:
    void enabledChanged(bool enabled);

public Q_SLOTS:
    virtual DExpected<void> reset();

protected:
    explicit DInputDeviceGeneric(const DeviceInfo &info = DeviceInfoInitializer, bool enabled = true, QObject *parent = nullptr);
    DInputDeviceGeneric(DInputDeviceGenericPrivate &dd, const DeviceInfo &info, bool enabled, QObject *parent);
    void setId(quint32 id);
    void setName(const QString &name);
    void setType(DeviceType type);
    void setEnabled(bool enabled);
    void setDeviceInfo(const DeviceInfo &info);

private:
    friend class DInputDeviceManager;
    D_DECLARE_PRIVATE(DInputDeviceGeneric)
};

DDEVICE_END_NAMESPACE

#endif
