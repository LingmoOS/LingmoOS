// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QScopedPointer>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DSettings;
class LIBDTKCORESHARED_EXPORT DSettingsBackend : public QObject
{
    Q_OBJECT
public:
    explicit DSettingsBackend(QObject *parent = Q_NULLPTR): QObject(parent)
    {
        connect(this, &DSettingsBackend::sync, this, &DSettingsBackend::doSync, Qt::QueuedConnection);
        connect(this, &DSettingsBackend::setOption, this, &DSettingsBackend::doSetOption, Qt::QueuedConnection);
    }
    virtual ~DSettingsBackend() {}

    virtual QStringList keys() const = 0;
    virtual QVariant getOption(const QString &key) const = 0;

    virtual void doSync() = 0;

protected:
    virtual void doSetOption(const QString &key, const QVariant &value) = 0;

Q_SIGNALS:
    void optionChanged(const QString &key, const QVariant &value);

    // private signals;
Q_SIGNALS:
    void sync();
    void setOption(const QString &key, const QVariant &value);
};

DCORE_END_NAMESPACE
