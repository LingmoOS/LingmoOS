// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QPointer>

#include "dsettingsoption.h"

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DSettingsGroupPrivate;
class LIBDTKCORESHARED_EXPORT DSettingsGroup : public QObject
{
    Q_OBJECT
public:
    explicit DSettingsGroup(QObject *parent = Q_NULLPTR);
    ~DSettingsGroup();

    QPointer<DSettingsGroup> parentGroup() const;
    void setParentGroup(QPointer<DSettingsGroup> parentGroup);

    QString key() const;
    QString name() const;
    bool isHidden() const;

    QPointer<DSettingsGroup> childGroup(const QString &groupKey) const;
    QPointer<DSettingsOption> option(const QString &key) const;

    QList<QPointer<DSettingsGroup> > childGroups() const;
    QList<QPointer<DSettingsOption> > childOptions() const;
    QList<QPointer<DSettingsOption> > options() const;

    static QPointer<DSettingsGroup> fromJson(const QString &prefixKey, const QJsonObject &group);

private:
    void parseJson(const QString &prefixKey, const QJsonObject &group);

    QScopedPointer<DSettingsGroupPrivate> dd_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(dd_ptr), DSettingsGroup)
};

typedef QPointer<DSettingsGroup> GroupPtr;

DCORE_END_NAMESPACE
