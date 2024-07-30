/*
    SPDX-FileCopyrightText: 2014 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KMESSAGEWIDGETAUTOTEST_H
#define KMESSAGEWIDGETAUTOTEST_H

#include <QObject>

class KMessageWidgetTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testAnimationSignals();
    void testShowOnVisible();
    void testHideOnInvisible();
    void testOverlappingShowAndHide_data();
    void testOverlappingShowAndHide();
    void testOverlappingHideAndShow_data();
    void testOverlappingHideAndShow();
    void testOverlappingDoubleShow_data();
    void testOverlappingDoubleShow();
    void testOverlappingDoubleHide_data();
    void testOverlappingDoubleHide();
    void testHideWithNotYetShownParent();
    void testNonAnimatedShowAfterAnimatedHide();
    void testResizeFlickerOnAnimatedShow();
};

#endif
