/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "plasmawindowmodel.h"
#include "plasmawindowmanagement.h"

#include <QMetaEnum>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN PlasmaWindowModel::Private
{
public:
    Private(PlasmaWindowModel *q);
    QList<PlasmaWindow *> windows;
    PlasmaWindow *window = nullptr;

    void addWindow(PlasmaWindow *window);
    void dataChanged(PlasmaWindow *window, int role);

private:
    PlasmaWindowModel *q;
};

PlasmaWindowModel::Private::Private(PlasmaWindowModel *q)
    : q(q)
{
}

void PlasmaWindowModel::Private::addWindow(PlasmaWindow *window)
{
    if (windows.indexOf(window) != -1) {
        return;
    }

    const int count = windows.count();
    q->beginInsertRows(QModelIndex(), count, count);
    windows.append(window);
    q->endInsertRows();

    auto removeWindow = [window, this] {
        const int row = windows.indexOf(window);
        if (row != -1) {
            q->beginRemoveRows(QModelIndex(), row, row);
            windows.removeAt(row);
            q->endRemoveRows();
        }
    };

    QObject::connect(window, &PlasmaWindow::unmapped, q, removeWindow);
    QObject::connect(window, &QObject::destroyed, q, removeWindow);

    QObject::connect(window, &PlasmaWindow::titleChanged, q, [window, this] {
        this->dataChanged(window, Qt::DisplayRole);
    });

    QObject::connect(window, &PlasmaWindow::iconChanged, q, [window, this] {
        this->dataChanged(window, Qt::DecorationRole);
    });

    QObject::connect(window, &PlasmaWindow::appIdChanged, q, [window, this] {
        this->dataChanged(window, PlasmaWindowModel::AppId);
    });

    QObject::connect(window, &PlasmaWindow::activeChanged, q, [window, this] {
        this->dataChanged(window, IsActive);
    });

    QObject::connect(window, &PlasmaWindow::fullscreenableChanged, q, [window, this] {
        this->dataChanged(window, IsFullscreenable);
    });

    QObject::connect(window, &PlasmaWindow::fullscreenChanged, q, [window, this] {
        this->dataChanged(window, IsFullscreen);
    });

    QObject::connect(window, &PlasmaWindow::maximizeableChanged, q, [window, this] {
        this->dataChanged(window, IsMaximizable);
    });

    QObject::connect(window, &PlasmaWindow::maximizedChanged, q, [window, this] {
        this->dataChanged(window, IsMaximized);
    });

    QObject::connect(window, &PlasmaWindow::minimizeableChanged, q, [window, this] {
        this->dataChanged(window, IsMinimizable);
    });

    QObject::connect(window, &PlasmaWindow::minimizedChanged, q, [window, this] {
        this->dataChanged(window, IsMinimized);
    });

    QObject::connect(window, &PlasmaWindow::keepAboveChanged, q, [window, this] {
        this->dataChanged(window, IsKeepAbove);
    });

    QObject::connect(window, &PlasmaWindow::keepBelowChanged, q, [window, this] {
        this->dataChanged(window, IsKeepBelow);
    });

    QObject::connect(window, &PlasmaWindow::onAllDesktopsChanged, q, [window, this] {
        this->dataChanged(window, IsOnAllDesktops);
    });

    QObject::connect(window, &PlasmaWindow::demandsAttentionChanged, q, [window, this] {
        this->dataChanged(window, IsDemandingAttention);
    });

    QObject::connect(window, &PlasmaWindow::skipTaskbarChanged, q, [window, this] {
        this->dataChanged(window, SkipTaskbar);
    });

    QObject::connect(window, &PlasmaWindow::skipSwitcherChanged, q, [window, this] {
        this->dataChanged(window, SkipSwitcher);
    });

    QObject::connect(window, &PlasmaWindow::shadeableChanged, q, [window, this] {
        this->dataChanged(window, IsShadeable);
    });

    QObject::connect(window, &PlasmaWindow::shadedChanged, q, [window, this] {
        this->dataChanged(window, IsShaded);
    });

    QObject::connect(window, &PlasmaWindow::movableChanged, q, [window, this] {
        this->dataChanged(window, IsMovable);
    });

    QObject::connect(window, &PlasmaWindow::resizableChanged, q, [window, this] {
        this->dataChanged(window, IsResizable);
    });

    QObject::connect(window, &PlasmaWindow::virtualDesktopChangeableChanged, q, [window, this] {
        this->dataChanged(window, IsVirtualDesktopChangeable);
    });

    QObject::connect(window, &PlasmaWindow::closeableChanged, q, [window, this] {
        this->dataChanged(window, IsCloseable);
    });

    QObject::connect(window, &PlasmaWindow::geometryChanged, q, [window, this] {
        this->dataChanged(window, Geometry);
    });

    QObject::connect(window, &PlasmaWindow::plasmaVirtualDesktopEntered, q, [window, this] {
        this->dataChanged(window, VirtualDesktops);
    });

    QObject::connect(window, &PlasmaWindow::plasmaVirtualDesktopLeft, q, [window, this] {
        this->dataChanged(window, VirtualDesktops);
    });
}

void PlasmaWindowModel::Private::dataChanged(PlasmaWindow *window, int role)
{
    QModelIndex idx = q->index(windows.indexOf(window));
    Q_EMIT q->dataChanged(idx, idx, QList<int>() << role);
}

PlasmaWindowModel::PlasmaWindowModel(PlasmaWindowManagement *parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
    connect(parent, &PlasmaWindowManagement::interfaceAboutToBeReleased, this, [this] {
        beginResetModel();
        d->windows.clear();
        endResetModel();
    });

    connect(parent, &PlasmaWindowManagement::windowCreated, this, [this](PlasmaWindow *window) {
        d->addWindow(window);
    });

    for (auto it = parent->windows().constBegin(); it != parent->windows().constEnd(); ++it) {
        d->addWindow(*it);
    }
}

PlasmaWindowModel::~PlasmaWindowModel()
{
}

QHash<int, QByteArray> PlasmaWindowModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles.insert(Qt::DisplayRole, "display");
    roles.insert(Qt::DecorationRole, "decoration");

    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("AdditionalRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    return roles;
}

QVariant PlasmaWindowModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= d->windows.count()) {
        return QVariant();
    }

    const PlasmaWindow *window = d->windows.at(index.row());

    if (role == Qt::DisplayRole) {
        return window->title();
    } else if (role == Qt::DecorationRole) {
        return window->icon();
    } else if (role == AppId) {
        return window->appId();
    } else if (role == Pid) {
        return window->pid();
    } else if (role == IsActive) {
        return window->isActive();
    } else if (role == IsFullscreenable) {
        return window->isFullscreenable();
    } else if (role == IsFullscreen) {
        return window->isFullscreen();
    } else if (role == IsMaximizable) {
        return window->isMaximizeable();
    } else if (role == IsMaximized) {
        return window->isMaximized();
    } else if (role == IsMinimizable) {
        return window->isMinimizeable();
    } else if (role == IsMinimized) {
        return window->isMinimized();
    } else if (role == IsKeepAbove) {
        return window->isKeepAbove();
    } else if (role == IsKeepBelow) {
        return window->isKeepBelow();
    } else if (role == IsOnAllDesktops) {
        return window->isOnAllDesktops();
    } else if (role == IsDemandingAttention) {
        return window->isDemandingAttention();
    } else if (role == SkipTaskbar) {
        return window->skipTaskbar();
    } else if (role == SkipSwitcher) {
        return window->skipSwitcher();
    } else if (role == IsShadeable) {
        return window->isShadeable();
    } else if (role == IsShaded) {
        return window->isShaded();
    } else if (role == IsMovable) {
        return window->isMovable();
    } else if (role == IsResizable) {
        return window->isResizable();
    } else if (role == IsVirtualDesktopChangeable) {
        return window->isVirtualDesktopChangeable();
    } else if (role == IsCloseable) {
        return window->isCloseable();
    } else if (role == Geometry) {
        return window->geometry();
    } else if (role == VirtualDesktops) {
        return window->plasmaVirtualDesktops();
    } else if (role == Uuid) {
        return window->uuid();
    }

    return QVariant();
}
QMap<int, QVariant> PlasmaWindowModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> ret = QAbstractItemModel::itemData(index);
    for (int role = AppId; role < LastRole; ++role) {
        ret.insert(role, data(index, role));
    }
    return ret;
}

int PlasmaWindowModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->windows.count();
}

QModelIndex PlasmaWindowModel::index(int row, int column, const QModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column, d->windows.at(row)) : QModelIndex();
}

Q_INVOKABLE void PlasmaWindowModel::requestActivate(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestActivate();
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestClose(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestClose();
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestMove(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestMove();
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestResize(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestResize();
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestEnterVirtualDesktop(int row, const QString &id)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestEnterVirtualDesktop(id);
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestToggleKeepAbove(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestToggleKeepAbove();
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestToggleKeepBelow(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestToggleKeepBelow();
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestToggleMinimized(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestToggleMinimized();
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestToggleMaximized(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestToggleMaximized();
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestToggleFullscreen(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestToggleFullscreen();
    }
}

Q_INVOKABLE void PlasmaWindowModel::setMinimizedGeometry(int row, Surface *panel, const QRect &geom)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->setMinimizedGeometry(panel, geom);
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestToggleShaded(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestToggleShaded();
    }
}

}
}

#include "moc_plasmawindowmodel.cpp"
