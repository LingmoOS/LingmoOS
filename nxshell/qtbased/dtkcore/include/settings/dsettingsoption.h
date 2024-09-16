// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QVariant>
#include <QObject>
#include <QPointer>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DSettingsGroup;
class DSettingsOptionPrivate;
class LIBDTKCORESHARED_EXPORT DSettingsOption : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

public:
    explicit DSettingsOption(QObject *parent = Q_NULLPTR);
    ~DSettingsOption();

    QPointer<DSettingsGroup> parentGroup() const;
    void setParentGroup(QPointer<DSettingsGroup> parentGroup);

    QString key() const;
    QString name() const;
    bool canReset() const;
    QVariant defaultValue() const;
    QVariant value() const;
    QVariant data(const QString &dataType) const;

    QString viewType() const;
    bool isHidden() const;

    static QPointer<DSettingsOption> fromJson(const QString &prefixKey, const QJsonObject &json);
Q_SIGNALS:
    void valueChanged(QVariant value);
    void dataChanged(const QString &dataType, QVariant value);

public Q_SLOTS:
    void setValue(QVariant value);
    void setData(const QString &dataType, QVariant value);

private:
    void parseJson(const QString &prefixKey, const QJsonObject &option);

    QScopedPointer<DSettingsOptionPrivate> dd_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(dd_ptr), DSettingsOption)
};

typedef QPointer<DSettingsOption> OptionPtr;

DCORE_END_NAMESPACE
