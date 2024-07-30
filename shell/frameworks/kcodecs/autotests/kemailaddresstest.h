/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2004 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef TESTEMAIL_H
#define TESTEMAIL_H

#include <QObject>

class KEmailAddressTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testGetNameAndEmail();
    void testGetNameAndEmail_data();
    void testIsValidEmailAddress();
    void testIsValidEmailAddress_data();
    void testIsValidAddressList();
    void testIsValidAddressList_data();
    void testIsValidSimpleEmailAddress();
    void testIsValidSimpleEmailAddress_data();
    void testGetEmailAddress();
    void testGetEmailAddress_data();
    void testCheckSplitEmailAddrList();
    void testCheckSplitEmailAddrList_data();
    void testNormalizeAddressesAndEncodeIDNs();
    void testNormalizeAddressesAndEncodeIDNs_data();
    void testNormalizeAddressesAndDecodeIDNs();
    void testNormalizeAddressesAndDecodeIDNs_data();
    void testQuoteIfNecessary();
    void testQuoteIfNecessary_data();
    void testMailtoUrls();
    void testMailtoUrls_data();
};

#endif
