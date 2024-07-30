/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCURSOR_P_H
#define KCURSOR_P_H

#include <QCursor>
#include <QHash>
#include <QObject>
#include <QTimer>

class QWidget;

/**
 * @internal
 * I don't want the eventFilter to be in KCursor, so we have another class
 * for that stuff
 * @author John Firebaugh <jfirebaugh@kde.org>
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KCursorPrivateAutoHideEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit KCursorPrivateAutoHideEventFilter(QWidget *widget);
    ~KCursorPrivateAutoHideEventFilter() override;

    bool eventFilter(QObject *o, QEvent *e) override;

    void resetWidget();

private Q_SLOTS:
    void hideCursor();
    void unhideCursor();

private:
    QWidget *mouseWidget() const;

    QTimer m_autoHideTimer;
    QWidget *m_widget;
    bool m_wasMouseTracking;
    bool m_isCursorHidden;
    bool m_isOwnCursor;
    QCursor m_oldCursor;
};

/**
 * @internal
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @author John Firebaugh <jfirebaugh@kde.org>
 */
class KCursorPrivate : public QObject
{
    friend class KCursor; // to shut up the compiler
    Q_OBJECT

public:
    static KCursorPrivate *self();

    void setAutoHideCursor(QWidget *w, bool enable, bool customEventFilter);
    bool eventFilter(QObject *o, QEvent *e) override;

    int hideCursorDelay;

private Q_SLOTS:
    void slotViewportDestroyed(QObject *);
    void slotWidgetDestroyed(QObject *);

private:
    KCursorPrivate();
    ~KCursorPrivate() override;

    bool enabled;
    static KCursorPrivate *s_self;

    QHash<QObject *, KCursorPrivateAutoHideEventFilter *> m_eventFilters;
};

#endif // KCURSOR_PRIVATE_H
