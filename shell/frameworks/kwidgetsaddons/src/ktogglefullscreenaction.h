/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KTOGGLEFULLSCREENACTION_H
#define KTOGGLEFULLSCREENACTION_H

#include <ktoggleaction.h>

class QWidget;

class KToggleFullScreenActionPrivate;

/**
 * @class KToggleFullScreenAction ktogglefullscreenaction.h KToggleFullScreenAction
 *
 * An action for switching between to/from full screen mode. Note that
 * QWidget::isFullScreen() may reflect the new or the old state
 * depending on how the action was triggered (by the application or
 * from the window manager). Also don't try to track the window state
 * yourself. Rely on this action's state (isChecked()) instead.
 *
 * Important: If you need to set/change the fullscreen state manually,
 * use KToggleFullScreenAction::setFullScreen() or a similar function,
 * do not call directly the slot connected to the toggled() signal. The slot
 * still needs to explicitly set the window state though.
 *
 * @note Do NOT use QWidget::showFullScreen() or QWidget::showNormal().
 * They have several side-effects besides just switching the fullscreen
 * state (for example, showNormal() resets all window states, not just
 * fullscreen). Use the KToggleFullScreenAction::setFullScreen() helper function.
 */
class KWIDGETSADDONS_EXPORT KToggleFullScreenAction : public KToggleAction
{
    Q_OBJECT

public:
    /**
     * Create a KToggleFullScreenAction. Call setWindow() to associate this
     * action with a window.
     *
     *  @param parent This action's parent object.
     */
    explicit KToggleFullScreenAction(QObject *parent);

    /**
     * Create a KToggleFullScreenAction
     *  @param window the window that will switch to/from full screen mode
     *  @param parent This action's parent object.
     */
    KToggleFullScreenAction(QWidget *window, QObject *parent);

    /**
     * Destroys the toggle fullscreen action.
     */
    ~KToggleFullScreenAction() override;

    /**
     * Sets the window that will be related to this action.
     */
    void setWindow(QWidget *window);

    /**
     * Helper function to set or reset the fullscreen state of a window.
     * Use this function rather than showFullScreen()/showNormal() QWidget functions.
     * @since 4.0.3
     */
    static void setFullScreen(QWidget *window, bool set);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

    // KF6 TODO: remove
protected Q_SLOTS:
    void slotToggled(bool checked) override;

private:
    Q_DECLARE_PRIVATE(KToggleFullScreenAction)
};

#endif
