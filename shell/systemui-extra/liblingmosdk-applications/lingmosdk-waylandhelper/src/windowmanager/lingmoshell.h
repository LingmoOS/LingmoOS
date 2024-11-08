#ifndef LINGMOSHELL_H
#define LINGMOSHELL_H


#include <QObject>
#include <QSize>
#include <QPointer>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/event_queue.h>
#include "lingmo-shell-client-protocol.h"

struct wl_surface;
struct lingmo_surface;
struct wl_seat;

class LingmoUIShellSurface;

using namespace KWayland::Client;

class  LingmoUIShell : public QObject
{
    Q_OBJECT
public:
    explicit LingmoUIShell(QObject *parent = nullptr);
    ~LingmoUIShell() override;


    bool isValid() const;

    void release();

    void destroy();

    void setup(lingmo_shell *shell);

    void setEventQueue(EventQueue *queue);

    EventQueue *eventQueue();

    QString seatName();

    QString outputName();

    bool isCurrentOutputReady();

    LingmoUIShellSurface *createSurface(wl_surface *surface, QObject *parent = nullptr);

    LingmoUIShellSurface *createSurface(Surface *surface, QObject *parent = nullptr);

    void updateCurrentOutput();

    operator lingmo_shell *();
    operator lingmo_shell *() const;

Q_SIGNALS:

    void interfaceAboutToBeReleased();

    void interfaceAboutToBeDestroyed();

    void removed();

    void currentOutputReady();

private:
    class Private;
    QScopedPointer<Private> d;
};


class  LingmoUIShellSurface : public QObject
{
    Q_OBJECT
public:
    explicit LingmoUIShellSurface(QObject *parent);
    ~LingmoUIShellSurface() override;

    void release();

    void destroy();

    void setup(lingmo_surface *surface);

    static LingmoUIShellSurface *get(Surface *surf);

    bool isValid() const;

    operator lingmo_surface *();
    operator lingmo_surface *() const;

    enum class Role {
        Normal,
        Desktop,
        Panel,
        OnScreenDisplay,
        Notification,
        ToolTip,
        CriticalNotification,
        AppletPop,
        ScreenLock,
        Watermark,
        SystemWindow,
        InputPanel,
        Logout,
        ScreenLockNotification,
        Switcher
    };

    enum class SurfaceProperty {
        NoTitleBar,
        Theme,
        WindowRadius,
        BorderWidth,
        BorderColor,
        ShadowRadius,
        ShadowOffset,
        ShadowColor
    };


    void setRole(Role role);

    Role role() const;

    void setSurfaceProperty(SurfaceProperty property, uint32_t value);

    SurfaceProperty surfaceProperty() const;

    void setPosition(const QPoint &point);

    void setSkipTaskbar(bool skip);

    void setSkipSwitcher(bool skip);

    void setPanelTakesFocus(bool takesFocus);

    void setPanelAutoHide(bool autoHide);

    void setGrabKeyboard(wl_seat* seat);

    void setOpenUnderCursor();

    void setOpenUnderCursor(int x, int y);

    void setIconName(const QString& iconName);

Q_SIGNALS:

    void autoHidePanelHidden();

    void autoHidePanelShown();

private:
    friend class LingmoUIShell;
    class Private;
    QScopedPointer<Private> d;
};


#endif // LINGMOSHELL_H
