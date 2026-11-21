// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/data_control.h"

#include <algorithm>
#include <memory>
#include <unistd.h>


#include "clipboard/selection.h"
#include "ext-data-control-v1-server-protocol.h"
#include "input/seat.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/server_util.h"
#include "wlr-data-control-unstable-v1-server-protocol.h"

namespace Lingmo::Wayland {

namespace {

// Both protocols share request/event opcodes; their interface identities differ.
struct Protocol {
    const wl_interface* manager;
    const wl_interface* device;
    const wl_interface* source;
    const wl_interface* offer;
    uint32_t version;
};

const Protocol ext {&ext_data_control_manager_v1_interface, &ext_data_control_device_v1_interface,
                    &ext_data_control_source_v1_interface, &ext_data_control_offer_v1_interface, 1};
const Protocol wlr {&zwlr_data_control_manager_v1_interface, &zwlr_data_control_device_v1_interface,
                    &zwlr_data_control_source_v1_interface, &zwlr_data_control_offer_v1_interface,
                    2};

void destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

class Source : public clipboard::DataSource
{
public:
    wl_resource* resource = nullptr;

    void send(const std::string& mime, int fd) override
    {
        if (resource && alive() && supports(mime) &&
            !isClientDestroyed(wl_resource_get_client(resource))) {
            wl_resource_post_event(resource, 0, mime.c_str(), fd);
            wl_client_flush(wl_resource_get_client(resource));
        }
        close(fd);
    }

    void cancelled() override
    {
        if (resource && !isClientDestroyed(wl_resource_get_client(resource))) {
            wl_resource_post_event(resource, 1);
            wl_client_flush(wl_resource_get_client(resource));
        }
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

void sourceOffer(wl_client*, wl_resource* resource, const char* mime)
{
    auto source = getUserDataAs<SourceResource>(resource)->source;
    if (source->used()) {
        wl_resource_post_error(resource, 1, "cannot add MIME types to a used source");
        return;
    }
    source->offer(mime);
}

const struct ext_data_control_source_v1_interface sourceImplementation = {sourceOffer, destroy};

struct OfferResource {
    base::WeakPtr<clipboard::SelectionController> controller;
    std::weak_ptr<clipboard::DataSource> source;
    std::weak_ptr<bool> device;
    std::vector<std::string> formats;
    uint64_t generation;
};

void receive(wl_client*, wl_resource* resource, const char* mime, int fd)
{
    auto* offer = getUserDataAs<OfferResource>(resource);
    auto source = offer->source.lock();
    if (offer->device.expired() || !offer->controller || !source || !source->alive() ||
        offer->generation != offer->controller->generation() ||
        std::find(offer->formats.begin(), offer->formats.end(), mime) == offer->formats.end()) {
        close(fd);
        return;
    }
    source->send(mime, fd);
}

const struct ext_data_control_offer_v1_interface offerImplementation = {receive, destroy};

class Device
{
public:
    Device(wl_resource* resourceArg, const Protocol* protocol,
           clipboard::SelectionController* controller, clipboard::SelectionController* primary)
        : resource(resourceArg)
        , protocol(protocol)
        , controller(controller->weakPtr())
    {
        // wlr added primary selection in version 2; ext has it from version 1.
        if (protocol == &ext ||
            wl_resource_get_version(resource) >=
                ZWLR_DATA_CONTROL_DEVICE_V1_PRIMARY_SELECTION_SINCE_VERSION)
            this->primary = primary ? primary->weakPtr() : nullptr;
        selectionChanged = controller->selectionChanged.connect([this] {
            deliver(this->controller.get(), 1);
        });
        if (this->primary) {
            primaryChanged = this->primary->selectionChanged.connect([this] {
                deliver(this->primary.get(), 3);
            });
        }
        controllerDestroyed = controller->destroyed.connect([this] {
            lifetime.reset();
            wl_resource_post_event(resource, 2);
            wl_client_flush(wl_resource_get_client(resource));
        });
    }

    void deliverAll()
    {
        deliver(controller.get(), 1);
        if (primary)
            deliver(primary.get(), 3);
    }

    // event is the selection (1) or primary_selection (3) opcode.
    void deliver(clipboard::SelectionController* selection, uint32_t event)
    {
        auto* client = wl_resource_get_client(resource);
        if (!selection || !lifetime || isClientDestroyed(client))
            return;
        wl_resource* offer = nullptr;
        if (auto source = selection->source()) {
            offer = wl_resource_create(client, protocol->offer, 1, 0);
            if (!offer) {
                wl_client_post_no_memory(client);
                return;
            }
            auto data = std::make_unique<OfferResource>();
            data->controller = selection->weakPtr();
            data->source = source;
            data->device = lifetime;
            data->formats = source->formats();
            data->generation = selection->generation();
            setImplementation(offer, &offerImplementation, std::move(data));
            wl_resource_post_event(resource, 0, offer);
            for (const auto& mime : source->formats())
                wl_resource_post_event(offer, 0, mime.c_str());
        }
        wl_resource_post_event(resource, event, offer);
        wl_client_flush(client);
    }

    wl_resource* resource;
    const Protocol* protocol;
    base::WeakPtr<clipboard::SelectionController> controller;
    base::WeakPtr<clipboard::SelectionController> primary;
    std::shared_ptr<bool> lifetime = std::make_shared<bool>(true);
    base::Subscription selectionChanged;
    base::Subscription primaryChanged;
    base::Subscription controllerDestroyed;
};

void setSource(wl_client* client, wl_resource* resource, wl_resource* sourceResource,
               clipboard::SelectionController* selection)
{
    auto* device = getUserDataAs<Device>(resource);
    if (!device || !selection)
        return;
    std::shared_ptr<Source> source;
    if (sourceResource) {
        if (wl_resource_get_client(sourceResource) != client ||
            !wl_resource_instance_of(sourceResource, device->protocol->source,
                                     &sourceImplementation))
            return;
        source = getUserDataAs<SourceResource>(sourceResource)->source;
        if (source->used()) {
            wl_resource_post_error(resource, 1, "source was already used");
            return;
        }
    }
    selection->setSource(std::move(source));
}

void setSelection(wl_client* client, wl_resource* resource, wl_resource* sourceResource)
{
    auto* device = getUserDataAs<Device>(resource);
    setSource(client, resource, sourceResource, device ? device->controller.get() : nullptr);
}

void setPrimary(wl_client* client, wl_resource* resource, wl_resource* sourceResource)
{
    auto* device = getUserDataAs<Device>(resource);
    setSource(client, resource, sourceResource, device ? device->primary.get() : nullptr);
}

const struct ext_data_control_device_v1_interface deviceImplementation = {setSelection, destroy,
                                                                          setPrimary};

struct ManagerResource {
    DataControlManager* manager;
    const Protocol* protocol;
};

void createSource(wl_client* client, wl_resource* resource, uint32_t id)
{
    auto* manager = getUserDataAs<ManagerResource>(resource);
    auto* source = wl_resource_create(client, manager->protocol->source, 1, id);
    if (!source) {
        wl_client_post_no_memory(client);
        return;
    }
    auto data = std::make_unique<SourceResource>();
    data->source->resource = source;
    setImplementation(source, &sourceImplementation, std::move(data));
}

void getDevice(wl_client* client, wl_resource* resource, uint32_t id, wl_resource* seatResource)
{
    auto* manager = getUserDataAs<ManagerResource>(resource);
    auto* device = wl_resource_create(client, manager->protocol->device,
                                      wl_resource_get_version(resource), id);
    if (!device) {
        wl_client_post_no_memory(client);
        return;
    }
    auto* seat = waylandSeatFromResource(seatResource);
    if (!seat || seat != manager->manager->seat() ||
        wl_resource_get_client(seatResource) != client) {
        wl_resource_set_implementation(device, &deviceImplementation, nullptr, nullptr);
        wl_resource_post_event(device, 2);
        return;
    }
    auto data = std::make_unique<Device>(device, manager->protocol, seat->seat->selection(),
                                         seat->seat->primarySelection());
    auto* raw = data.get();
    setImplementation(device, &deviceImplementation, std::move(data));
    raw->deliverAll();
}

const struct ext_data_control_manager_v1_interface managerImplementation = {createSource, getDevice,
                                                                            destroy};

void bind(wl_client* client, void* data, uint32_t version, uint32_t id, const Protocol* protocol)
{
    auto* manager = static_cast<DataControlManager*>(data);
    if (!manager->authorized(client)) {
        wl_client_post_implementation_error(client,
                                            "data control requires a trusted clipboard client");
        return;
    }
    auto* resource =
        wl_resource_create(client, protocol->manager, std::min(version, protocol->version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    auto state = std::make_unique<ManagerResource>();
    state->manager = manager;
    state->protocol = protocol;
    setImplementation(resource, &managerImplementation, std::move(state));
}

void bindExt(wl_client* client, void* data, uint32_t version, uint32_t id)
{
    bind(client, data, version, id, &ext);
}

void bindWlr(wl_client* client, void* data, uint32_t version, uint32_t id)
{
    bind(client, data, version, id, &wlr);
}

} // namespace

DataControlManager::DataControlManager(wl_display* display, WaylandSeat* seat, Authorize authorize)
    : m_seat(seat)
    , m_authorize(std::move(authorize))
    , m_ext(wl_global_create(display, ext.manager, ext.version, this, bindExt))
    , m_wlr(wl_global_create(display, wlr.manager, wlr.version, this, bindWlr))
{
}

DataControlManager::~DataControlManager()
{
    shutdown();
}

void DataControlManager::shutdown()
{
    if (m_ext)
        wl_global_destroy(m_ext);
    if (m_wlr)
        wl_global_destroy(m_wlr);
    m_ext = nullptr;
    m_wlr = nullptr;
}

} // namespace Lingmo::Wayland
