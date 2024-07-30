/*
    This file is part of the KDE libraries and the Kate part.
    SPDX-FileCopyrightText: 2013-2016 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EMULATEDCOMMANDBARSETUPANDTEARDOWN_H
#define EMULATEDCOMMANDBARSETUPANDTEARDOWN_H

#include <QObject>

namespace KTextEditor
{
class ViewPrivate;
}

class KateViInputMode;
class QMainWindow;

/**
 * This class is used by the EmulatedCommandBarSetUpAndTearDown class so
 * the main window is active all the time.
 */
class WindowKeepActive : public QObject
{
    Q_OBJECT

public:
    explicit WindowKeepActive(QMainWindow *mainWindow);

public Q_SLOTS:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QMainWindow *m_mainWindow;
};

/**
 * Helper class that is used to setup and tear down tests affecting
 * the command bar in any way.
 */
class EmulatedCommandBarSetUpAndTearDown
{
public:
    EmulatedCommandBarSetUpAndTearDown(KateViInputMode *inputMode, KTextEditor::ViewPrivate *view, QMainWindow *window);

    ~EmulatedCommandBarSetUpAndTearDown();

private:
    KTextEditor::ViewPrivate *m_view;
    QMainWindow *m_window;
    WindowKeepActive m_windowKeepActive;
    KateViInputMode *m_viInputMode;
};

#endif
