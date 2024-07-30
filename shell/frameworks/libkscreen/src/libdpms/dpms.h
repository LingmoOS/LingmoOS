// SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
// SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QScreen>

#include "kscreendpms_export.h"

namespace KScreen
{

class AbstractDpmsHelper;

/**
 * @class Dpms, allows controlling the system's Display Power Management Signaling
 *
 * Provides an API to switch the system's mode on a per-display basis.
 *
 * It has backends for X11 and Wayland.
 */
class KSCREENDPMS_EXPORT Dpms : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isSupported READ isSupported NOTIFY supportedChanged)
    Q_PROPERTY(bool hasPendingChanges READ hasPendingChanges NOTIFY hasPendingChangesChanged)

public:
    explicit Dpms(QObject *parent = nullptr);
    ~Dpms() override;

    enum Mode {
        On,
        Standby,
        Suspend,
        Off,
        Toggle,
    };
    Q_ENUM(Mode)

    /**
     * @returns true if the DPMS system is supported
     */
    bool isSupported() const;

    /**
     * @returns true if there still are pending DPMS changes
     * This would happen after @m switchMode is called as most implementations will be async.
     */
    bool hasPendingChanges() const;

    /**
     * Switches the @p screens to @p mode
     *
     * If @p screens is empty, it will use all the screens as returned by QGuiApplication::screens()
     */
    Q_SCRIPTABLE void switchMode(Mode mode, const QList<QScreen *> &screen = {});

Q_SIGNALS:
    /**
     * Notifies about the class being ready for usage
     */
    void supportedChanged(bool supported);

    /**
     * Tells which is the new @p mode that the @p screen just adopted
     */
    void modeChanged(Mode mode, QScreen *screen);

    void hasPendingChangesChanged(bool hasPendingChanges);

private:
    QScopedPointer<AbstractDpmsHelper> m_helper;
};

} // namespace
