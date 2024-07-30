/*
 *   SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@caldera.de>
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __USB_DB_H__
#define __USB_DB_H__

#include <QMap>
#include <QString>

#include <stdint.h>

class USBDB
{
public:
    USBDB();

    QString vendor(uint16_t id) const;
    QString device(uint16_t vendor, uint16_t id) const;

    QString cls(uint8_t cls) const;
    QString subclass(uint8_t cls, uint8_t sub) const;
    QString protocol(uint8_t cls, uint8_t sub, uint8_t prot) const;

private:
    QMap<QString, QString> _classes;
    QMap<QString, QString> _ids;
};

#endif
