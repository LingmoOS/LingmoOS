// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "drfmanager.h"

#include <linux/rfkill.h>

#include <QDebug>
#include <QFile>
#include <QSocketNotifier>

#include <fcntl.h>
#include <unistd.h>

#ifdef USE_FAKE_RFKILL_FILE
// for ut test file
#  define D_PATH_DEV_RFKILL "/tmp/dev/rfkill"
#  define D_SYS_CLS_RF_NAME "/tmp/dev/rfkill%1/name"
#else
#  define D_PATH_DEV_RFKILL "/dev/rfkill"
#  define D_SYS_CLS_RF_NAME "/sys/class/rfkill/rfkill%1/name"
#endif

DRFMGR_BEGIN_NAMESPACE

class DRfmanagerPrivate : public QObject
{
public:
    DRfmanagerPrivate(DRfmanager *q, QObject *parent = nullptr)
        : QObject(parent)
        , q_ptr(q)
    {
    }

    qint8 bluetoothBlocked = -1;
    qint8 wifiBlocked = -1;
    qint8 allBlocked = -1;
    QList<DRfmanager::RfDevice> deviceList;

    void init();
    int openRf(int openMode = O_RDONLY, bool nonblock = true);
    void readRf(int fd);
    int readRfEvent(int fd, rfkill_event *event);
    bool writeRfEvent(int fd, rfkill_event *event);
    QString deviceName(quint32 idx);
    bool deviceFromId(quint32 idx, DRfmanager::RfDevice *device = nullptr);
    void updateDevice(const rfkill_event &re);
    void updateDeviceInfo(const rfkill_event &re);
    void appendDevice(const rfkill_event &re);
    qint8 isBlocked(DRfmanager::RfType type);

    DRfmanager *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(DRfmanager)
};

void DRfmanagerPrivate::init()
{
    int fd = openRf(O_RDONLY);
    if (fd < 0) {
        qWarning("cannot open " D_PATH_DEV_RFKILL);
        return;
    }

    readRf(fd);

    QSocketNotifier *sn = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(sn, &QSocketNotifier::destroyed, this, [fd]() {
        close(fd);
    });
    connect(sn, &QSocketNotifier::activated, this, [fd, this](int socket) {
        if (socket == fd)
            readRf(fd);
    });
}

int DRfmanagerPrivate::openRf(int openMode, bool nonblock)
{
    int fd = open(D_PATH_DEV_RFKILL, openMode);
    if (fd < 0) {
        qWarning("cannot open " D_PATH_DEV_RFKILL);
        return fd;
    }

    if (nonblock && fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
        qWarning("cannot set non-blocking " D_PATH_DEV_RFKILL);
        close(fd);
        return -1;
    }

    return fd;
}

void DRfmanagerPrivate::readRf(int fd)
{
    rfkill_event event;
    ssize_t len = 0;
    while (1) {
        len = readRfEvent(fd, &event);
        if (len < 0)
            break;

        updateDevice(event);
    }
}

int DRfmanagerPrivate::readRfEvent(int fd, rfkill_event *event)
{
    ssize_t len = read(fd, event, sizeof(rfkill_event));
    if (len < 0) {
        if (errno == EAGAIN)
            return -1;

        qErrnoWarning("cannot read %s", D_PATH_DEV_RFKILL, strerror(errno));
        return -1;
    }

    if (static_cast<unsigned long>(len) < RFKILL_EVENT_SIZE_V1) {
        qWarning("wrong size of rfkill event: %zu < %lu", len, RFKILL_EVENT_SIZE_V1);
        return -1;
    }

    return int(len);
}

bool DRfmanagerPrivate::writeRfEvent(int fd, rfkill_event *event)
{
    ssize_t len = write(fd, event, sizeof(rfkill_event));
    if (len < 0)
        qErrnoWarning("write failed: %s", D_PATH_DEV_RFKILL, strerror(errno));
    else
        updateDevice(*event);

    return len == sizeof(rfkill_event);
}

QString DRfmanagerPrivate::deviceName(quint32 idx)
{
    // get device name
    QFile deviceFile(QString(D_SYS_CLS_RF_NAME).arg(idx));
    if (!deviceFile.open(QIODevice::ReadOnly)) {
        qWarning() << deviceFile.errorString();
        return QString();
    }

    return deviceFile.readAll().trimmed();
}

bool DRfmanagerPrivate::deviceFromId(quint32 idx, DRfmanager::RfDevice *device)
{
    auto it = std::find_if(deviceList.begin(),
                           deviceList.end(),
                           [idx](const DRfmanager::RfDevice &dev) {
                               return idx == dev.idx;
                           });

    if (device && it != deviceList.end())
        *device = *it;

    return it != deviceList.end();
}

void DRfmanagerPrivate::updateDevice(const rfkill_event &re)
{
    Q_Q(DRfmanager);

    switch (re.op) {
    case DRfmanager::ADD:
        appendDevice(re);
        updateDeviceInfo(re);
        break;
    case DRfmanager::DEL:
        for (int i = 0; i < deviceList.count(); i++) {
            if (re.idx == deviceList.at(i).idx) {
                deviceList.removeAt(i);
                break;
            }
        }
        emit q->countChanged(deviceList.count());
        updateDeviceInfo(re);
        break;
    case DRfmanager::CHANGE:
    case DRfmanager::CHANGE_ALL:
        updateDeviceInfo(re);
        if (re.idx != INT_MAX)
            emit q->blockedChanged(re.idx);
        break;
    default:
        break;
    }
}

void DRfmanagerPrivate::updateDeviceInfo(const rfkill_event &re)
{
    Q_Q(DRfmanager);

    if (re.op == DRfmanager::CHANGE_ALL) {
        for (auto &&device : deviceList)
            if (re.type == DRfmanager::ALL || re.type == device.type)
                device.softBlocked = re.soft;
    } else {
        for (auto &&device : deviceList) {
            if (re.idx == device.idx) {
                device.name = deviceName(device.idx);
                device.type = static_cast<DRfmanager::RfType>(re.type);
                device.softBlocked = re.soft;
                device.hardBlocked = re.hard;
            }
        }
    }

    auto oldBlockAll = allBlocked;
    auto oldBlockWifi = wifiBlocked;
    auto oldBlockBluetooth = bluetoothBlocked;

    wifiBlocked = isBlocked(DRfmanager::WIFI);
    bluetoothBlocked = isBlocked(DRfmanager::BLUETOOTH);
    allBlocked = isBlocked(DRfmanager::ALL);

#ifdef QT_DEBUG
    qDebug() << "wifiBlocked:" << wifiBlocked << "\tbluetoothBlocked:" << bluetoothBlocked
             << "\tallBlocked:" << allBlocked;
#endif

    // if device not found blocked state switch to false
    if (oldBlockBluetooth >= 0 && oldBlockBluetooth != bluetoothBlocked)
        emit q->bluetoothBlockedChanged(bluetoothBlocked);

    if (oldBlockWifi >= 0 && oldBlockWifi != wifiBlocked)
        emit q->wifiBlockedChanged(wifiBlocked);

    if (oldBlockAll >= 0 && oldBlockAll != allBlocked)
        emit q->allBlockedChanged(allBlocked);
}

void DRfmanagerPrivate::appendDevice(const rfkill_event &re)
{
    if (deviceFromId(re.idx)) {
        qWarning() << "device [" << re.idx << "already existed!";
        return;
    }

    DRfmanager::RfDevice device;
    device.idx = re.idx;
    device.name = deviceName(re.idx);
    device.type = static_cast<DRfmanager::RfType>(re.type);
    device.softBlocked = re.soft;
    device.hardBlocked = re.hard;
    deviceList.append(device);

    Q_Q(DRfmanager);
    emit q->countChanged(deviceList.count());
}

qint8 DRfmanagerPrivate::isBlocked(DRfmanager::RfType type)
{
    int count = 0;
    auto cmpFunc = [&](DRfmanager::RfDevice device) -> bool {
        if (type != device.type && type != DRfmanager::ALL)
            return false;

        ++count;
        return !(device.softBlocked | device.hardBlocked);
    };

    auto it = std::find_if(deviceList.begin(), deviceList.end(), cmpFunc);

    // if device type not found ? return -1
    // if device type found, check if devices blocked(soft and hard)
    return count > 0 ? it == deviceList.end() : -1;
}

DRfmanager::DRfmanager(QObject *parent)
    : QObject(parent)
    , d_dptr(new DRfmanagerPrivate(this, this))
{
    d_dptr->init();
}

DRfmanager::~DRfmanager() { }

/**
 * \~english \brief DRfmanager::block block specify type of device
 *
 * \~english \param type the type of device to be blocked
 * \~english \param blocked the state of device to be blocked or not blocked
 * \~english \return true if all works fine,otherwise return false
 * \~english \note userspace changes all type devices into a state blocked or unblocked
 *
 */
bool DRfmanager::block(DRfmanager::RfType type, bool blocked)
{
    Q_D(DRfmanager);

    rfkill_event event = {};
    event.idx = INT_MAX;
    event.op = CHANGE_ALL;
    event.type = type;
    event.soft = blocked;

    int fd = d->openRf(O_WRONLY, false);
    return d->writeRfEvent(fd, &event) && close(fd) == 0;
}

/**
 * \~english \brief DRfmanager::block block specify device with index id
 *
 * \~english \param id the id of device to be blocked
 * \~english \param blocked the state of device to be blocked or not blocked
 * \~english \return true if all works fine,otherwise return false
 * \~english \note userspace changes device with index id into a state blocked or unblocked
 *
 */
bool DRfmanager::block(quint32 id, bool blocked)
{
    Q_D(DRfmanager);

    if (d->deviceName(id).isEmpty())
        return false;

    rfkill_event event = {};
    event.op = CHANGE;
    event.idx = id;
    event.soft = blocked;

    DRfmanager::RfDevice device;
    if (d->deviceFromId(id, &device)) {
        event.type = device.type;
        event.hard = device.hardBlocked;
    }

    int fd = d->openRf(O_WRONLY, false);
    return d->writeRfEvent(fd, &event) && close(fd) == 0;
}

/**
 * \~english \brief DRfmanager::isBluetoothBlocked check if all bluetooth devices are blocked
 *
 * \~english \return true if all bluetooth devices are blocked,otherwise return false
 *
 */
bool DRfmanager::isBluetoothBlocked() const
{
    Q_D(const DRfmanager);
    return d->bluetoothBlocked >= 0 ? d->bluetoothBlocked : false;
}

/**
 * \~english \brief DRfmanager::isWifiBlocked check if all wifi(wlan) devices are blocked
 *
 * \~english \return true if all wifi(wlan) devices are blocked,otherwise return false
 *
 */
bool DRfmanager::isWifiBlocked() const
{
    Q_D(const DRfmanager);
    return d->wifiBlocked >= 0 ? d->wifiBlocked : false;
}

/**
 * \~english \brief DRfmanager::count get current state of all
 * \~english available devices list count
 *
 * \~english \return the count of rfkill supported devices list
 *
 */
int DRfmanager::count() const
{
    Q_D(const DRfmanager);
    return d->deviceList.count();
}

/**
 * \~english \brief DRfmanager::isWifiBlocked check if all wifi(wlan) devices are blocked
 *
 * \~english \return true if all wifi(wlan) devices are blocked,otherwise return false
 *
 */
bool DRfmanager::isAllBlocked() const
{
    Q_D(const DRfmanager);
    return d->allBlocked >= 0 ? d->allBlocked : false;
}

QList<DRfmanager::RfDevice> DRfmanager::deviceList() const
{
    Q_D(const DRfmanager);
    return d->deviceList;
}

DExpected<bool> DRfmanager::blockBluetooth(bool bluetoothBlocked /*= true*/)
{
    Q_D(DRfmanager);
    if (d->bluetoothBlocked == bluetoothBlocked && count() > 0)
        return true;

    return block(BLUETOOTH, bluetoothBlocked);
}

DExpected<bool> DRfmanager::blockWifi(bool wifiBlocked /*= true*/)
{
    Q_D(DRfmanager);
    if (d->wifiBlocked == wifiBlocked && count() > 0)
        return true;

    return block(WLAN, wifiBlocked);
}

DExpected<bool> DRfmanager::blockAll(bool blockAll /*= true*/)
{
    Q_D(DRfmanager);
    if (d->allBlocked == blockAll && count() > 0)
        return true;

    return block(ALL, blockAll);
}

#if !defined(QT_NO_DEBUG_STREAM)
QDebug operator<<(QDebug dbg, const DRfmanager::RfDevice &device)
{
    QDebugStateSaver save(dbg);
    dbg.resetFormat();

    dbg << "id:" << device.idx << "\ttype:" << device.type << "\tdevice:" << device.name
        << "\tsoft blocked:" << device.softBlocked << "\thard blocked:" << device.hardBlocked
        << "\n";
    return dbg;
}
#endif

DRFMGR_END_NAMESPACE
