/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 KDE-PIM team <kde-pim@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef GEOTEST_H
#define GEOTEST_H

#include <QObject>

class GeoTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void constructor();
    void isValid();
    void setData();
    void equals();
    void differs();
    void serialization();
    void shouldParseGeoVCard3();
    void shouldParseGeoVCard4();
    void shouldGenerateVCard3();
    void shouldGenerateVCard4();
    void shouldGenerateWithoutGeo();
};

#endif
