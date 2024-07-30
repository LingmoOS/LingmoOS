/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Tim D. Gilman <tdgilman@best.org>
    SPDX-FileCopyrightText: 1998-2001 Mirko Boehm <mirko@kde.org>
    SPDX-FileCopyrightText: 2007 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPOPUPFRAME_H
#define KPOPUPFRAME_H

#include <kwidgetsaddons_export.h>

#include <QFrame>
#include <memory>

/**
 * @class KPopupFrame kpopupframe.h KPopupFrame
 *
 * Frame with popup menu behavior.
 * @author Tim Gilman, Mirko Boehm
 */
class KWIDGETSADDONS_EXPORT KPopupFrame : public QFrame
{
    Q_OBJECT
protected:
    /**
     * Catch key press events.
     */
    void keyPressEvent(QKeyEvent *e) override;

    /**
     * Catch hide events.
     */
    void hideEvent(QHideEvent *e) override;

public Q_SLOTS:
    /**
     * Close the popup window. This is called from the main widget, usually.
     *
     * @param r is the result returned from exec()
     */
    void close(int r);

public:
    /**
     * The constructor. Creates a dialog without buttons.
     */
    KPopupFrame(QWidget *parent = nullptr);

    /**
     * The destructor
     */
    ~KPopupFrame() override;

    /**
     * Set the main widget. You cannot set the main widget from the constructor,
     * since it must be a child of the frame itself.
     * Be careful: the size is set to the main widgets size. It is up to you to
     * set the main widgets correct size before setting it as the main
     * widget.
     */
    void setMainWidget(QWidget *m);

    /**
     * The resize event. Simply resizes the main widget to the whole
     * widgets client size.
     */
    void resizeEvent(QResizeEvent *resize) override;

    /**
     * Open the popup window at position pos.
     */
    void popup(const QPoint &pos);

    /**
     * Execute the popup window.
     */
    int exec(const QPoint &p);

    /**
     * Execute the popup window.
     */
    int exec(int x, int y);

Q_SIGNALS:
    void leaveModality();

private:
    friend class KPopupFramePrivate;
    std::unique_ptr<class KPopupFramePrivate> const d;

    Q_DISABLE_COPY(KPopupFrame)
};

#endif // KPOPUPFRAME_H
