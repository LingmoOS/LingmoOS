/*
    SPDX-FileCopyrightText: 2018 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

#include "kwin_export.h"

#include <QObject>
#include <memory>

namespace KWin
{
class Display;
class LingmoVirtualDesktopInterface;
class LingmoVirtualDesktopInterfacePrivate;
class LingmoVirtualDesktopManagementInterfacePrivate;

/**
 * @short Wrapper for the org_kde_lingmo_virtual_desktop_management interface.
 *
 * This class provides a convenient wrapper for the org_kde_lingmo_virtual_desktop_management interface.
 */
class KWIN_EXPORT LingmoVirtualDesktopManagementInterface : public QObject
{
    Q_OBJECT

public:
    explicit LingmoVirtualDesktopManagementInterface(Display *display, QObject *parent = nullptr);
    ~LingmoVirtualDesktopManagementInterface() override;

    /**
     * Sets how many rows the virtual desktops should be laid into
     */
    void setRows(quint32 rows);

    /**
     * @returns A desktop identified uniquely by this id.
     * If not found, nullptr will be returned.
     * @see createDesktop
     */
    LingmoVirtualDesktopInterface *desktop(const QString &id);

    /**
     * @returns A desktop identified uniquely by this id, if not found
     * a new desktop will be created for this id at a given position.
     * @param id the id for the desktop
     * @param position the position the desktop will be in, if not provided,
     *                 it will be appended at the end. If the desktop was already
     *                 existing, position is ignored.
     */
    LingmoVirtualDesktopInterface *createDesktop(const QString &id, quint32 position = std::numeric_limits<uint32_t>::max());

    /**
     * Removed and destroys the desktop identified by id, if present
     */
    void removeDesktop(const QString &id);

    /**
     * @returns All tghe desktops present.
     */
    QList<LingmoVirtualDesktopInterface *> desktops() const;

    /**
     * Inform the clients that all the properties have been sent, and
     * their client-side representation is complete.
     */
    void sendDone();

Q_SIGNALS:
    /**
     * A desktop has been activated
     */
    void desktopActivated(const QString &id);

    /**
     * The client asked to remove a desktop, It's responsibility of the server
     * deciding whether to remove it or not.
     */
    void desktopRemoveRequested(const QString &id);

    /**
     * The client asked to create a desktop, It's responsibility of the server
     * deciding whether to create it or not.
     * @param name The desired user readable name
     * @param position The desired position. Normalized, guaranteed to be in the range 0-count
     */
    void desktopCreateRequested(const QString &name, quint32 position);

private:
    std::unique_ptr<LingmoVirtualDesktopManagementInterfacePrivate> d;
};

class KWIN_EXPORT LingmoVirtualDesktopInterface : public QObject
{
    Q_OBJECT
public:
    ~LingmoVirtualDesktopInterface() override;

    /**
     * @returns the unique id for this desktop.
     * ids are set at creation time by LingmoVirtualDesktopManagementInterface::createDesktop
     * and can't be changed at runtime.
     */
    QString id() const;

    /**
     * Sets a new name for this desktop
     */
    void setName(const QString &name);

    /**
     * @returns the name for this desktop
     */
    QString name() const;

    /**
     * Set this desktop as active or not.
     * It's the compositor responsibility to manage the mutual exclusivity of active desktops.
     */
    void setActive(bool active);

    /**
     * @returns true if this desktop is active. Only one at a time will be.
     */
    bool isActive() const;

    /**
     * Inform the clients that all the properties have been sent, and
     * their client-side representation is complete.
     */
    void sendDone();

Q_SIGNALS:
    /**
     * Emitted when the client asked to activate this desktop:
     * it's the decision of the server whether to perform the activation or not.
     */
    void activateRequested();

private:
    LingmoVirtualDesktopInterface();
    friend class LingmoVirtualDesktopManagementInterface;
    friend class LingmoVirtualDesktopManagementInterfacePrivate;

    std::unique_ptr<LingmoVirtualDesktopInterfacePrivate> d;
};

}
