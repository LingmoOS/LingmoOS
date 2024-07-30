/*
    SPDX-FileCopyrightText: 2017 Henrik Fehlauer <rkflx@lab12.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSQUEEZEDTEXTLABELAUTOTEST_H
#define KSQUEEZEDTEXTLABELAUTOTEST_H

#include <QObject>

class KSqueezedTextLabel;

class KSqueezedTextLabelAutotest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testEmptyText();
    void testElisionOnResize_data();
    void testElisionOnResize();
    void testElisionOnTextUpdate();
    void testElideMode_data();
    void testElideMode();
    void testSizeHints();
    void testClearing();
    void testChrome_data();
    void testChrome();
};

#endif
