// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dummysettings.h"
#include <QDebug>

class DummySettingsPrivate : public QObject
{
public:
    DummySettingsPrivate(DummySettings *q, const QString &domain, QObject *parent = nullptr);
    ~DummySettingsPrivate();

protected:
    QString domain;
    QByteArrayList keys;
    DummySettings *q_ptr = nullptr;
    bool initialized = true;

    Q_DECLARE_PUBLIC(DummySettings)
};

DummySettingsPrivate::DummySettingsPrivate(DummySettings *q, const QString &domain, QObject *parent)
    : QObject(parent)
    , domain(domain)
    , q_ptr(q)
{

}

DummySettingsPrivate::~DummySettingsPrivate()
{

}

DummySettings::DummySettings(const QString &domain)
    :d_ptr(new DummySettingsPrivate(this, domain))
{

}

DummySettings::~DummySettings()
{

}

bool DummySettings::initialized() const
{
    Q_D(const DummySettings);
    return d->initialized;
}

bool DummySettings::isEmpty() const
{
    Q_D(const DummySettings);
    return d->keys.isEmpty();
}

bool DummySettings::contains(const QByteArray &property) const
{
    Q_D(const DummySettings);
    return d->property(property.data()).isValid();
}

QVariant DummySettings::setting(const QByteArray &property) const
{
    Q_D(const DummySettings);
    return d->property(property.data());
}

void DummySettings::setSetting(const QByteArray &property, const QVariant &value)
{
    if (setting(property) == value)
        return;

    Q_D(DummySettings);
    if (!d->keys.contains(property))
        d->keys << property;

    d->setProperty(property.data(), value);

    // call baseobject's propertychanged
    handlePropertyChanged(property, value);
}

QByteArrayList DummySettings::settingKeys() const
{
    Q_D(const DummySettings);
    return d->keys;
}

void DummySettings::emitSignal(const QByteArray &signal, qint32 data1, qint32 data2)
{
    qInfo() << "emitSignal[" << signal << "] "<< data1 << data2;
}
