// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACCESSPOINTINTERFACE_H
#define DACCESSPOINTINTERFACE_H

#include "dnetworkmanagertypes.h"
#include <DDBusInterface>
#include <QString>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DAccessPointInterface : public QObject
{
    Q_OBJECT
public:
    explicit DAccessPointInterface(const QByteArray &path, QObject *parent = nullptr);
    ~DAccessPointInterface() override = default;

    Q_PROPERTY(QString SSID READ SSID NOTIFY SsidChanged)
    Q_PROPERTY(quint8 strength READ strength NOTIFY StrengthChanged)
    Q_PROPERTY(quint32 flags READ flags NOTIFY FlagsChanged)
    Q_PROPERTY(quint32 frequency READ frequency NOTIFY FrequencyChanged)
    Q_PROPERTY(quint32 rsnFlags READ rsnFlags NOTIFY RsnFlagsChanged)
    Q_PROPERTY(quint32 wpaFlags READ wpaFlags NOTIFY WpaFlagsChanged)

    QString SSID() const;
    quint8 strength() const;
    quint32 flags() const;
    quint32 frequency() const;
    quint32 rsnFlags() const;
    quint32 wpaFlags() const;

Q_SIGNALS:

    void SsidChanged(const QString &ssid);
    void StrengthChanged(const quint8 strength);
    void FrequencyChanged(const quint32 frequency);
    void FlagsChanged(const quint32 flags);
    void RsnFlagsChanged(const quint32 rsnFlags);
    void WpaFlagsChanged(const quint32 wpaFlags);

private:
    DDBusInterface *m_inter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
