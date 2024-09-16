// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QScopedPointer>

#include "dsettingsbackend.h"

DCORE_BEGIN_NAMESPACE

class QSettingBackendPrivate;
class LIBDTKCORESHARED_EXPORT QSettingBackend : public Dtk::Core::DSettingsBackend
{
    Q_OBJECT
public:
    explicit QSettingBackend(const QString &filepath, QObject *parent = 0);
    ~QSettingBackend();

    virtual QStringList keys() const Q_DECL_OVERRIDE;
    virtual QVariant getOption(const QString &key) const Q_DECL_OVERRIDE;

protected Q_SLOTS:
    virtual void doSetOption(const QString &key, const QVariant &value) Q_DECL_OVERRIDE;
    virtual void doSync() Q_DECL_OVERRIDE;

private:
    QScopedPointer<QSettingBackendPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), QSettingBackend)
};

DCORE_END_NAMESPACE
