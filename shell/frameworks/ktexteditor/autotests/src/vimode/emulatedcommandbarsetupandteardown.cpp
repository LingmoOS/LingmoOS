/*
    This file is part of the KDE libraries and the Kate part.
    SPDX-FileCopyrightText: 2013-2016 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "emulatedcommandbarsetupandteardown.h"

#include <inputmode/kateviinputmode.h>
#include <kateconfig.h>
#include <kateview.h>
#include <vimode/emulatedcommandbar/emulatedcommandbar.h>

#include <QApplication>
#include <QMainWindow>
#include <QMetaObject>
#include <QTest>

// BEGIN: WindowKeepActive

WindowKeepActive::WindowKeepActive(QMainWindow *mainWindow)
    : m_mainWindow(mainWindow)
{
    /* There's nothing to do here. */
}

bool WindowKeepActive::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);

    if (event->type() == QEvent::WindowDeactivate) {
        // With some combinations of Qt and Xvfb, invoking/ dismissing a popup
        // will deactivate the m_mainWindow, preventing it from receiving shortcuts.
        // If we detect this, set it back to being the active window again.
        event->ignore();
        m_mainWindow->activateWindow();
        return true;
    }
    return false;
}

// END: WindowKeepActive

// BEGIN: EmulatedCommandBarSetUpAndTearDown

EmulatedCommandBarSetUpAndTearDown::EmulatedCommandBarSetUpAndTearDown(KateViInputMode *inputMode, KTextEditor::ViewPrivate *view, QMainWindow *window)
    : m_view(view)
    , m_window(window)
    , m_windowKeepActive(window)
    , m_viInputMode(inputMode)
{
    m_window->show();
    m_view->show();
    QApplication::setActiveWindow(m_window);
    m_view->setFocus();
    QApplication::processEvents();
    KateViewConfig::global()->setValue(KateViewConfig::ViInputModeStealKeys, true);
    m_window->installEventFilter(&m_windowKeepActive);

    (void)QTest::qWaitForWindowExposed(m_view);
    QTest::qWait(100); // For whatever reason needed
}
EmulatedCommandBarSetUpAndTearDown::~EmulatedCommandBarSetUpAndTearDown()
{
    m_window->removeEventFilter(&m_windowKeepActive);
    // Use invokeMethod to avoid having to export KateViewBar for testing.
    QMetaObject::invokeMethod(m_viInputMode->viModeEmulatedCommandBar(), "hideMe");
    m_view->hide();
    m_window->hide();
    KateViewConfig::global()->setValue(KateViewConfig::ViInputModeStealKeys, false);
    QApplication::processEvents();
}

// END: EmulatedCommandBarSetUpAndTearDown

#include "moc_emulatedcommandbarsetupandteardown.cpp"
