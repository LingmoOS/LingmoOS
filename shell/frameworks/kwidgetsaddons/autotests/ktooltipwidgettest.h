/*
    SPDX-FileCopyrightText: 2017 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KTOOLTIPWIDGETTEST_H
#define KTOOLTIPWIDGETTEST_H

#include <QObject>
#include <QRect>

class KToolTipWidgetTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase();

    void showTooltipShouldShowContent();
    void hideLaterShouldHideAfterDelay();
    void setZeroDelayShouldHideImmediately();
    void shouldHideImmediatelyIfContentDestroyed();
    void shouldNotTakeOwnershipOfContent();

    void shouldNotObscureTarget_data();
    void shouldNotObscureTarget();

private:
    QRect m_screenGeometry;
    const int m_offset = 10;
};

#endif
