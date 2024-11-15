/*
 *   SPDX-FileCopyrightText: 2010 Alex Fiestas <alex@eyeos.org>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "bluetoothagent.h"

#include <QDBusObjectPath>
#include <QFile>
#include <QStandardPaths>
#include <QXmlStreamReader>

#include <BluezQt/Device>
#include <QDebug>

#include <QThread>
#include <QThreadStorage>

#include <unistd.h>

static int cRandom()
{
    static QThreadStorage<bool> initialized_threads;
    if (!initialized_threads.localData()) {
        unsigned int seed;
        initialized_threads.setLocalData(true);
        QFile urandom(QStringLiteral("/dev/urandom"));
        bool opened = urandom.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
        if (!opened || urandom.read(reinterpret_cast<char *>(&seed), sizeof(seed)) != sizeof(seed)) {
            // silence warnings about use of deprecated qsrand()/qrand()
            // Porting to QRandomGenerator::global() instead might result in no new seed set for the generator behind qrand()
            // which then might affect other places indirectly relying on this.
            // So just keeping the old calls here, as this method is also deprecated and will disappear together with qsrand/qrand.
            QT_WARNING_PUSH
            QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
            QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
            // No /dev/urandom... try something else.
            qsrand(getpid());
            seed = qrand() ^ time(nullptr) ^ reinterpret_cast<quintptr>(QThread::currentThread());
        }
        qsrand(seed);
    }
    return qrand();
    QT_WARNING_POP
}

BluetoothAgent::BluetoothAgent(QObject *parent)
    : BluezQt::Agent(parent)
    , m_fromDatabase(false)
{
}

QString BluetoothAgent::pin()
{
    return m_pin;
}

void BluetoothAgent::setPin(const QString &pin)
{
    m_pin = pin;
    m_fromDatabase = false;
}

bool BluetoothAgent::isFromDatabase()
{
    return m_fromDatabase;
}

QString BluetoothAgent::getPin(BluezQt::DevicePtr device)
{
    m_fromDatabase = false;
    m_pin = QString::number(cRandom());
    m_pin = m_pin.left(6);

    const QString &xmlPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("pin-code-database.xml"));

    QFile file(xmlPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Can't open the pin-code-database.xml";
        return m_pin;
    }

    QXmlStreamReader xml(&file);

    QString deviceType = BluezQt::Device::typeToString(device->type());
    if (deviceType == QLatin1String("audiovideo")) {
        deviceType = QStringLiteral("audio");
    }

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.name() != QLatin1String("device")) {
            continue;
        }
        QXmlStreamAttributes attr = xml.attributes();

        if (attr.count() == 0) {
            continue;
        }

        if (attr.hasAttribute(QLatin1String("type")) && attr.value(QLatin1String("type")) != QLatin1String("any")) {
            if (deviceType != attr.value(QLatin1String("type")).toString()) {
                continue;
            }
        }

        if (attr.hasAttribute(QLatin1String("oui"))) {
            if (!device->address().startsWith(attr.value(QLatin1String("oui")).toString())) {
                continue;
            }
        }

        if (attr.hasAttribute(QLatin1String("name"))) {
            if (device->name() != attr.value(QLatin1String("name")).toString()) {
                continue;
            }
        }

        m_pin = attr.value(QLatin1String("pin")).toString();
        m_fromDatabase = true;
        if (m_pin.startsWith(QLatin1String("max:"))) {
            m_fromDatabase = false;
            int num = m_pin.rightRef(m_pin.length() - 4).toInt();
            m_pin = QString::number(cRandom()).left(num);
        }

        qDebug() << "PIN: " << m_pin;
        return m_pin;
    }

    return m_pin;
}

QDBusObjectPath BluetoothAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/agent"));
}

void BluetoothAgent::requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &req)
{
    qDebug() << "AGENT-RequestPinCode" << device->ubi();

    Q_EMIT pinRequested(m_pin);
    req.accept(m_pin);
}

void BluetoothAgent::displayPinCode(BluezQt::DevicePtr device, const QString &pinCode)
{
    qDebug() << "AGENT-DisplayPinCode" << device->ubi() << pinCode;

    Q_EMIT pinRequested(pinCode);
}

void BluetoothAgent::requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &req)
{
    qDebug() << "AGENT-RequestPasskey" << device->ubi();

    Q_EMIT pinRequested(m_pin);
    req.accept(m_pin.toUInt());
}

void BluetoothAgent::displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered)
{
    Q_UNUSED(entered);

    qDebug() << "AGENT-DisplayPasskey" << device->ubi() << passkey;

    Q_EMIT pinRequested(passkey);
}

void BluetoothAgent::requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &req)
{

    qDebug() << "AGENT-RequestConfirmation " << device->ubi() << passkey;

    Q_EMIT confirmationRequested(passkey, req);
}
