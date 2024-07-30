/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_PLASMAWINDOWMODEL_H
#define WAYLAND_PLASMAWINDOWMODEL_H

#include <QAbstractListModel>

#include "KWayland/Client/kwaylandclient_export.h"

namespace KWayland
{
namespace Client
{
class PlasmaWindowManagement;
class Surface;

/**
 * @short Exposes the window list and window state as a Qt item model.
 *
 * This class is a QAbstractListModel implementation that exposes information
 * from a PlasmaWindowManagement instance passed as parent and enables convenient
 * calls to PlasmaWindow methods through a model row index.
 *
 * The model is destroyed when the PlasmaWindowManagement parent is.
 *
 * The model resets when the PlasmaWindowManagement parent signals that its
 * interface is about to be destroyed.
 *
 * To use this class you can create an instance yourself, or preferably use the
 * convenience method in PlasmaWindowManagement:
 * @code
 * PlasmaWindowModel *model = wm->createWindowModel();
 * @endcode
 *
 * @see PlasmaWindowManagement
 * @see PlasmaWindow
 **/

class KWAYLANDCLIENT_EXPORT PlasmaWindowModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum AdditionalRoles {
        AppId = Qt::UserRole + 1,
        IsActive,
        IsFullscreenable,
        IsFullscreen,
        IsMaximizable,
        IsMaximized,
        IsMinimizable,
        IsMinimized,
        IsKeepAbove,
        IsKeepBelow,
        IsOnAllDesktops,
        IsDemandingAttention,
        SkipTaskbar,
        /**
         * @since 5.22
         */
        IsShadeable,
        /**
         * @since 5.22
         */
        IsShaded,
        /**
         * @since 5.22
         */
        IsMovable,
        /**
         * @since 5.22
         */
        IsResizable,
        /**
         * @since 5.22
         */
        IsVirtualDesktopChangeable,
        /**
         * @since 5.22
         */
        IsCloseable,
        /**
         * @since 5.25
         */
        Geometry,
        /**
         * @since 5.35
         */
        Pid,
        /**
         * @since 5.47
         */
        SkipSwitcher,
        /**
         * @since 5.53
         */
        VirtualDesktops,
        /**
         * @since 5.73
         */
        Uuid,
        LastRole,
    };
    Q_ENUM(AdditionalRoles)

    explicit PlasmaWindowModel(PlasmaWindowManagement *parent);
    ~PlasmaWindowModel() override;

    QHash<int, QByteArray> roleNames() const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Returns an index with internalPointer() pointing to a PlasmaWindow instance.
     **/
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;

    QMap<int, QVariant> itemData(const QModelIndex &index) const override;

    /**
     * Request the window at this model row index be activated.
     **/
    Q_INVOKABLE void requestActivate(int row);

    /**
     * Request the window at this model row index be closed.
     **/
    Q_INVOKABLE void requestClose(int row);

    /**
     * Request an interactive move for the window at this model row index.
     * @since 5.22
     **/
    Q_INVOKABLE void requestMove(int row);

    /**
     * Request an interactive resize for the window at this model row index.
     * @since 5.22
     **/
    Q_INVOKABLE void requestResize(int row);

    /**
     * Request the window at the model index @p row to be moved to the virtual desktop @p id.
     *
     * @since 5.90
     **/
    Q_INVOKABLE void requestEnterVirtualDesktop(int row, const QString &id);

    /**
     * Requests the window at this model row index have its keep above state toggled.
     * @since 5.35
     */
    Q_INVOKABLE void requestToggleKeepAbove(int row);

    /**
     * Requests the window at this model row index have its keep above state toggled.
     * @since 5.35
     */
    Q_INVOKABLE void requestToggleKeepBelow(int row);

    /**
     * Requests the window at this model row index have its minimized state toggled.
     */
    Q_INVOKABLE void requestToggleMinimized(int row);

    /**
     * Requests the window at this model row index have its maximized state toggled.
     */
    Q_INVOKABLE void requestToggleMaximized(int row);

    /**
     * Requests the window at this model row index have its fullscreen state toggled.
     * @since 6.0
     */
    Q_INVOKABLE void requestToggleFullscreen(int row);

    /**
     * Sets the geometry of the taskbar entry for the window at the model row
     * relative to a panel in particular. QRectF, intended for use from QML
     * @since 5.5
     */
    Q_INVOKABLE void setMinimizedGeometry(int row, Surface *panel, const QRect &geom);

    /**
     * Requests the window at this model row index have its shaded state toggled.
     * @since 5.22
     */
    Q_INVOKABLE void requestToggleShaded(int row);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
