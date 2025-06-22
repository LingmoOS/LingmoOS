/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Teo Mrnjavac <teo@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kaboutapplicationlistview_p.h"

#include <QScrollBar>

namespace KDEPrivate
{
KAboutApplicationListView::KAboutApplicationListView(QWidget *parent)
    : QListView(parent)
{
    setVerticalScrollMode(ScrollPerPixel);
    setFrameShape(QFrame::NoFrame);

    QPalette p = palette();
    QColor c = p.color(QPalette::Base);
    c.setAlpha(0);
    p.setColor(QPalette::Base, c);
    setBackgroundRole(QPalette::Base);
    setPalette(p);
    setSelectionMode(NoSelection);
    setEditTriggers(NoEditTriggers);
}

void KAboutApplicationListView::wheelEvent(QWheelEvent *e)
{
    // HACK: Workaround for Qt bug 7232: Smooth scrolling (scroll per pixel) in ItemViews
    //      does not work as expected.
    verticalScrollBar()->setSingleStep(3);
    QListView::wheelEvent(e);
}

} // namespace KDEPrivate
