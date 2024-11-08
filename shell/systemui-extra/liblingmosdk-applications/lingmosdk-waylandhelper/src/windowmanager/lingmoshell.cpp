#include "lingmowaylandpointer.h"
#include "lingmoshell.h"

class Q_DECL_HIDDEN LingmoUIShell::Private
{
public:
    Private(LingmoUIShell *q);

    LingmoUIWaylandPointer<lingmo_shell, lingmo_shell_destroy> shell;
    EventQueue *queue = nullptr;
    LingmoUIShell *q;
    QString m_seatName;
    QString m_outputName;
    bool m_currentOutputReady = false;
    void init_listener();

    static struct lingmo_shell_listener s_listener;
    static void currentOutputCallback(void *data, struct lingmo_shell *lingmo_shell, const char *output_name, const char *seat_name);
    static void doneCallback(void *data, struct lingmo_shell *lingmo_shell);

};

lingmo_shell_listener LingmoUIShell::Private::s_listener = {
    .current_output = currentOutputCallback,
    .done = doneCallback
};

class Q_DECL_HIDDEN LingmoUIShellSurface::Private
{
public:
    Private(LingmoUIShellSurface *q);
    ~Private();
    void setup(lingmo_surface *surface);

    LingmoUIWaylandPointer<lingmo_surface, lingmo_surface_destroy> surface;
    QSize size;
    QPointer<Surface> parentSurface;
    LingmoUIShellSurface::Role role;
    LingmoUIShellSurface::SurfaceProperty property;

    static LingmoUIShellSurface *get(Surface *surface);

private:
    LingmoUIShellSurface *q;
    static QVector<Private *> s_surfaces;
};

QVector<LingmoUIShellSurface::Private *> LingmoUIShellSurface::Private::s_surfaces;

LingmoUIShell::LingmoUIShell(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

LingmoUIShell::~LingmoUIShell()
{
    release();
}

void LingmoUIShell::destroy()
{
    if (!d->shell) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->shell.destroy();
}

void LingmoUIShell::release()
{
    if (!d->shell) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->shell.release();
}

void LingmoUIShell::setup(lingmo_shell *shell)
{
    Q_ASSERT(!d->shell);
    Q_ASSERT(shell);
    d->shell.setup(shell);
    d->init_listener();

}

void LingmoUIShell::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *LingmoUIShell::eventQueue()
{
    return d->queue;
}

QString LingmoUIShell::seatName()
{
    if(d->m_currentOutputReady)
        return d->m_seatName;
    else
        return QString();
}

QString LingmoUIShell::outputName()
{
    if(d->m_currentOutputReady)
        return d->m_outputName;
    else
        return QString();
}

bool LingmoUIShell::isCurrentOutputReady()
{
    return d->m_currentOutputReady;
}

LingmoUIShellSurface *LingmoUIShell::createSurface(wl_surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto kwS = Surface::get(surface);
    if (kwS) {
        if (auto s = LingmoUIShellSurface::Private::get(kwS)) {
            return s;
        }
    }
    LingmoUIShellSurface *s = new LingmoUIShellSurface(parent);
    connect(this, &LingmoUIShell::interfaceAboutToBeReleased, s, &LingmoUIShellSurface::release);
    connect(this, &LingmoUIShell::interfaceAboutToBeDestroyed, s, &LingmoUIShellSurface::destroy);
    auto w = lingmo_shell_create_surface(d->shell, surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    s->d->parentSurface = QPointer<Surface>(kwS);
    return s;
}

LingmoUIShellSurface *LingmoUIShell::createSurface(Surface *surface, QObject *parent)
{
    return createSurface(*surface, parent);
}

void LingmoUIShell::updateCurrentOutput()
{
    lingmo_shell_get_current_output(d->shell);
}

bool LingmoUIShell::isValid() const
{
    return d->shell.isValid();
}

LingmoUIShell::operator lingmo_shell *()
{
    return d->shell;
}

LingmoUIShell::operator lingmo_shell *() const
{
    return d->shell;
}

LingmoUIShellSurface::Private::Private(LingmoUIShellSurface *q)
    : role(LingmoUIShellSurface::Role::Normal)
    , q(q)
{
    s_surfaces << this;
}

LingmoUIShellSurface::Private::~Private()
{
    s_surfaces.removeAll(this);
}

LingmoUIShellSurface *LingmoUIShellSurface::Private::get(Surface *surface)
{
    if (!surface) {
        return nullptr;
    }
    for (auto it = s_surfaces.constBegin(); it != s_surfaces.constEnd(); ++it) {
        if ((*it)->parentSurface == surface) {
            return (*it)->q;
        }
    }
    return nullptr;
}

void LingmoUIShellSurface::Private::setup(lingmo_surface *s)
{
    Q_ASSERT(s);
    Q_ASSERT(!surface);
    surface.setup(s);
}

LingmoUIShellSurface::LingmoUIShellSurface(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

LingmoUIShellSurface::~LingmoUIShellSurface()
{
    release();
}

void LingmoUIShellSurface::release()
{
    d->surface.release();
}

void LingmoUIShellSurface::destroy()
{
    d->surface.destroy();
}

void LingmoUIShellSurface::setup(lingmo_surface *surface)
{
    d->setup(surface);
}

LingmoUIShellSurface *LingmoUIShellSurface::get(Surface *surface)
{
    if (auto s = LingmoUIShellSurface::Private::get(surface)) {
        return s;
    }

    return nullptr;
}

bool LingmoUIShellSurface::isValid() const
{
    return d->surface.isValid();
}

LingmoUIShellSurface::operator lingmo_surface *()
{
    return d->surface;
}

LingmoUIShellSurface::operator lingmo_surface *() const
{
    return d->surface;
}

void LingmoUIShellSurface::setPosition(const QPoint &point)
{
    Q_ASSERT(isValid());
    lingmo_surface_set_position(d->surface, point.x(), point.y());
}

void LingmoUIShellSurface::setRole(LingmoUIShellSurface::Role role)
{
    Q_ASSERT(isValid());
    uint32_t wlRole = LINGMO_SURFACE_ROLE_NORMAL;
    switch (role) {
    case Role::Normal:
        wlRole = LINGMO_SURFACE_ROLE_NORMAL;
        break;
    case Role::Desktop:
        wlRole = LINGMO_SURFACE_ROLE_DESKTOP;
        break;
    case Role::Panel:
        wlRole = LINGMO_SURFACE_ROLE_PANEL;
        break;
    case Role::OnScreenDisplay:
        wlRole = LINGMO_SURFACE_ROLE_ONSCREENDISPLAY;
        break;
    case Role::Notification:
        wlRole = LINGMO_SURFACE_ROLE_NOTIFICATION;
        break;
    case Role::ToolTip:
        wlRole = LINGMO_SURFACE_ROLE_TOOLTIP;
        break;
    case Role::CriticalNotification:
        wlRole = LINGMO_SURFACE_ROLE_CRITICALNOTIFICATION;
        break;
    case Role::AppletPop:
        wlRole = LINGMO_SURFACE_ROLE_APPLETPOPUP;
        break;
    case Role::ScreenLock:
        wlRole = LINGMO_SURFACE_ROLE_SCREENLOCK;
        break;
    case Role::Watermark:
        wlRole = LINGMO_SURFACE_ROLE_WATERMARK;
        break;
    case Role::SystemWindow:
        wlRole = LINGMO_SURFACE_ROLE_SYSTEMWINDOW;
        break;
    case Role::InputPanel:
        wlRole = LINGMO_SURFACE_ROLE_INPUTPANEL;
        break;
    case  Role::Logout:
        wlRole = LINGMO_SURFACE_ROLE_LOGOUT;
        break;
    case  Role::ScreenLockNotification:
        wlRole = LINGMO_SURFACE_ROLE_SCREENLOCKNOTIFICATION;
        break;
    case  Role::Switcher:
        wlRole = LINGMO_SURFACE_ROLE_SWITCHER;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    lingmo_surface_set_role(d->surface, wlRole);
    d->role = role;
}

LingmoUIShellSurface::Role LingmoUIShellSurface::role() const
{
    return d->role;
}

void LingmoUIShellSurface::setSurfaceProperty(SurfaceProperty property,uint32_t value)
{
    uint32_t wlPropery = -1;
    switch(property)
    {
    case SurfaceProperty::NoTitleBar:
        wlPropery = LINGMO_SURFACE_PROPERTY_NO_TITLEBAR;
        break;
    case SurfaceProperty::Theme:
        wlPropery = LINGMO_SURFACE_PROPERTY_THEME;
        break;
    case SurfaceProperty::WindowRadius:
        wlPropery = LINGMO_SURFACE_PROPERTY_WINDOW_RADIUS;
        break;
    case SurfaceProperty::BorderWidth:
        wlPropery = LINGMO_SURFACE_PROPERTY_BORDER_WIDTH;
        break;
    case SurfaceProperty::BorderColor:
        wlPropery = LINGMO_SURFACE_PROPERTY_BORDER_COLOR;
        break;
    case SurfaceProperty::ShadowRadius:
        wlPropery = LINGMO_SURFACE_PROPERTY_SHADOW_RADIUS;
        break;
    case SurfaceProperty::ShadowOffset:
        wlPropery = LINGMO_SURFACE_PROPERTY_SHADOW_OFFSET;
        break;
    case SurfaceProperty::ShadowColor:
        wlPropery = LINGMO_SURFACE_PROPERTY_SHADOW_COLOR;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }

    lingmo_surface_set_property(d->surface, wlPropery,value);
    d->property = property;
}

LingmoUIShellSurface::SurfaceProperty LingmoUIShellSurface::surfaceProperty() const
{
    return d->property;
}

void LingmoUIShellSurface::setSkipTaskbar(bool skip)
{
    lingmo_surface_set_skip_taskbar(d->surface, skip);
}

void LingmoUIShellSurface::setSkipSwitcher(bool skip)
{
    lingmo_surface_set_skip_switcher(d->surface, skip);
}

void LingmoUIShellSurface::setPanelTakesFocus(bool takesFocus)
{
    lingmo_surface_set_panel_takes_focus(d->surface, takesFocus);
}

void LingmoUIShellSurface::setPanelAutoHide(bool autoHide)
{
    lingmo_surface_set_panel_auto_hide(d->surface, autoHide);
}

void LingmoUIShellSurface::setGrabKeyboard(wl_seat *seat)
{
    lingmo_surface_grab_keyboard(d->surface, seat);
}

void LingmoUIShellSurface::setOpenUnderCursor()
{
    lingmo_surface_open_under_cursor(d->surface, 0 , 0);
}

void LingmoUIShellSurface::setOpenUnderCursor(int x, int y)
{
    lingmo_surface_open_under_cursor(d->surface, x , y);
}

void LingmoUIShellSurface::setIconName(const QString& iconName)
{
    if (iconName.isEmpty()) {
        lingmo_surface_set_icon(d->surface, NULL);
    } else {
        lingmo_surface_set_icon(d->surface, iconName.toStdString().c_str());
    }
}

LingmoUIShell::Private::Private(LingmoUIShell *q)
    :q(q)
{

}

void LingmoUIShell::Private::init_listener()
{
    lingmo_shell_add_listener(shell, &s_listener, this);
}

void LingmoUIShell::Private::currentOutputCallback(void *data, lingmo_shell *lingmo_shell, const char *output_name, const char *seat_name)
{
    auto lingmoShell = reinterpret_cast<LingmoUIShell::Private *>(data);
    if(lingmoShell->shell != lingmo_shell)
        return;
    lingmoShell->m_outputName = QString::fromUtf8(output_name);
    lingmoShell->m_seatName = QString::fromUtf8(seat_name);
}

void LingmoUIShell::Private::doneCallback(void *data, lingmo_shell *lingmo_shell)
{
    auto lingmoShell = reinterpret_cast<LingmoUIShell::Private *>(data);
    if(lingmoShell->shell != lingmo_shell)
        return;
    lingmoShell->m_currentOutputReady = true;
    Q_EMIT lingmoShell->q->currentOutputReady();
}
