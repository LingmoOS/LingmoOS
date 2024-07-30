/*
    SPDX-License-Identifier: LGPL-2.0-or-later
    SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>
*/

#ifndef KCURSORSAVERTEST_H
#define KCURSORSAVERTEST_H

#include <QObject>

class KCursorSaverTest : public QObject
{
    Q_OBJECT
public:
    explicit KCursorSaverTest(QObject *parent = nullptr);
    ~KCursorSaverTest() override = default;
private Q_SLOTS:
    void ignoreWarning();
};

#endif // KCURSORSAVERTEST_H
