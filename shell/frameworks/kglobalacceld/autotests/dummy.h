/*
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2024 Yifan Zhu <fanzhuyifan@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DUMMY_H
#define DUMMY_H

#include "kglobalaccel_interface.h"

#include <QObject>

/**
 * @internal
 *
 * The KGlobalAccel private class handles grabbing of global keys,
 * and notification of when these keys are pressed.
 */
class KGlobalAccelImpl : public KGlobalAccelInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kglobalaccel5.KGlobalAccelInterface" FILE "dummy.json")
    Q_INTERFACES(KGlobalAccelInterface)

public:
    KGlobalAccelImpl(QObject *parent = nullptr);
    ~KGlobalAccelImpl() override;

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
    bool grabKey(int key, bool grab) override;

    /// Enable/disable all shortcuts. There will not be any grabbed shortcuts at this point.
    void setEnabled(bool) override;

    static KGlobalAccelImpl *instance();

public Q_SLOTS:
    bool checkKeyPressed(int keyQt);
    bool checkKeyReleased(int keyQt);
    bool checkPointerPressed(Qt::MouseButtons button);
    bool checkAxisTriggered(int axis);
};

#endif // DUMMY_H
