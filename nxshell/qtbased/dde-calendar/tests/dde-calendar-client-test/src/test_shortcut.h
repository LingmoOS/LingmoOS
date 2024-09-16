// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_SHORTCUT_H
#define TEST_SHORTCUT_H

#include <QObject>

class test_Shortcut : public QObject
{
    Q_OBJECT
public:
    explicit test_Shortcut(QObject *parent = nullptr);

signals:

public slots:
};

#endif // TEST_SHORTCUT_H