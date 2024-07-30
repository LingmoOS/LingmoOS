/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 Daniel Calviño Sánchez <danxuliu@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSELECTACTION_UNITTEST_H
#define KSELECTACTION_UNITTEST_H

#include <QObject>

class KSelectAction_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    // The next 8 are from bug 205293.
    void testSetToolTipBeforeRequestingComboBoxWidget();
    void testSetToolTipAfterRequestingComboBoxWidget();
    void testSetToolTipBeforeRequestingToolButtonWidget();
    void testSetToolTipAfterRequestingToolButtonWidget();

    void testSetWhatsThisBeforeRequestingComboBoxWidget();
    void testSetWhatsThisAfterRequestingComboBoxWidget();
    void testSetWhatsThisBeforeRequestingToolButtonWidget();
    void testSetWhatsThisAfterRequestingToolButtonWidget();

    // Test for the eventFilter code.
    void testChildActionStateChangeComboMode();

    void testCrashComboBoxDestruction();

    // The next 6 are from bug 203114.
    void testRequestWidgetComboBoxModeWidgetParent();
    void testRequestWidgetComboBoxModeWidgetParentSeveralActions();

    void testRequestWidgetMenuModeWidgetParent();
    void testRequestWidgetMenuModeWidgetParentSeveralActions();
    void testRequestWidgetMenuModeWidgetParentAddActions();
    void testRequestWidgetMenuModeWidgetParentRemoveActions();
};

#endif
