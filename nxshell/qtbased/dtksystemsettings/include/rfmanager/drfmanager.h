// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DRFMANAGER_H
#define DRFMANAGER_H

#include "dtkrfmanager_global.h"
#include "dtksystemsettings_global.h"

#include <dexpected.h>

#include <QObject>

DRFMGR_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;
class DRfmanagerPrivate;

class LIBDTKSYSTEMSETTINGSSHARED_EXPORT DRfmanager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool bluetoothBlocked READ isBluetoothBlocked WRITE blockBluetooth NOTIFY
                       bluetoothBlockedChanged)
    Q_PROPERTY(bool wifiBlocked READ isWifiBlocked WRITE blockWifi NOTIFY wifiBlockedChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool allBlocked READ isAllBlocked WRITE blockAll NOTIFY allBlockedChanged)

public:
    //!< @~english enum RfOp - rf device operation types
    enum RfOperation {
        ADD = 0,    /*!< @~english a device was added */
        DEL,        /*!< @~english a device was removed */
        CHANGE,     /*!< @~english a device's state changed -- userspace changes one device */
        CHANGE_ALL, /*!< @~english userspace changes all devices (of a type, or all)
                        into a state, also updating the default state used for devices that
                        are hot-plugged later. */
    };
    Q_ENUM(RfOperation)

    //!< @~english enum RFType type of rf device type.
    enum RfType {
        ALL = 0,     /*!< @~english toggles all switches (requests only - not a switch type) */
        WLAN,        /*!< @~english switch is on a 802.11 wireless network device. e.g. wifi */
        WIFI = WLAN, /*!< @~english alias of wlan */
        BLUETOOTH,   /*!< @~english switch is on a bluetooth device. */
        UWB,         /*!< @~english switch is on a ultra wideband device. */
        WIMAX,       /*!< @~english switch is on a WiMAX device. */
        WWAN,        /*!< @~english switch is on a wireless WAN device. */
        GPS,         /*!< @~english switch is on a GPS device. */
        FM,          /*!< @~english switch is on a FM radio device. */
        NFC,         /*!< @~english switch is on an NFC device. */

        NUM_TYPES, /*!< @~english switch is on a 802.11 wireless network device. */
    };
    Q_ENUM(RfType)

    //!< @~english structure used for userspace communication on /dev/rfkill,
    struct RfDevice
    {
        quint32 idx;      /*!< @~english index of device */
        RfType type;      /*!< @~english device type */
        QString name;     /*!< @~english name of device */
        bool softBlocked; /*!< @~english devive soft block state */
        bool hardBlocked; /*!< @~english devive hard block state */
    };

    explicit DRfmanager(QObject *parent = nullptr);
    virtual ~DRfmanager() override;

    bool block(RfType type, bool blocked = true);
    bool block(quint32 id, bool blocked = true);

    bool isBluetoothBlocked() const;
    bool isWifiBlocked() const;
    bool isAllBlocked() const;
    int count() const;
    QList<RfDevice> deviceList() const;

public slots:
    DExpected<bool> blockBluetooth(bool bluetoothBlocked = true);
    DExpected<bool> blockWifi(bool wifiBlocked = true);
    DExpected<bool> blockAll(bool blockAll = true);

signals:
    void bluetoothBlockedChanged(bool bluetoothBlocked);
    void wifiBlockedChanged(bool wifiBlocked);
    void countChanged(int count);
    void allBlockedChanged(bool blockAll);
    void blockedChanged(quint32 idx);

private:
    QScopedPointer<DRfmanagerPrivate> d_dptr;
    Q_DECLARE_PRIVATE_D(d_dptr, DRfmanager)
};

#if !defined(QT_NO_DEBUG_STREAM)
Q_CORE_EXPORT QDebug operator<<(QDebug, const DRfmanager::RfDevice &);
#endif

DRFMGR_END_NAMESPACE
#endif // DRFMANAGER_H
