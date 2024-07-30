/*
    SPDX-FileCopyrightText: 2014 Nicolás Alvarez <nicolas.alvarez@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef FACEICONTEST_H
#define FACEICONTEST_H

#include <QWidget>

class FaceIconTest : public QWidget
{
    Q_OBJECT
public:
    FaceIconTest();

private:
    class QListWidget *listWidget;
};

#endif
