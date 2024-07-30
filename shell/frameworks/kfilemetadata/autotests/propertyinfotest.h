/*
    This file is part of the KDE KFileMetaData project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PROPERTYINFOTEST_H
#define PROPERTYINFOTEST_H

#include <QObject>

namespace KFileMetaData {

class PropertyInfoTest : public QObject
{
    Q_OBJECT
public:
    void setLocalized(bool);

private Q_SLOTS:
    void init();
    void testNameIdMapping();
    void testFormatAsDisplayString();
    void testFormatAsDisplayString_data();
    void benchmarkPropertyInfo();
    void benchmarkPropertyInfo_data();
    void benchmarkPropertyInfoFromName();
    void benchmarkPropertyInfoFromName_data();
    void benchmarkPropertyInfoDisplayName();
    void benchmarkPropertyInfoDisplayName_data();
private:
    bool m_useLocalization = false;
};

}

#endif // PROPERTYINFOTEST_H
