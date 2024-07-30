/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KGLOBALACCEL_INTERFACE_H
#define KGLOBALACCEL_INTERFACE_H

#include <QObject>

#include "kglobalacceld_export.h"

class GlobalShortcutsRegistry;

#define KGlobalAccelInterface_iid "org.kde.kglobalaccel5.KGlobalAccelInterface"

/**
 * Abstract interface for plugins to implement
 */
class KGLOBALACCELD_EXPORT KGlobalAccelInterface : public QObject
{
    Q_OBJECT

public:
    explicit KGlobalAccelInterface(QObject *parent);
    ~KGlobalAccelInterface() override;

public:
    /**
     * This function registers or unregisters a certain key for global capture,
     * depending on \b grab.
     *
     * Before destruction, every grabbed key will be released, so this
     * object does not need to do any tracking.
     *
     * \param key the Qt keycode to grab or release.
     * \param grab true to grab they key, false to release the key.
     *
     * \return true if successful, otherwise false.
     */
    virtual bool grabKey(int key, bool grab) = 0;

    /*
     * Enable/disable all shortcuts. There will not be any grabbed shortcuts at this point.
     */
    virtual void setEnabled(bool) = 0;

    void setRegistry(GlobalShortcutsRegistry *registry);

protected:
    /**
     * called by the implementation to inform us about key presses
     * @returns @c true if the key was handled
     **/
    bool keyPressed(int keyQt);
    void grabKeys();
    void ungrabKeys();
    /**
     * Called by the implementation to inform us about key releases
     *
     * @param keyQt the key that was just released
     *
     * @returns @c true if the key was handled
     **/
    bool keyReleased(int keyQt);
    /**
     * Called by the implementation to inform us about pointer presses
     * Currently only used for clearing modifier only shortcuts
     *
     * @param buttons the buttons that were pressed
     *
     * @returns @c true if the key was handled
     */
    bool pointerPressed(Qt::MouseButtons buttons);
    /**
     * Called by the implementation to inform us about pointer axis events
     * Currently only used for clearing modifier only shortcuts
     *
     * @param axis the axis that was triggered
     *
     * @returns @c true if the key was handled
     */
    bool axisTriggered(int axis);

    class Private;
    QScopedPointer<Private> d;
};

Q_DECLARE_INTERFACE(KGlobalAccelInterface, KGlobalAccelInterface_iid)

#endif
