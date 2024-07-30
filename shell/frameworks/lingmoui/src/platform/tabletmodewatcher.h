/*
 *  SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef LINGMOUI_TABLETMODEWATCHER_H
#define LINGMOUI_TABLETMODEWATCHER_H

#include <QEvent>
#include <QObject>

#include "lingmouiplatform_export.h"

namespace LingmoUI
{
namespace Platform
{
class TabletModeWatcherPrivate;

class LINGMOUIPLATFORM_EXPORT TabletModeChangedEvent : public QEvent
{
public:
    TabletModeChangedEvent(bool tablet)
        : QEvent(TabletModeChangedEvent::type)
        , tabletMode(tablet)
    {
    }

    bool tabletMode = false;

    inline static QEvent::Type type = QEvent::None;
};

/**
 * @class TabletModeWatcher tabletmodewatcher.h <LingmoUI/TabletModeWatcher>
 *
 * This class reports on the status of certain transformable
 * devices which can be both tablets and laptops at the same time,
 * with a detachable keyboard.
 * It reports whether the device supports a tablet mode and if
 * the device is currently in such mode or not, emitting a signal
 * when the user switches.
 */
class LINGMOUIPLATFORM_EXPORT TabletModeWatcher : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool tabletModeAvailable READ isTabletModeAvailable NOTIFY tabletModeAvailableChanged FINAL)
    Q_PROPERTY(bool tabletMode READ isTabletMode NOTIFY tabletModeChanged FINAL)

public:
    ~TabletModeWatcher() override;
    static TabletModeWatcher *self();

    /**
     * @returns true if the device supports a tablet mode and has a switch
     * to report when the device has been transformed.
     * For debug purposes, if either the environment variable QT_QUICK_CONTROLS_MOBILE
     * or KDE_LINGMOUI_TABLET_MODE are set to true, isTabletModeAvailable will be true
     */
    bool isTabletModeAvailable() const;

    /**
     * @returns true if the machine is now in tablet mode, such as the
     * laptop keyboard flipped away or detached.
     * Note that this doesn't mean exactly a tablet form factor, but
     * that the preferred input mode for the device is the touch screen
     * and that pointer and keyboard are either secondary or not available.
     *
     * For debug purposes, if either the environment variable QT_QUICK_CONTROLS_MOBILE
     * or KDE_LINGMOUI_TABLET_MODE are set to true, isTabletMode will be true
     */
    bool isTabletMode() const;

    /**
     * Register an arbitrary QObject to send events from this.
     * At the moment only one event will be sent: TabletModeChangedEvent
     */
    void addWatcher(QObject *watcher);

    /*
     * Unsubscribe watcher from receiving events from TabletModeWatcher.
     */
    void removeWatcher(QObject *watcher);

Q_SIGNALS:
    void tabletModeAvailableChanged(bool tabletModeAvailable);
    void tabletModeChanged(bool tabletMode);

private:
    LINGMOUIPLATFORM_NO_EXPORT explicit TabletModeWatcher(QObject *parent = nullptr);
    TabletModeWatcherPrivate *d;
    friend class TabletModeWatcherSingleton;
};

}
}

#endif // LINGMOUI_TABLETMODEWATCHER
