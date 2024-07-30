/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_DATASOURCE_H
#define WAYLAND_DATASOURCE_H

#include "buffer.h"
#include "datadevicemanager.h"

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_data_source;
class QMimeType;

namespace KWayland
{
namespace Client
{
/**
 * @short Wrapper for the wl_data_source interface.
 *
 * This class is a convenient wrapper for the wl_data_source interface.
 * To create a DataSource call DataDeviceManager::createDataSource.
 *
 * @see DataDeviceManager
 **/
class KWAYLANDCLIENT_EXPORT DataSource : public QObject
{
    Q_OBJECT
public:
    explicit DataSource(QObject *parent = nullptr);
    ~DataSource() override;

    /**
     * Setup this DataSource to manage the @p dataSource.
     * When using DataDeviceManager::createDataSource there is no need to call this
     * method.
     **/
    void setup(wl_data_source *dataSource);
    /**
     * Releases the wl_data_source interface.
     * After the interface has been released the DataSource instance is no
     * longer valid and can be setup with another wl_data_source interface.
     **/
    void release();
    /**
     * Destroys the data held by this DataSource.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_data_source interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DataSource gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a wl_data_source.
     **/
    bool isValid() const;

    void offer(const QString &mimeType);
    void offer(const QMimeType &mimeType);

    /**
     * Sets the actions that the source side client supports for this
     * operation.
     *
     * This request must be made once only, and can only be made on sources
     * used in drag-and-drop, so it must be performed before
     * @link{DataDevice::startDrag}. Attempting to use the source other than
     * for drag-and-drop will raise a protocol error.
     * @since 5.42
     **/
    void setDragAndDropActions(DataDeviceManager::DnDActions actions);

    /**
     * The currently selected drag and drop action by the compositor.
     * @see selectedDragAndDropActionChanged
     * @since 5.42
     **/
    DataDeviceManager::DnDAction selectedDragAndDropAction() const;

    operator wl_data_source *();
    operator wl_data_source *() const;

Q_SIGNALS:
    /**
     * Emitted when a target accepts pointer_focus or motion events. If
     * a target does not accept any of the offered types, @p mimeType is empty.
     **/
    void targetAccepts(const QString &mimeType);
    /**
     * Request for data from the client. Send the data as the
     * specified @p mimeType over the passed file descriptor @p fd, then close
     * it.
     **/
    void sendDataRequested(const QString &mimeType, qint32 fd);
    /**
     * This DataSource has been replaced by another DataSource.
     * The client should clean up and destroy this DataSource.
     **/
    void cancelled();

    /**
     * The drag-and-drop operation physically finished.
     *
     * The user performed the drop action. This signal does not
     * indicate acceptance, @link{cancelled} may still be
     * emitted afterwards if the drop destination does not accept any
     * mime type.
     *
     * However, this signal might not be received if the
     * compositor cancelled the drag-and-drop operation before this
     * signal could happen.
     *
     * Note that the DataSource may still be used in the future and
     * should not be destroyed here.
     * @since 5.42
     **/
    void dragAndDropPerformed();

    /**
     * The drag-and-drop operation concluded.
     *
     * The drop destination finished interoperating with this DataSource,
     * so the client is now free to destroy this DataSource.
     *
     * If the action used to perform the operation was "move", the
     * source can now delete the transferred data.
     * @since 5.42
     */
    void dragAndDropFinished();

    /**
     * Emitted whenever the selected drag and drop action changes.
     * @see selectedDragAndDropAction
     * @since 5.42
     **/
    void selectedDragAndDropActionChanged();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
