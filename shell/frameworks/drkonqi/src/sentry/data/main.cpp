// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

// Collects various data into a json blob for use in the sentry payloads.

#include <chrono>
#include <iostream>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QJsonDocument>
#include <QJsonObject>

#include <KOSRelease>

using namespace std::chrono_literals;
using namespace Qt::StringLiterals;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QVariantHash blob;

    struct BusInterface {
        QString service;
        QString path;
        QString interface;
        QDBusConnection bus = QDBusConnection::sessionBus();
    };

    auto fillProperties = [&blob]<typename... Args>(const BusInterface &iface, Args &&...properties) {
        QDBusInterface dbusIface(iface.service, iface.path, iface.interface, iface.bus);
        dbusIface.setTimeout(std::chrono::milliseconds(4s).count()); // arbitrarily low timeout
        for (const auto &property : {properties...}) {
            const QVariant value = dbusIface.property(qUtf8Printable(property));
            if (value.isValid()) {
                Q_ASSERT(!blob.contains(property));
                blob[property] = value;
            }
        }
    };

    fillProperties({.service = u"org.freedesktop.hostname1"_qs,
                    .path = u"/org/freedesktop/hostname1"_qs,
                    .interface = u"org.freedesktop.hostname1"_qs,
                    .bus = QDBusConnection::systemBus()},
                   u"Hostname"_qs,
                   u"Chassis"_qs);

    fillProperties({.service = u"org.freedesktop.systemd1"_qs,
                    .path = u"/org/freedesktop/systemd1"_qs,
                    .interface = u"org.freedesktop.systemd1.Manager"_qs,
                    .bus = QDBusConnection::sessionBus()},
                   u"Virtualization"_qs);
    // Convert to bool
    blob[u"Virtualization"_qs] = !blob[u"Virtualization"_qs].toString().isEmpty();

    fillProperties({.service = u"org.freedesktop.timedate1"_qs,
                    .path = u"/org/freedesktop/timedate1"_qs,
                    .interface = u"org.freedesktop.timedate1"_qs,
                    .bus = QDBusConnection::systemBus()},
                   u"Timezone"_qs);

    KOSRelease os;
    blob[u"OS_NAME"_qs] = os.name();
    blob[u"OS_VERSION_ID"_qs] = os.versionId();
    blob[u"OS_BUILD_ID"_qs] = os.buildId();
    blob[u"OS_VARIANT_ID"_qs] = os.variantId();

    QJsonDocument doc(QJsonObject::fromVariantHash(blob));
    std::cout << doc.toJson().toStdString() << std::endl;

    return 0;
}
