#include "blurwaylandinterface.h"
#include <QtGlobal>

static const u_int32_t lingmo_blur_surpported_version = 1;

static void handle_global(void *data,struct wl_registry *wl_registry,uint32_t name, const char *interface,  uint32_t version)
{
    auto blurInterface = reinterpret_cast<BlurWaylandInterface *>(data);
    blurInterface->handleGlobal(data,wl_registry,name,interface,version);
}
static void handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name)
{
}
const struct wl_registry_listener BlurWaylandInterface::s_registryListener = {handle_global, handle_global_remove};

BlurWaylandInterface::BlurWaylandInterface(QObject *parent)
    : BlurAbstractInterface(parent)
{
    m_connection = KWayland::Client::ConnectionThread::fromApplication(qApp);
    m_registry = new Registry(this);
    m_registry->create(m_connection->display());

    wl_registry* registry = *m_registry;
    wl_registry_add_listener(registry, &s_registryListener, this);

    m_registry->setup();
    m_connection->roundtrip();
}

void BlurWaylandInterface::handleGlobal(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    if (strcmp(interface, lingmo_blur_manager_v1_interface.name) == 0)
    {
        lingmo_blur_manager_v1 *blur_manager_handle =  (lingmo_blur_manager_v1 *) wl_registry_bind(registry, name, &lingmo_blur_manager_v1_interface, qMin(lingmo_blur_surpported_version, version));
        if(blur_manager_handle)
        {
            m_blur_manager = new LingmoUIBlurManager(this);
            m_blur_manager->setup(blur_manager_handle);
        }
    }
    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        wl_compositor *wl_compositor_handle =  (wl_compositor *) wl_registry_bind(registry, name, &wl_compositor_interface, version);
        if(wl_compositor_handle)
        {
            m_compositor = new Compositor(this);
            m_compositor->setup(wl_compositor_handle);
        }
    }

}

bool BlurWaylandInterface::eventFilter(QObject *obj, QEvent *ev)
{
    auto window = qobject_cast<QWindow*>(obj);
    if(window && ev->type() == QEvent::Hide)
    {
        if(m_surfaces.contains(window))
        {
            auto surface = m_surfaces.value(window);
            if(surface)
            {
                surface->release();
                surface->destroy();
            }
            m_surfaces.remove(window);
        }
        if(m_regions.contains(window))
        {
            auto region = m_regions.value(window);
            if(region)
            {
                region->release();
                region->destroy();
            }
            m_regions.remove(window);
        }
        if(m_blurs.contains(window))
        {
            auto blur_handle = m_blurs.value(window);
            if(blur_handle)
            {
                blur_handle->release();
                blur_handle->destroy();
            }
            m_blurs.remove(window);
        }
    }
    return QObject::eventFilter(obj,ev);
}

bool BlurWaylandInterface::setBlurBehindWithStrength(QWindow *window, bool enable, const QRegion &region, uint32_t strength)
{
    if(!window || !m_blur_manager || !m_compositor)
        return false;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return false;
    if(!m_surfaces.contains(window))
    {
        m_surfaces.insert(window,surface);
    }
    if(enable)
    {
        LingmoUIBlur * blurHandle = m_blur_manager->createBlur(surface, this);
        if(!blurHandle)
            return false;
        if(!m_blurs.contains(window))
        {
            m_blurs.insert(window,blurHandle);
        }
        Region* region_hanlde = m_compositor->createRegion(this);
        if(!region_hanlde)
            return false;
        if(!m_regions.contains(window))
        {
            m_regions.insert(window,region_hanlde);
        }
        for(const QRect &rect : region)
        {
            region_hanlde->add(rect);
        }
        blurHandle->setRegion(region_hanlde);
        blurHandle->setStrength(strength);
        window->installEventFilter(this);
        return true;
    }
    else
    {
        m_blur_manager->removeBlur(surface);
        window->removeEventFilter(this);
        return true;
    }

}
