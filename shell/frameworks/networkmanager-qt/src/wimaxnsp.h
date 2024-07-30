/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2011-2013 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIMAXNSP_H
#define NETWORKMANAGERQT_WIMAXNSP_H

#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QSharedPointer>

namespace NetworkManager
{
class WimaxNspPrivate;

/**
 * Wimax network service provider (access point)
 */
class NETWORKMANAGERQT_EXPORT WimaxNsp : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<WimaxNsp> Ptr;
    typedef QList<Ptr> List;
    /**
     * network types a NSP can have
     */
    enum NetworkType {
        Unknown = 0x1,
        Home = 0x2,
        Partner = 0x3,
        RoamingPartner = 0x4,
    };

    explicit WimaxNsp(const QString &path, QObject *parent = nullptr);
    ~WimaxNsp() override;

    QString uni() const;
    /**
     * The network type of the NSP
     */
    NetworkType networkType() const;
    /**
     * The name of the NSP
     */
    QString name() const;
    /**
     * The current signal quality of the NSP, in percent
     */
    uint signalQuality() const;

Q_SIGNALS:
    /**
     * This signal is emitted when the network type of this NSP has changed.
     *
     * @param type the new type
     */
    void networkTypeChanged(NetworkType type);

    /**
     * This signal is emitted when the name of this NSP has changed
     *
     * @param name the new name for this NSP
     */
    void nameChanged(const QString &name);

    /**
     * This signal is emitted when the signal quality of this NSP has changed.
     *
     * @param quality the new quality
     */
    void signalQualityChanged(uint quality);

private:
    Q_DECLARE_PRIVATE(WimaxNsp)

    WimaxNspPrivate *const d_ptr;
};
}
#endif
