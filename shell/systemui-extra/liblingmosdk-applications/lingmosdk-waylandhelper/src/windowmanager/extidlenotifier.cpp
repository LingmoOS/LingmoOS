#include "extidlenotifier.h"
#include "lingmowaylandpointer.h"

class Q_DECL_HIDDEN ExtIdleNotifier::Private
{
public:
    Private(ExtIdleNotifier *q);
    ~Private();
    LingmoUIWaylandPointer<ext_idle_notifier_v1, ext_idle_notifier_v1_destroy> notifier;
    ExtIdleNotifier *q;
};

ExtIdleNotifier::ExtIdleNotifier(QObject *parent)
    : QObject(parent)
    ,d(new Private(this))
{

}

ExtIdleNotifier::~ExtIdleNotifier()
{

}

bool ExtIdleNotifier::isValid() const
{
    return d->notifier.isValid();
}

void ExtIdleNotifier::release()
{
    return d->notifier.release();
}

void ExtIdleNotifier::destroy()
{
    return d->notifier.destroy();
}

void ExtIdleNotifier::setup(ext_idle_notifier_v1 *notifier)
{
    Q_ASSERT(!d->notifier);
    Q_ASSERT(notifier);
    d->notifier.setup(notifier);
}

ExtIdleNotifier::operator ext_idle_notifier_v1 *()
{
    return d->notifier;
}

ExtIdleNotification *ExtIdleNotifier::requestExtIdleNotification(wl_seat *seat, int msec)
{
    ExtIdleNotification* notification = new ExtIdleNotification(this);
    if(!seat || msec <= 0)
        return notification;
    ext_idle_notification_v1 *origin_notification =
            ext_idle_notifier_v1_get_idle_notification(d->notifier,msec,seat);
    notification->setup(origin_notification);
    return notification;
}


ExtIdleNotifier::operator ext_idle_notifier_v1 *() const
{
    return d->notifier;
}


ExtIdleNotifier::Private::Private(ExtIdleNotifier *q)
    :q(q)
{
}

ExtIdleNotifier::Private::~Private()
{
}


class Q_DECL_HIDDEN ExtIdleNotification::Private
{
public:
    Private(ExtIdleNotification *q);
    ~Private();
    LingmoUIWaylandPointer<ext_idle_notification_v1, ext_idle_notification_v1_destroy> notification;
    void init_listener();

private:
    ExtIdleNotification *q;
    static const struct ext_idle_notification_v1_listener s_ext_idle_notification_v1_listener;
    static void handle_idled(void *data,
              struct ext_idle_notification_v1 *ext_idle_notification_v1);

    static void handle_resumed(void *data,
            struct ext_idle_notification_v1 *ext_idle_notification_v1);
    void idled_callback();
    void resumed_callback();
};


const struct ext_idle_notification_v1_listener ExtIdleNotification::Private::s_ext_idle_notification_v1_listener = {
    ExtIdleNotification::Private::handle_idled,
    ExtIdleNotification::Private::handle_resumed
};

ExtIdleNotification::ExtIdleNotification(QObject *parent)
    : QObject(parent)
    ,d(new Private(this))
{

}

ExtIdleNotification::~ExtIdleNotification()
{

}

bool ExtIdleNotification::isValid() const
{
    return d->notification.isValid();
}

void ExtIdleNotification::release()
{
    return d->notification.release();
}

void ExtIdleNotification::destroy()
{
    return d->notification.destroy();
}

void ExtIdleNotification::setup(ext_idle_notification_v1 *notification)
{
    Q_ASSERT(!d->notification);
    Q_ASSERT(notification);
    d->notification.setup(notification);
    d->init_listener();
}

ExtIdleNotification::operator ext_idle_notification_v1 *()
{
    return d->notification;
}

ExtIdleNotification::operator ext_idle_notification_v1 *() const
{
    return d->notification;
}


ExtIdleNotification::Private::Private(ExtIdleNotification *q)
    :q(q)
{
}

ExtIdleNotification::Private::~Private()
{
}

void ExtIdleNotification::Private::init_listener()
{
    ext_idle_notification_v1_add_listener(notification,&s_ext_idle_notification_v1_listener, this);
}

void ExtIdleNotification::Private::handle_idled(void *data, ext_idle_notification_v1 *ext_idle_notification_v1)
{
    static_cast<ExtIdleNotification::Private *>(data)->idled_callback();
}

void ExtIdleNotification::Private::handle_resumed(void *data, ext_idle_notification_v1 *ext_idle_notification_v1)
{
    static_cast<ExtIdleNotification::Private *>(data)->resumed_callback();
}

void ExtIdleNotification::Private::idled_callback()
{
    Q_EMIT q->idled();
}

void ExtIdleNotification::Private::resumed_callback()
{
    Q_EMIT q->resumed();
}
