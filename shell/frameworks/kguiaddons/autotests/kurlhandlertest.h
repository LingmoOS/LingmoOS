/*
    SPDX-FileCopyrightText: 2021 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KURLHANDLERTEST_H
#define KURLHANDLERTEST_H

#include <QObject>

class KUrlHandlerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testOpeningDocsUrls_data();
    void testOpeningDocsUrls();
};

#endif // KURLHANDLERTEST_H
