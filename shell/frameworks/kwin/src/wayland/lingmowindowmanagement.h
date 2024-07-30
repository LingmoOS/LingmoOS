/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

#include "kwin_export.h"

#include <QObject>
#include <memory>

class QSize;

namespace KWin
{
class Display;
class OutputInterface;
class LingmoWindowActivationFeedbackInterfacePrivate;
class LingmoWindowInterface;
class LingmoVirtualDesktopManagementInterface;
class LingmoWindowActivationInterfacePrivate;
class LingmoWindowManagementInterfacePrivate;
class LingmoWindowInterfacePrivate;
class SurfaceInterface;

class KWIN_EXPORT LingmoWindowActivationInterface
{
public:
    ~LingmoWindowActivationInterface();

    void sendAppId(const QString &id);

private:
    friend class LingmoWindowActivationFeedbackInterface;
    explicit LingmoWindowActivationInterface();

    std::unique_ptr<LingmoWindowActivationInterfacePrivate> d;
};

class KWIN_EXPORT LingmoWindowActivationFeedbackInterface : public QObject
{
    Q_OBJECT

public:
    explicit LingmoWindowActivationFeedbackInterface(Display *display, QObject *parent = nullptr);
    ~LingmoWindowActivationFeedbackInterface() override;

    /**
     * Notify about a new application with @p app_id being started
     *
     * @returns an instance of @class LingmoWindowActivationInterface to
     * be destroyed as the activation process ends.
     */
    std::unique_ptr<LingmoWindowActivationInterface> createActivation(const QString &app_id);

private:
    std::unique_ptr<LingmoWindowActivationFeedbackInterfacePrivate> d;
};

class KWIN_EXPORT LingmoWindowManagementInterface : public QObject
{
    Q_OBJECT

public:
    explicit LingmoWindowManagementInterface(Display *display, QObject *parent = nullptr);
    ~LingmoWindowManagementInterface() override;
    enum class ShowingDesktopState {
        Disabled,
        Enabled,
    };
    void setShowingDesktopState(ShowingDesktopState state);

    LingmoWindowInterface *createWindow(QObject *parent, const QUuid &uuid);
    QList<LingmoWindowInterface *> windows() const;

    /**
     * Associate a LingmoVirtualDesktopManagementInterface to this window management.
     * It's necessary to associate one in orderto use the lingmo virtual desktop features
     * of LingmoWindowInterface, as a window must know what are the deasktops available
     */
    void setLingmoVirtualDesktopManagementInterface(LingmoVirtualDesktopManagementInterface *manager);

    /**
     * @returns the LingmoVirtualDesktopManagementInterface associated to this LingmoWindowManagementInterface
     */
    LingmoVirtualDesktopManagementInterface *lingmoVirtualDesktopManagementInterface() const;

    /**
     * Associate stacking order to this window management
     */
    void setStackingOrder(const QList<quint32> &stackingOrder);

    void setStackingOrderUuids(const QList<QString> &stackingOrderUuids);

Q_SIGNALS:
    void requestChangeShowingDesktop(ShowingDesktopState requestedState);

private:
    std::unique_ptr<LingmoWindowManagementInterfacePrivate> d;
};

/**
 * @todo Add documentation
 */
class KWIN_EXPORT LingmoWindowInterface : public QObject
{
    Q_OBJECT
public:
    ~LingmoWindowInterface() override;

    void setTitle(const QString &title);
    void setAppId(const QString &appId);
    void setPid(quint32 pid);
    void setActive(bool set);
    void setMinimized(bool set);
    void setMaximized(bool set);
    void setFullscreen(bool set);
    void setKeepAbove(bool set);
    void setKeepBelow(bool set);
    void setOnAllDesktops(bool set);
    void setDemandsAttention(bool set);
    void setCloseable(bool set);
    void setMinimizeable(bool set);
    void setMaximizeable(bool set);
    void setFullscreenable(bool set);
    void setSkipTaskbar(bool skip);
    void setSkipSwitcher(bool skip);
    void setShadeable(bool set);
    void setShaded(bool set);
    void setMovable(bool set);
    void setResizable(bool set);
    void setResourceName(const QString &resourceName);
    /**
     * FIXME: still relevant with new desktops?
     */
    void setVirtualDesktopChangeable(bool set);

    /**
     * This method removes the Window and the Client is supposed to release the resource
     * bound for this Window.
     *
     * No more events should be sent afterwards.
     */
    void unmap();

    /**
     * @returns Geometries of the taskbar entries, indicized by the
     *          surface of the panels
     */
    QHash<SurfaceInterface *, QRect> minimizedGeometries() const;

    /**
     * Sets this LingmoWindowInterface as a transient window to @p parentWindow.
     * If @p parentWindow is @c nullptr, the LingmoWindowInterface is a toplevel
     * window and does not have a parent window.
     */
    void setParentWindow(LingmoWindowInterface *parentWindow);

    /**
     * Sets the window @p geometry of this LingmoWindow.
     *
     * @param geometry The geometry in absolute coordinates
     */
    void setGeometry(const QRect &geometry);

    /**
     * Set the icon of the LingmoWindowInterface.
     *
     * In case the icon has a themed name, only the name is sent to the client.
     * Otherwise the client is only informed that there is an icon and the client
     * can request the icon in an asynchronous way by passing a file descriptor
     * into which the icon will be serialized.
     *
     * @param icon The new icon
     */
    void setIcon(const QIcon &icon);

    /**
     * Adds a new desktop to this window: a window can be on
     * an arbitrary subset of virtual desktops.
     * If it's on none it will be considered on all desktops.
     */
    void addLingmoVirtualDesktop(const QString &id);

    /**
     * Removes a visrtual desktop from a window
     */
    void removeLingmoVirtualDesktop(const QString &id);

    /**
     * The ids of all the desktops currently associated with this window.
     * When a desktop is deleted it will be automatically removed from this list
     */
    QStringList lingmoVirtualDesktops() const;

    /**
     * Adds an activity to this window: a window can be on
     * an arbitrary subset of activities.
     * If it's on none it will be considered on all activities.
     */
    void addLingmoActivity(const QString &id);

    /**
     * Removes an activity from a window
     */
    void removeLingmoActivity(const QString &id);

    /**
     * The ids of all the activities currently associated with this window.
     * When an activity is deleted it will be automatically removed from this list
     */
    QStringList lingmoActivities() const;

    /**
     * Set the application menu D-BUS service name and object path for the window.
     */
    void setApplicationMenuPaths(const QString &serviceName, const QString &objectPath);

    /**
     * Return the window internal id
     */
    quint32 internalId() const;

    /**
     * @return a unique string that identifies this window
     */
    QString uuid() const;

    /**
     * Set the client geometry (i.e. without window border/style)
     */
    void setClientGeometry(const QRect &geometry);

Q_SIGNALS:
    void closeRequested();
    void moveRequested();
    void resizeRequested();
    void activeRequested(bool set);
    void minimizedRequested(bool set);
    void maximizedRequested(bool set);
    void fullscreenRequested(bool set);
    void keepAboveRequested(bool set);
    void keepBelowRequested(bool set);
    void demandsAttentionRequested(bool set);
    void closeableRequested(bool set);
    void minimizeableRequested(bool set);
    void maximizeableRequested(bool set);
    void fullscreenableRequested(bool set);
    void skipTaskbarRequested(bool set);
    void skipSwitcherRequested(bool set);
    QRect minimizedGeometriesChanged();
    void shadeableRequested(bool set);
    void shadedRequested(bool set);
    void movableRequested(bool set);
    void resizableRequested(bool set);
    /**
     * FIXME: still relevant with new virtual desktops?
     */
    void virtualDesktopChangeableRequested(bool set);

    /**
     * Emitted when the client wishes this window to enter in a new virtual desktop.
     * The server will decide whether to consent this request
     */
    void enterLingmoVirtualDesktopRequested(const QString &desktop);

    /**
     * Emitted when the client wishes this window to enter in
     * a new virtual desktop to be created for it.
     * The server will decide whether to consent this request
     */
    void enterNewLingmoVirtualDesktopRequested();

    /**
     * Emitted when the client wishes to remove this window from a virtual desktop.
     * The server will decide whether to consent this request
     */
    void leaveLingmoVirtualDesktopRequested(const QString &desktop);

    /**
     * Emitted when the client wishes this window to enter an activity.
     * The server will decide whether to consent this request
     */
    void enterLingmoActivityRequested(const QString &activity);

    /**
     * Emitted when the client wishes to remove this window from an activity.
     * The server will decide whether to consent this request
     */
    void leaveLingmoActivityRequested(const QString &activity);

    /**
     * Requests sending the window to @p output
     */
    void sendToOutput(KWin::OutputInterface *output);
    void clientGeometryChanged(const QRect &geometry);

private:
    friend class LingmoWindowManagementInterface;
    friend class LingmoWindowInterfacePrivate;
    friend class LingmoWindowManagementInterfacePrivate;
    explicit LingmoWindowInterface(LingmoWindowManagementInterface *wm, QObject *parent);

    std::unique_ptr<LingmoWindowInterfacePrivate> d;
};

}

Q_DECLARE_METATYPE(KWin::LingmoWindowManagementInterface::ShowingDesktopState)
