// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <qdbuspendingreply.h>
#include <qdebug.h>
#include <qobject.h>
#include <qscopedpointer.h>

// class DKbdBacklightPrivate;

class UPowerKbdBacklightService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakePower")
public:
    explicit UPowerKbdBacklightService(QObject *parent = nullptr);
    virtual ~UPowerKbdBacklightService();

    // signals:
    //     Q_SCRIPTABLE void PropertiesChanged(const QVariantMap &properties);

public slots:

    Q_SCRIPTABLE void Reset() { m_reset = true; }

    Q_SCRIPTABLE qint32 GetBrightness() const;
    Q_SCRIPTABLE qint32 GetMaxBrightness() const;
    Q_SCRIPTABLE void SetBrightness(const qint32 value);

public:
    bool m_reset;

private:
    void registerService();
    void unRegisterService();
};
