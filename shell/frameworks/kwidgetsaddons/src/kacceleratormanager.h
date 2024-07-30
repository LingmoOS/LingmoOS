/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2002 Matthias Hölzer-Klüpfel <mhk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef K_ACCELERATORMANAGER_H
#define K_ACCELERATORMANAGER_H

#include <kwidgetsaddons_export.h>

#include <QStringList>

class QWidget;
class QString;

/**
 * @class KAcceleratorManager kacceleratormanager.h KAcceleratorManager
 *
 * KDE Accelerator manager.
 *
 * This class can be used to find a valid and working set of
 * accelerators for any widget.
 *
 * @author Matthias Hölzer-Klüpfel <mhk@kde.org>
 */
class KWIDGETSADDONS_EXPORT KAcceleratorManager
{
public:
    /**
     * Manages the accelerators of a widget.
     *
     * Call this function on the top widget of the hierarchy you
     * want to manage. It will fix the accelerators of the child widgets so
     * there are never duplicate accelerators. It also tries to put
     * accelerators on as many widgets as possible.
     *
     * The algorithm used tries to take the existing accelerators into
     * account, as well as the class of each widget. Hopefully, the result
     * is close to what you would assign manually.
     *
     * QMenu's are managed dynamically, so when you add or remove entries,
     * the accelerators are reassigned. If you add or remove widgets to your
     * toplevel widget, you will have to call manage again to fix the
     * accelerators.
     *
     * @param widget The toplevel widget you want to manage.
     * @param programmers_mode if true, KAcceleratorManager adds (&) for removed
     *             accels and & before added accels
     */

    static void manage(QWidget *widget, bool programmers_mode = false);

    /** \internal returns the result of the last manage operation. */
    static void last_manage(QString &added, QString &changed, QString &removed);

    /**
     * Use this method for a widget (and its children) you want no accels to be set on.
     */
    static void setNoAccel(QWidget *widget);

    /**
     * Append names to a list of standard action names.
     * These strings will be given higher priority when assigning keyboard accelerators.
     * @since 5.0
     */
    static void addStandardActionNames(const QStringList &names);
};

#endif // K_ACCELERATORMANAGER_H
