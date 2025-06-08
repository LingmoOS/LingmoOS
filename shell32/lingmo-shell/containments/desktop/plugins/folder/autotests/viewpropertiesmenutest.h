/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: 2017 Laurent Montel <laurent.montel@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

class ViewPropertiesMenuTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void shouldHaveDefaultValues();
};
