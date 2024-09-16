// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QScopedPointer>

#include "dsettingsbackend.h"

DCORE_BEGIN_NAMESPACE

class GSettingsBackendPrivate;
class LIBDTKCORESHARED_EXPORT GSettingsBackend: public DSettingsBackend
{
    Q_OBJECT
public:
    explicit GSettingsBackend(DSettings *settings, QObject *parent = nullptr);
    ~GSettingsBackend();

    virtual QStringList keys() const Q_DECL_OVERRIDE;
    virtual QVariant getOption(const QString &key) const Q_DECL_OVERRIDE;

protected Q_SLOTS:
    virtual void doSetOption(const QString &key, const QVariant &value) Q_DECL_OVERRIDE;
    virtual void doSync() Q_DECL_OVERRIDE;

private:
    QScopedPointer<GSettingsBackendPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), GSettingsBackend)
};

DCORE_END_NAMESPACE
