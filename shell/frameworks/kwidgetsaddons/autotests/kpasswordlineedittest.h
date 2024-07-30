/*
    SPDX-FileCopyrightText: 2017 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PASSWORDLINEEDITTEST_H
#define PASSWORDLINEEDITTEST_H

#include <QObject>

class PasswordLineEditTest : public QObject
{
    Q_OBJECT
public:
    explicit PasswordLineEditTest(QObject *parent = nullptr);
    ~PasswordLineEditTest() override = default;
private Q_SLOTS:
    void shouldHaveDefaultValue();
    void shouldShowTogglePassword();
    void shouldNotShowToggleWhenSetPassword();
    void shouldShowRevealPassword();
    void shouldEmitSignalPasswordChanged();
    void testReadOnly();
};

#endif
