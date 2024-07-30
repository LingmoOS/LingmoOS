/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_DATAOFFER_H
#define WAYLAND_DATAOFFER_H

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

#include "datadevicemanager.h"

struct wl_data_offer;

class QMimeType;

namespace KWayland
{
namespace Client
{
class DataDevice;

/**
 * @short Wrapper for the wl_data_offer interface.
 *
 * This class is a convenient wrapper for the wl_data_offer interface.
 * The DataOffer gets created by DataDevice.
 *
 * @see DataOfferManager
 **/
class KWAYLANDCLIENT_EXPORT DataOffer : public QObject
{
    Q_OBJECT
public:
    ~DataOffer() override;

    /**
     * Releases the wl_data_offer interface.
     * After the interface has been released the DataOffer instance is no
     * longer valid and can be setup with another wl_data_offer interface.
     **/
    void release();
    /**
     * Destroys the data held by this DataOffer.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_data_offer interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DataOffer gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a wl_data_offer.
     **/
    bool isValid() const;

    QList<QMimeType> offeredMimeTypes() const;

    /**
     * Indicates that the client can accept data of the given @a mimeType.
     * The @a serial parameter specifies the serial number of the corresponding
     * dragEntered() event.
     *
     * @see DataDevice::dragEntered
     */
    void accept(const QMimeType &mimeType, quint32 serial);
    /**
     * Indicates that the client can accept data of the given @a mimeType.
     * The @a serial parameter specifies the serial number of the corresponding
     * dragEntered() event.
     *
     * @see DataDevice::dragEntered
     */
    void accept(const QString &mimeType, quint32 serial);

    void receive(const QMimeType &mimeType, qint32 fd);
    void receive(const QString &mimeType, qint32 fd);

    /**
     * Notifies the compositor that the drag destination successfully
     * finished the drag-and-drop operation.
     *
     * After this operation it is only allowed to release the DataOffer.
     *
     * @since 5.42
     **/
    void dragAndDropFinished();

    /**
     * The actions offered by the DataSource.
     * @since 5.42
     * @see sourceDragAndDropActionsChanged
     **/
    DataDeviceManager::DnDActions sourceDragAndDropActions() const;

    /**
     * Sets the @p supported and @p preferred Drag and Drop actions.
     * @since 5.42
     **/
    void setDragAndDropActions(DataDeviceManager::DnDActions supported, DataDeviceManager::DnDAction preferred);

    /**
     * The currently selected drag and drop action by the compositor.
     * @see selectedDragAndDropActionChanged
     * @since 5.42
     **/
    DataDeviceManager::DnDAction selectedDragAndDropAction() const;

    operator wl_data_offer *();
    operator wl_data_offer *() const;

Q_SIGNALS:
    void mimeTypeOffered(const QString &);
    /**
     * Emitted whenever the @link{sourceDragAndDropActions} changed, e.g. on enter or when
     * the DataSource changes the supported actions.
     * @see sourceDragAndDropActions
     * @since 5.42
     **/
    void sourceDragAndDropActionsChanged();
    /**
     * Emitted whenever the selected drag and drop action changes.
     * @see selectedDragAndDropAction
     * @since 5.42
     **/
    void selectedDragAndDropActionChanged();

private:
    friend class DataDevice;
    explicit DataOffer(DataDevice *parent, wl_data_offer *dataOffer);
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::DataOffer *)

#endif
