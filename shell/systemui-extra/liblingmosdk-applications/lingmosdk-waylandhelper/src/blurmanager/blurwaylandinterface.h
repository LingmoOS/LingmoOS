#ifndef BLURWAYLANDINTERFACE_H
#define BLURWAYLANDINTERFACE_H

#include <QObject>
#include <QRegion>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/output.h>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/compositor.h>
#include <QApplication>
#include "lingmoblur.h"
#include "blurabstractinterface.h"

using namespace KWayland::Client;

class BlurWaylandInterface : public BlurAbstractInterface
{
    Q_OBJECT

public:
    explicit BlurWaylandInterface(QObject *parent = nullptr);

    bool setBlurBehindWithStrength(QWindow *window, bool enable = true, const QRegion &region = QRegion(), uint32_t strength = -1) override;

    void handleGlobal(void *data, struct wl_registry *registry,
                              uint32_t name, const char *interface, uint32_t version);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    BlurWaylandInterface* q_ptr;
    ConnectionThread *m_connection = nullptr;
    Compositor* m_compositor = nullptr;
    Registry* m_registry = nullptr;
    LingmoUIBlur* m_blur = nullptr;
    LingmoUIBlurManager* m_blur_manager = nullptr;
    QMap<QWindow*,Surface*>m_surfaces;
    QMap<QWindow*,Region*>m_regions;
    QMap<QWindow*,LingmoUIBlur*>m_blurs;
    static const struct wl_registry_listener s_registryListener;
};

#endif // BLURWAYLANDINTERFACE_H
