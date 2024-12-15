// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QPointer>
#include <QScopedPointer>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DSettingsBackend;
class DSettingsOption;
class DSettingsGroup;
class DSettingsPrivate;
class LIBDTKCORESHARED_EXPORT DSettings : public QObject
{
    Q_OBJECT
public:
    explicit DSettings(QObject *parent = Q_NULLPTR);
    ~DSettings();

    void setBackend(DSettingsBackend *backend = nullptr);

    static QPointer<DSettings> fromJson(const QByteArray &json);
    static QPointer<DSettings> fromJsonFile(const QString &filepath);
    QJsonObject meta() const;

    QStringList keys() const;
    QList<QPointer<DSettingsOption>> options() const;
    QPointer<DSettingsOption> option(const QString &key) const;
    QVariant value(const QString &key) const;

    QStringList groupKeys() const;
    QList<QPointer<DSettingsGroup>> groups() const;
    QPointer<DSettingsGroup> group(const QString &key) const;

    QVariant getOption(const QString &key) const;

Q_SIGNALS:
    void valueChanged(const QString &key, const QVariant &value);

public Q_SLOTS:
    //!
    //! \brief sync
    //! WARNING: sync will block
    void sync() ;

    void setOption(const QString &key, const QVariant &value);
    void reset() ;

private:
    void parseJson(const QByteArray &json);
    void loadValue();

    QScopedPointer<DSettingsPrivate> dd_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(dd_ptr), DSettings)
};

DCORE_END_NAMESPACE
