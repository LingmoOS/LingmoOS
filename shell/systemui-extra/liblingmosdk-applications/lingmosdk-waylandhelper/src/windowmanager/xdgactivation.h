#ifndef XDGACTIVATION_H
#define XDGACTIVATION_H

#include <QObject>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/seat.h>
#include <KWayland/Client/surface.h>

#include "wayland-xdg-activation-v1-client-protocol.h"

using namespace KWayland::Client;

class XdgActivationToken;

class XdgActivation : public QObject
{
    Q_OBJECT
public:
    explicit XdgActivation(QObject *parent = nullptr);
    ~XdgActivation();

    bool isValid() const;

    void release();

    void destroy();

    void setup(xdg_activation_v1 *activation);


    void setEventQueue(EventQueue *queue);

    EventQueue *eventQueue();

    operator xdg_activation_v1 *();
    operator xdg_activation_v1 *() const;

    XdgActivationToken* getXdgActivationToken();
    XdgActivationToken* requestXdgActivationToken(wl_seat *seat, Surface *surface, uint32_t serial, const QString &app_id);
    void activateWindow(const QString&token,  Surface *surface);

private:
    class Private;
    QScopedPointer<Private> d;

};

class XdgActivationToken: public QObject
{
    Q_OBJECT
public:
    explicit XdgActivationToken(QObject *parent = nullptr);
    ~XdgActivationToken();

    bool isValid() const;

    void release();

    void destroy();

    void setup(xdg_activation_token_v1 *token);

    void set_serial(uint32_t serial, wl_seat *seat);

    void set_app_id(const QString &app_id);

    void set_surface(Surface *surface);

    void commit();

    operator xdg_activation_token_v1 *();
    operator xdg_activation_token_v1 *() const;

Q_SIGNALS:
    void done(const QString &token);

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // XDGACTIVATION_H
