/*
    SPDX-FileCopyrightText: 2015 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNEWPASSWORDWIDGETTEST_H
#define KNEWPASSWORDWIDGETTEST_H

#include <QObject>

class KNewPasswordWidgetTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testEmptyPasswordAllowed();
    void testEmptyPasswordNotAllowed();
    void testPasswordTooShort();
    void testPasswordMatch();
    void testPasswordNotVerified();
    void testWeakPassword();
    void testStrongPassword();
    void testReasonablePasswordLength();
    void testPasswordStrengthWarningLevel();
    void testNoWarningColorBeforeMismatch();
    void testWarningColorIfMismatch();
    void testWarningColorPostMatch();
    void disablingWidgetShouldUseDisabledPalette();
    void disablingParentShouldUseDisabledPalette();
    void disablingRevealPasswordShouldHideVisibilityAction();
    void shouldNotHideVisibilityActionInPlaintextMode();
    void shouldHideVerificationLineEditInPlaintextMode();
};

#endif
