/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2022 Mirco Miranda

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KMEMORYINFOTEST_H
#define KMEMORYINFOTEST_H

#include <QObject>

/**
 * @brief The KMemoryInfoTest class
 */
class KMemoryInfoTest : public QObject
{
    Q_OBJECT
public:
    KMemoryInfoTest(QObject *parent = nullptr);

private Q_SLOTS:
    void isNull();

    void operators();
};

#endif // KMEMORYINFOTEST_H
