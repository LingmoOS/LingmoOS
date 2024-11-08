#include "xdgactivation.h"
#include "lingmowaylandpointer.h"

class Q_DECL_HIDDEN XdgActivation::Private
{
public:
    Private(XdgActivation *q);
    ~Private();
    LingmoUIWaylandPointer<xdg_activation_v1, xdg_activation_v1_destroy> activation;
    EventQueue *queue = nullptr;
    XdgActivation *q;
};


XdgActivation::XdgActivation(QObject *parent)
    : QObject(parent)
    ,d(new Private(this))
{

}

XdgActivation::~XdgActivation()
{

}

bool XdgActivation::isValid() const
{
    return d->activation.isValid();
}

void XdgActivation::release()
{
    return d->activation.release();
}

void XdgActivation::destroy()
{
    return d->activation.destroy();
}

void XdgActivation::setup(xdg_activation_v1 *activation)
{
    Q_ASSERT(!d->activation);
    Q_ASSERT(activation);
    d->activation.setup(activation);
}

void XdgActivation::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgActivation::eventQueue()
{
    return d->queue;
}

XdgActivation::operator xdg_activation_v1 *()
{
    return d->activation;
}

XdgActivationToken *XdgActivation::getXdgActivationToken()
{
    XdgActivationToken* token = new XdgActivationToken(this);
    auto origin_token = xdg_activation_v1_get_activation_token((d->activation));
    token->setup(origin_token);
    return token;
}

void XdgActivation::activateWindow(const QString &token, Surface *surface)
{
    xdg_activation_v1_activate(
        d->activation,
        token.toUtf8().constData(),
        *surface);
}

XdgActivationToken *XdgActivation::requestXdgActivationToken(wl_seat *seat, Surface *surface, uint32_t serial, const QString &app_id)
{
    auto provider = getXdgActivationToken();
    if (surface) {
        provider->set_surface(surface);
    }

    if (!app_id.isEmpty()) {
        provider->set_app_id(app_id);
    }

    if (seat) {
        provider->set_serial(serial, seat);
    }
    provider->commit();
    return provider;
}

XdgActivation::operator xdg_activation_v1 *() const
{
    return d->activation;
}


class Q_DECL_HIDDEN XdgActivationToken::Private
{
public:
    Private(XdgActivationToken *q);
    ~Private();
    LingmoUIWaylandPointer<xdg_activation_token_v1, xdg_activation_token_v1_destroy> token;
    void init_listener();

private:
    XdgActivationToken *q;

    static const struct xdg_activation_token_v1_listener m_xdg_activation_token_v1_listener;
    static void handle_done(
        void *data,
        struct ::xdg_activation_token_v1 *object,
        const char *token);
    void doneCallback(const QString &token);
};

XdgActivationToken::XdgActivationToken(QObject *parent)
    : QObject(parent)
    ,d(new Private(this))
{
}

XdgActivationToken::~XdgActivationToken()
{

}

bool XdgActivationToken::isValid() const
{
    return d->token.isValid();
}

void XdgActivationToken::release()
{
    return d->token.release();
}

void XdgActivationToken::destroy()
{
    return d->token.destroy();
}

void XdgActivationToken::setup(xdg_activation_token_v1 *token)
{
    Q_ASSERT(!d->token);
    Q_ASSERT(token);
    d->token.setup(token);
    d->init_listener();
}

void XdgActivationToken::set_serial(uint32_t serial, wl_seat *seat)
{
    xdg_activation_token_v1_set_serial(d->token,serial,seat);
}

void XdgActivationToken::set_app_id(const QString &app_id)
{
    xdg_activation_token_v1_set_app_id(
        d->token,
        app_id.toUtf8().constData());
}

void XdgActivationToken::set_surface(Surface *surface)
{
    xdg_activation_token_v1_set_surface(
        d->token,
        *surface);
}

void XdgActivationToken::commit()
{
    xdg_activation_token_v1_commit(
        d->token);
}

XdgActivationToken::operator xdg_activation_token_v1 *()
{
     return d->token;
}

XdgActivationToken::operator xdg_activation_token_v1 *() const
{
     return d->token;
}

XdgActivationToken::Private::Private(XdgActivationToken *q)
    :q(q)
{
}

XdgActivationToken::Private::~Private()
{

}

const struct xdg_activation_token_v1_listener XdgActivationToken::Private::m_xdg_activation_token_v1_listener = {
    XdgActivationToken::Private::handle_done,
};
void XdgActivationToken::Private::init_listener()
{
     xdg_activation_token_v1_add_listener(token, &m_xdg_activation_token_v1_listener, this);
}

void XdgActivationToken::Private::handle_done(void *data, xdg_activation_token_v1 *object, const char *token)
{
    static_cast<XdgActivationToken::Private *>(data)->doneCallback(QString::fromUtf8(token));
}

void XdgActivationToken::Private::doneCallback(const QString &token)
{
    Q_EMIT q->done(token);
}

XdgActivation::Private::Private(XdgActivation *q)
    :q(q)
{

}

XdgActivation::Private::~Private()
{

}
