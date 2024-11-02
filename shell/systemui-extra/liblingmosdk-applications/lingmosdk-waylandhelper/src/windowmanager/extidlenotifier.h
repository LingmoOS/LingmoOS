#ifndef EXTIDLENOTIFIER_H
#define EXTIDLENOTIFIER_H

#include <QObject>
#include <KWayland/Client/seat.h>
#include <KWayland/Client/surface.h>

#include "wayland-ext-idle-notify-v1-client-protocol.h"

using namespace KWayland::Client;

class ExtIdleNotification;

class ExtIdleNotifier : public QObject
{
    Q_OBJECT
public:
    explicit ExtIdleNotifier(QObject *parent = nullptr);
    ~ExtIdleNotifier();

    bool isValid() const;

    void release();

    void destroy();

    void setup(ext_idle_notifier_v1 *notifier);

    operator ext_idle_notifier_v1 *();
    operator ext_idle_notifier_v1 *() const;

   // static inline struct ext_idle_notification_v1 *
    //ext_idle_notifier_v1_get_idle_notification(struct ext_idle_notifier_v1 *ext_idle_notifier_v1, uint32_t timeout, struct wl_seat *seat)

    ExtIdleNotification* requestExtIdleNotification(wl_seat *seat, int msec);

private:
    class Private;
    QScopedPointer<Private> d;
};

class ExtIdleNotification : public QObject
{
    Q_OBJECT
public:
    explicit ExtIdleNotification(QObject *parent = nullptr);
    ~ExtIdleNotification();

    bool isValid() const;

    void release();

    void destroy();

    void setup(ext_idle_notification_v1 *notification);

    operator ext_idle_notification_v1 *();
    operator ext_idle_notification_v1 *() const;

Q_SIGNALS:
    void idled();
    void resumed();

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // EXTIDLENOTIFIER_H
