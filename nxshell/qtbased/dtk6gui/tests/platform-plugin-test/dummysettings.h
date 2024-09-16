// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DUMMYSETTINGS_H
#define DUMMYSETTINGS_H

#include <QObject>
#include "dplatformsettings.h"
DGUI_USE_NAMESPACE

class DummySettingsPrivate;
class DummySettings : public DPlatformSettings
{
public:
    explicit DummySettings(const QString &domain = QString());
    virtual ~DummySettings();

    virtual bool initialized() const;
    virtual bool isEmpty() const;

    virtual bool contains(const QByteArray &property) const;
    virtual QVariant setting(const QByteArray &property) const;
    virtual void setSetting(const QByteArray &property, const QVariant &value);
    virtual QByteArrayList settingKeys() const;

    virtual void emitSignal(const QByteArray &signal, qint32 data1, qint32 data2);
private:
    QScopedPointer<DummySettingsPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DummySettings)
};

#endif // DUMMYSETTINGS_H
