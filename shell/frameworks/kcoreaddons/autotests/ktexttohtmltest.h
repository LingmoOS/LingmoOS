/*
    SPDX-FileCopyrightText: 2007 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTTOHTMLTEST_H
#define KTEXTTOHTMLTEST_H

#include <QObject>

class KTextToHTMLTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testGetEmailAddress();
    void testGetUrl();
    void testHtmlConvert();
    void testHtmlConvert_data();
    void testEmoticons_data();
    void testEmoticons();
    void testEmoticonsNoReplace_data();
    void testEmoticonsNoReplace();

private:
    void testGetUrl2(const QString &left, const QString &right);
};

#endif
