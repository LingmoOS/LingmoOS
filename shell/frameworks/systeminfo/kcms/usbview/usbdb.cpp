/*
 *   SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "usbdb.h"

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStandardPaths>
#include <QTextStream>

USBDB::USBDB()
{
    QString db = QStringLiteral("/usr/share/hwdata/usb.ids"); /* on Fedora and Arch*/
    if (!QFile::exists(db)) {
        db = QStringLiteral("/usr/share/misc/usb.ids"); /* on Gentoo */
    }
    if (db.isEmpty())
        return;

    QFile f(db);

    if (f.open(QIODevice::ReadOnly)) {
        QTextStream ts(&f);

        QString line;
        int id = -1, subid = -1;
        const QRegularExpression vendor(QStringLiteral("^([0-9a-fA-F]{4})\\s+(.+)$"));
        const QRegularExpression product(QStringLiteral("^\t([0-9a-fA-F]{4})\\s+(.+)$"));
        const QRegularExpression cls(QStringLiteral("^C ([0-9a-fA-F]{2})\\s+(.+)$"));
        const QRegularExpression subclass(QStringLiteral("^\t([0-9a-fA-F]{2})\\s+(.+)$"));
        const QRegularExpression prot(QStringLiteral("^\t\t([0-9a-fA-F]{2})\\s+(.+)$"));
        while (ts.readLineInto(&line)) {
            if (!line.isEmpty() && line.startsWith(QLatin1String("#")))
                continue;

            // skip unhandled categories lines
            if (line.isEmpty() || line.startsWith(QLatin1String("AT ")) || line.startsWith(QLatin1String("HID ")) || line.startsWith(QLatin1String("R "))
                || line.startsWith(QLatin1String("BIAS ")) || line.startsWith(QLatin1String("PHY ")) || line.startsWith(QLatin1String("HUT "))
                || line.startsWith(QLatin1String("L ")) || line.startsWith(QLatin1String("HCC ")) || line.startsWith(QLatin1String("VT "))) {
                id = -1;
                subid = -1;
                continue;
            }

            QRegularExpressionMatch match;
            if (line.startsWith('C') && (match = cls.match(line)).hasMatch()) {
                id = match.capturedView(1).toInt(nullptr, 16);
                const QString name = match.capturedView(2).trimmed().toString();
                _classes.insert(QStringLiteral("%1").arg(id), name);
            } else if (id >= 0 && subid >= 0 && (match = prot.match(line)).hasMatch()) {
                const int protid = match.capturedView(1).toInt(nullptr, 16);
                const QString name = match.capturedView(2).trimmed().toString();
                _classes.insert(QStringLiteral("%1-%2-%3").arg(id).arg(subid).arg(protid), name);
            } else if (id >= 0 && (match = subclass.match(line)).hasMatch()) {
                subid = match.capturedView(1).toInt(nullptr, 16);
                const QString name = match.capturedView(2).trimmed().toString();
                _classes.insert(QStringLiteral("%1-%2").arg(id).arg(subid), name);
            } else if ((match = vendor.match(line)).hasMatch()) {
                id = match.capturedView(1).toInt(nullptr, 16);
                const QString name = match.captured(2);
                _ids.insert(QStringLiteral("%1").arg(id), name);
            } else if (id >= 0 && (match = product.match(line)).hasMatch()) {
                subid = match.capturedView(1).toInt(nullptr, 16);
                const QString name = match.capturedView(2).trimmed().toString();
                _ids.insert(QStringLiteral("%1-%2").arg(id).arg(subid), name);
            } else {
                id = -1;
                subid = -1;
            }
        }

        f.close();
    }
}

QString USBDB::vendor(uint16_t id) const
{
    QString s = _ids.value(QStringLiteral("%1").arg(id));
    if (id != 0) {
        return s;
    }
    return QString();
}

QString USBDB::device(uint16_t vendor, uint16_t id) const
{
    QString s = _ids.value(QStringLiteral("%1-%2").arg(vendor).arg(id));
    if ((id != 0) && (vendor != 0))
        return s;
    return QString();
}

QString USBDB::cls(uint8_t cls) const
{
    return _classes.value(QStringLiteral("%1").arg(cls));
}

QString USBDB::subclass(uint8_t cls, uint8_t sub) const
{
    return _classes.value(QStringLiteral("%1-%2").arg(cls).arg(sub));
}

QString USBDB::protocol(uint8_t cls, uint8_t sub, uint8_t prot) const
{
    return _classes.value(QStringLiteral("%1-%2-%3").arg(cls).arg(sub).arg(prot));
}
