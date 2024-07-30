/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "datasource.h"
#include "wayland_pointer_p.h"
// Qt
#include <QMimeType>
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN DataSource::Private
{
public:
    explicit Private(DataSource *q);
    void setup(wl_data_source *s);

    WaylandPointer<wl_data_source, wl_data_source_destroy> source;
    DataDeviceManager::DnDAction selectedAction = DataDeviceManager::DnDAction::None;

private:
    void setAction(DataDeviceManager::DnDAction action);
    static void targetCallback(void *data, wl_data_source *dataSource, const char *mimeType);
    static void sendCallback(void *data, wl_data_source *dataSource, const char *mimeType, int32_t fd);
    static void cancelledCallback(void *data, wl_data_source *dataSource);
    static void dndDropPerformedCallback(void *data, wl_data_source *wl_data_source);
    static void dndFinishedCallback(void *data, wl_data_source *wl_data_source);
    static void actionCallback(void *data, wl_data_source *wl_data_source, uint32_t dnd_action);

    static const struct wl_data_source_listener s_listener;

    DataSource *q;
};

const wl_data_source_listener DataSource::Private::s_listener =
    {targetCallback, sendCallback, cancelledCallback, dndDropPerformedCallback, dndFinishedCallback, actionCallback};

DataSource::Private::Private(DataSource *q)
    : q(q)
{
}

void DataSource::Private::targetCallback(void *data, wl_data_source *dataSource, const char *mimeType)
{
    auto d = reinterpret_cast<DataSource::Private *>(data);
    Q_ASSERT(d->source == dataSource);
    Q_EMIT d->q->targetAccepts(QString::fromUtf8(mimeType));
}

void DataSource::Private::sendCallback(void *data, wl_data_source *dataSource, const char *mimeType, int32_t fd)
{
    auto d = reinterpret_cast<DataSource::Private *>(data);
    Q_ASSERT(d->source == dataSource);
    Q_EMIT d->q->sendDataRequested(QString::fromUtf8(mimeType), fd);
}

void DataSource::Private::cancelledCallback(void *data, wl_data_source *dataSource)
{
    auto d = reinterpret_cast<DataSource::Private *>(data);
    Q_ASSERT(d->source == dataSource);
    Q_EMIT d->q->cancelled();
}

void DataSource::Private::dndDropPerformedCallback(void *data, wl_data_source *wl_data_source)
{
    Q_UNUSED(wl_data_source)
    auto d = reinterpret_cast<DataSource::Private *>(data);
    Q_EMIT d->q->dragAndDropPerformed();
}

void DataSource::Private::dndFinishedCallback(void *data, wl_data_source *wl_data_source)
{
    Q_UNUSED(wl_data_source)
    auto d = reinterpret_cast<DataSource::Private *>(data);
    Q_EMIT d->q->dragAndDropFinished();
}

void DataSource::Private::actionCallback(void *data, wl_data_source *wl_data_source, uint32_t dnd_action)
{
    Q_UNUSED(wl_data_source)
    auto d = reinterpret_cast<Private *>(data);
    switch (dnd_action) {
    case WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY:
        d->setAction(DataDeviceManager::DnDAction::Copy);
        break;
    case WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE:
        d->setAction(DataDeviceManager::DnDAction::Move);
        break;
    case WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK:
        d->setAction(DataDeviceManager::DnDAction::Ask);
        break;
    case WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE:
        d->setAction(DataDeviceManager::DnDAction::None);
        break;
    default:
        Q_UNREACHABLE();
    }
}

void DataSource::Private::setAction(DataDeviceManager::DnDAction action)
{
    if (action == selectedAction) {
        return;
    }
    selectedAction = action;
    Q_EMIT q->selectedDragAndDropActionChanged();
}

void DataSource::Private::setup(wl_data_source *s)
{
    Q_ASSERT(!source.isValid());
    Q_ASSERT(s);
    source.setup(s);
    wl_data_source_add_listener(s, &s_listener, this);
}

DataSource::DataSource(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DataSource::~DataSource()
{
    release();
}

void DataSource::release()
{
    d->source.release();
}

void DataSource::destroy()
{
    d->source.destroy();
}

bool DataSource::isValid() const
{
    return d->source.isValid();
}

void DataSource::setup(wl_data_source *dataSource)
{
    d->setup(dataSource);
}

void DataSource::offer(const QString &mimeType)
{
    wl_data_source_offer(d->source, mimeType.toUtf8().constData());
}

void DataSource::offer(const QMimeType &mimeType)
{
    if (!mimeType.isValid()) {
        return;
    }
    offer(mimeType.name());
}

DataSource::operator wl_data_source *() const
{
    return d->source;
}

DataSource::operator wl_data_source *()
{
    return d->source;
}

void DataSource::setDragAndDropActions(DataDeviceManager::DnDActions actions)
{
    uint32_t wlActions = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
    if (actions.testFlag(DataDeviceManager::DnDAction::Copy)) {
        wlActions |= WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
    }
    if (actions.testFlag(DataDeviceManager::DnDAction::Move)) {
        wlActions |= WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
    }
    if (actions.testFlag(DataDeviceManager::DnDAction::Ask)) {
        wlActions |= WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK;
    }
    wl_data_source_set_actions(d->source, wlActions);
}

DataDeviceManager::DnDAction DataSource::selectedDragAndDropAction() const
{
    return d->selectedAction;
}

}
}

#include "moc_datasource.cpp"
