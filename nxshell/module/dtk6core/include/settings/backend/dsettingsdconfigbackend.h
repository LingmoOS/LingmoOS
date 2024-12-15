// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QScopedPointer>

#include "dsettingsbackend.h"

DCORE_BEGIN_NAMESPACE

class DSettingsDConfigBackendPrivate;
class LIBDTKCORESHARED_EXPORT DSettingsDConfigBackend : public Dtk::Core::DSettingsBackend
{
    Q_OBJECT
public:
    explicit DSettingsDConfigBackend(const QString &name, const QString &subpath = QString(), QObject *parent = nullptr);
    ~DSettingsDConfigBackend() Q_DECL_OVERRIDE;

    virtual QStringList keys() const Q_DECL_OVERRIDE;
    virtual QVariant getOption(const QString &key) const Q_DECL_OVERRIDE;

protected Q_SLOTS:
    virtual void doSetOption(const QString &key, const QVariant &value) Q_DECL_OVERRIDE;
    virtual void doSync() Q_DECL_OVERRIDE;

private:
    QScopedPointer<DSettingsDConfigBackendPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DSettingsDConfigBackend)
};

DCORE_END_NAMESPACE
