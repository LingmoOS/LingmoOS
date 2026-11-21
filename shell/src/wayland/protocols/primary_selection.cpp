// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Uses the existing Lingmo selection model and primary-selection-unstable-v1.
#include "primary_selection.h"

#include "clipboard/selection.h"
#include "input/seat.h"
#include "primary-selection-unstable-v1-server-protocol.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/server_util.h"
#include <unistd.h>

namespace Lingmo::Wayland {
namespace {
class Source : public clipboard::DataSource
{
public:
    wl_resource *resource = nullptr;

    void send(const std::string &mime, int fd) override
    {
        if (resource && alive() && supports(mime)
            && !isClientDestroyed(wl_resource_get_client(resource))) {
            zwp_primary_selection_source_v1_send_send(resource, mime.c_str(), fd);
            wl_client_flush(wl_resource_get_client(resource));
        }
        close(fd);
    }

    void cancelled() override
    {
        if (resource && !isClientDestroyed(wl_resource_get_client(resource)))
            zwp_primary_selection_source_v1_send_cancelled(resource);
    }
};

struct SourceResource {
    std::shared_ptr<Source> source = std::make_shared<Source>();

    ~SourceResource()
    {
        source->resource = nullptr;
        source->invalidate();
    }
};

void destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void sourceOffer(wl_client *, wl_resource *resource, const char *mime)
{
    getUserDataAs<SourceResource>(resource)->source->offer(mime);
}
const struct zwp_primary_selection_source_v1_interface sourceImpl = {sourceOffer, destroy};

struct Offer {
    clipboard::DataOffer data;
    std::weak_ptr<bool> device;

    Offer(clipboard::SelectionController *controller, std::weak_ptr<bool> lifetime)
        : data(controller)
        , device(lifetime)
    {
    }
};

void receive(wl_client *, wl_resource *resource, const char *mime, int fd)
{
    auto *offer = getUserDataAs<Offer>(resource);
    if (offer->device.expired())
        close(fd);
    else
        offer->data.receive(mime, fd);
}
const struct zwp_primary_selection_offer_v1_interface offerImpl = {receive, destroy};

struct Device {
    wl_resource *resource;
    bool focused = false;
    base::WeakPtr<clipboard::SelectionController> controller, authorization;
    std::shared_ptr<bool> lifetime = std::make_shared<bool>(true);
    base::Subscription selectionChanged;
    base::Subscription recipientChanged;

    Device(wl_resource *resource, input::Seat *seat)
        : resource(resource)
    {
        if (!seat)
            return;
        controller = seat->primarySelection() ? seat->primarySelection()->weakPtr() : nullptr;
        authorization = seat->selection() ? seat->selection()->weakPtr() : nullptr;
        if (!controller)
            return;
        selectionChanged = controller->selectionChanged.connect([this] {
            deliver();
        });
        recipientChanged = controller->recipientChanged.connect([this] {
            deliver();
        });
    }

    void deliver()
    {
        auto *client = wl_resource_get_client(resource);
        if (isClientDestroyed(client))
            return;
        const bool eligible = controller && controller->recipient() == client;
        if (!eligible && !focused)
            return;
        focused = eligible;
        wl_resource *offer = nullptr;
        if (eligible && controller->source()) {
            offer = wl_resource_create(client, &zwp_primary_selection_offer_v1_interface, 1, 0);
            if (!offer) {
                wl_client_post_no_memory(client);
                return;
            }
            auto data = std::make_unique<Offer>(controller.get(), lifetime);
            auto formats = data->data.formats();
            setImplementation(offer, &offerImpl, std::move(data));
            zwp_primary_selection_device_v1_send_data_offer(resource, offer);
            for (const auto &mime : formats)
                zwp_primary_selection_offer_v1_send_offer(offer, mime.c_str());
        }
        zwp_primary_selection_device_v1_send_selection(resource, offer);
        wl_client_flush(client);
    }
};

void select(wl_client *client, wl_resource *resource, wl_resource *sourceResource, uint32_t serial)
{
    auto *device = getUserDataAs<Device>(resource);
    if (!device->controller || !device->authorization
        || !device->authorization->authorize(client, serial, *device->controller))
        return;
    std::shared_ptr<Source> source;
    if (sourceResource) {
        if (!wl_resource_instance_of(sourceResource, &zwp_primary_selection_source_v1_interface,
                                     &sourceImpl)
            || wl_resource_get_client(sourceResource) != client)
            return;
        source = getUserDataAs<SourceResource>(sourceResource)->source;
        if (!source->alive() || source->used())
            return;
    }
    device->controller->setSource(std::move(source));
}
const struct zwp_primary_selection_device_v1_interface deviceImpl = {select, destroy};

void createSource(wl_client *client, wl_resource *, uint32_t id)
{
    auto *resource = wl_resource_create(client, &zwp_primary_selection_source_v1_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto data = std::make_unique<SourceResource>();
    data->source->resource = resource;
    setImplementation(resource, &sourceImpl, std::move(data));
}

void getDevice(wl_client *client, wl_resource *manager, uint32_t id, wl_resource *seatResource)
{
    auto *seat = getUserDataAs<WaylandSeat>(manager);
    auto *resource = wl_resource_create(client, &zwp_primary_selection_device_v1_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto device = std::make_unique<Device>(resource,
                                           waylandSeatFromResource(seatResource) == seat
                                                           && wl_resource_get_client(seatResource)
                                                                   == client
                                                   ? seat->seat
                                                   : nullptr);
    auto *raw = device.get();
    setImplementation(resource, &deviceImpl, std::move(device));
    raw->deliver();
}
const struct zwp_primary_selection_device_manager_v1_interface managerImpl = {createSource,
                                                                              getDevice, destroy};

void bind(wl_client *client, void *data, uint32_t, uint32_t id)
{
    auto *resource =
            wl_resource_create(client, &zwp_primary_selection_device_manager_v1_interface, 1, id);
    if (resource)
        wl_resource_set_implementation(resource, &managerImpl, data, nullptr);
    else
        wl_client_post_no_memory(client);
}
} // namespace

wl_global *createPrimarySelectionGlobal(wl_display *display, WaylandSeat *seat)
{
    return wl_global_create(display, &zwp_primary_selection_device_manager_v1_interface, 1, seat,
                            bind);
}
} // namespace Lingmo::Wayland
