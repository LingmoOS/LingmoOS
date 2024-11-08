#ifndef LINGMOBLUR_H
#define LINGMOBLUR_H

#include <QObject>
#include <KWayland/Client/region.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/surface.h>
#include "lingmo-blur-manager-client-protocol.h"

struct lingmo_blur_manager_v1;
struct lingmo_blur_surface_v1;
using namespace KWayland::Client;

class LingmoUIBlur : public QObject
{
    Q_OBJECT
public:
    explicit LingmoUIBlur(QObject *parent = nullptr);
    ~LingmoUIBlur();

    bool isValid() const;

    void release();

    void destroy();

    void setup(lingmo_blur_surface_v1 *blur);

    void setRegion(Region *region);

    void setStrength(uint32_t strength);


    operator lingmo_blur_surface_v1 *();
    operator lingmo_blur_surface_v1 *() const;

private:
    class Private;
    QScopedPointer<Private> d;

};

class LingmoUIBlurManager : public QObject
{
    Q_OBJECT
public:
    explicit LingmoUIBlurManager(QObject *parent = nullptr);
    ~LingmoUIBlurManager();

    bool isValid() const;

    void release();

    void destroy();

    void setup(lingmo_blur_manager_v1 *blur_manager);

    void setEventQueue(EventQueue *queue);

    EventQueue *eventQueue();

    LingmoUIBlur *createBlur(Surface *surface, QObject *parent = nullptr);

    void removeBlur(Surface *surface);

    operator lingmo_blur_manager_v1 *();

    operator lingmo_blur_manager_v1 *() const;

private:
    class Private;
    QScopedPointer<Private> d;

};

#endif // LINGMOBLUR_H
