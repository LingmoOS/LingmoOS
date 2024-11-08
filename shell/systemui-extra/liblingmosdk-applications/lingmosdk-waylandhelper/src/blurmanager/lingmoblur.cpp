#include "lingmoblur.h"
#include "../windowmanager/lingmowaylandpointer.h"

class Q_DECL_HIDDEN LingmoUIBlur::Private
{
public:
    LingmoUIWaylandPointer<lingmo_blur_surface_v1, lingmo_blur_surface_v1_destroy> blur;
};

class Q_DECL_HIDDEN LingmoUIBlurManager::Private
{
public:
    Private(LingmoUIBlurManager *q);

    LingmoUIWaylandPointer<lingmo_blur_manager_v1, lingmo_blur_manager_v1_destroy> blurManager;
    EventQueue *queue = nullptr;

private:
    LingmoUIBlurManager* q;
};

LingmoUIBlur::LingmoUIBlur(QObject *parent)
    : QObject(parent),
      d(new Private)
{

}

LingmoUIBlur::~LingmoUIBlur()
{
    release();
}

bool LingmoUIBlur::isValid() const
{
    return d->blur.isValid();
}

void LingmoUIBlur::release()
{
    d->blur.release();
}

void LingmoUIBlur::destroy()
{
    d->blur.destroy();
}

void LingmoUIBlur::setup(lingmo_blur_surface_v1 *blur)
{
    Q_ASSERT(blur);
    Q_ASSERT(!d->blur);
    d->blur.setup(blur);
}

void LingmoUIBlur::setRegion(Region *region)
{
    lingmo_blur_surface_v1_set_region(d->blur, *region);
}

void LingmoUIBlur::setStrength(uint32_t strength)
{
    lingmo_blur_surface_v1_set_level(d->blur, strength);
}

LingmoUIBlur::operator lingmo_blur_surface_v1 *()
{
    return d->blur;
}

LingmoUIBlur::operator lingmo_blur_surface_v1 *() const
{
    return d->blur;
}

LingmoUIBlurManager::LingmoUIBlurManager(QObject *parent)
    : QObject(parent),
      d(new Private(this))
{

}

LingmoUIBlurManager::~LingmoUIBlurManager()
{
    release();
}

bool LingmoUIBlurManager::isValid() const
{
    if(!d->blurManager)
        return false;
    return d->blurManager.isValid();
}

void LingmoUIBlurManager::release()
{
    d->blurManager.release();
}

void LingmoUIBlurManager::destroy()
{
    d->blurManager.destroy();
}

void LingmoUIBlurManager::setup(lingmo_blur_manager_v1 *blur_manager)
{
    d->blurManager.setup(blur_manager);
}

void LingmoUIBlurManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *LingmoUIBlurManager::eventQueue()
{
    return d->queue;
}

LingmoUIBlur *LingmoUIBlurManager::createBlur(Surface *surface, QObject *parent)
{
    if(!isValid())
        return nullptr;
    LingmoUIBlur *s = new LingmoUIBlur(parent);
    auto w = lingmo_blur_manager_v1_get_blur(d->blurManager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

void LingmoUIBlurManager::removeBlur(Surface *surface)
{
    Q_ASSERT(isValid());
//    lingmo_blur_manager_unset(d->blurManager, *surface);
}

LingmoUIBlurManager::operator lingmo_blur_manager_v1 *()
{
    return d->blurManager;
}

LingmoUIBlurManager::operator lingmo_blur_manager_v1 *() const
{
    return d->blurManager;
}

LingmoUIBlurManager::Private::Private(LingmoUIBlurManager *q)
    :q(q)
{
}
