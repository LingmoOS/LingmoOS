// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wl_data_device_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: keeps the resource and event ordering and the fd
// marshalling, without the Chromium clipboard backend.
#include "wayland/protocols/wl_data_device_manager.h"

#include <algorithm>
#include <memory>
#include <unistd.h>
#include <wayland-server-protocol.h>

#include "clipboard/selection.h"
#include "compositor/surface.h"
#include "input/seat.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/serial_tracker.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

namespace {

class ExternalSource final : public clipboard::DataSource
{
public:
    wl_resource* resource = nullptr;
    bool actionsSet = false;
    bool dragOnly = false;
    uint32_t actions = 1;
    uint32_t lastAction = 0;

    void target(const char* mime) override
    {
        if (resource && alive() && !isClientDestroyed(wl_resource_get_client(resource))) {
            wl_data_source_send_target(resource, mime);
            wl_client_flush(wl_resource_get_client(resource));
        }
    }

    void action(uint32_t value) override
    {
        if (resource && alive() && !isClientDestroyed(wl_resource_get_client(resource)) &&
            wl_resource_get_version(resource) >= 3 && lastAction != value) {
            lastAction = value;
            wl_data_source_send_action(resource, value);
            wl_client_flush(wl_resource_get_client(resource));
        }
    }

    void dropPerformed() override
    {
        if (resource && alive() && !isClientDestroyed(wl_resource_get_client(resource)) &&
            wl_resource_get_version(resource) >= 3) {
            wl_data_source_send_dnd_drop_performed(resource);
            wl_client_flush(wl_resource_get_client(resource));
        }
    }

    void finished() override
    {
        if (resource && alive() && !isClientDestroyed(wl_resource_get_client(resource)) &&
            wl_resource_get_version(resource) >= 3) {
            wl_data_source_send_dnd_finished(resource);
            wl_client_flush(wl_resource_get_client(resource));
        }
    }

    void send(const std::string& mime, int fd) override
    {
        if (resource && alive() && supports(mime) &&
            !isClientDestroyed(wl_resource_get_client(resource))) {
            wl_data_source_send_send(resource, mime.c_str(), fd);
            wl_client_flush(wl_resource_get_client(resource));
        }
        // libwayland marshaling retains its own fd. Never retain the request fd.
        close(fd);
    }

    void cancelled() override
    {
        if (resource && !isClientDestroyed(wl_resource_get_client(resource))) {
            wl_data_source_send_cancelled(resource);
            wl_client_flush(wl_resource_get_client(resource));
        }
    }
};

struct SourceResource {
    std::shared_ptr<ExternalSource> source = std::make_shared<ExternalSource>();

    ~SourceResource()
    {
        source->resource = nullptr;
        source->invalidate();
    }
};

void sourceOffer(wl_client*, wl_resource* resource, const char* mime)
{
    getUserDataAs<SourceResource>(resource)->source->offer(mime);
}

void destroyResource(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void sourceActions(wl_client*, wl_resource* resource, uint32_t actions)
{
    auto source = getUserDataAs<SourceResource>(resource)->source;
    if (actions & ~7u) {
        wl_resource_post_error(resource, WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,
                               "invalid action mask");
        return;
    }
    if (source->actionsSet || source->used()) {
        wl_resource_post_error(resource, WL_DATA_SOURCE_ERROR_INVALID_SOURCE,
                               "source already configured or used");
        return;
    }
    source->actionsSet = true;
    source->actions = actions;
}

const struct wl_data_source_interface sourceImplementation = {sourceOffer, destroyResource,
                                                              sourceActions};

struct OfferResource {
    OfferResource(clipboard::SelectionController* controller, std::weak_ptr<bool> device)
        : offer(std::make_shared<clipboard::DataOffer>(controller))
        , device(std::move(device))
    {
    }

    explicit OfferResource(std::shared_ptr<input::DragOffer> drag, std::weak_ptr<bool> device)
        : drag(std::move(drag))
        , device(std::move(device))
    {
    }

    ~OfferResource()
    {
        actionConnection.reset();
        if (drag) {
            if (drag->legacy() && drag->state() == input::DragOffer::State::Dropped)
                drag->finish();
            drag->invalidate();
        }
    }

    base::Subscription actionConnection;
    std::shared_ptr<clipboard::DataOffer> offer;
    std::shared_ptr<input::DragOffer> drag;
    std::weak_ptr<bool> device;
};

void offerAccept(wl_client*, wl_resource* resource, uint32_t, const char* mime)
{
    auto* offer = getUserDataAs<OfferResource>(resource);
    if (offer->drag && !offer->device.expired()) {
        if (offer->drag->state() == input::DragOffer::State::Finished) {
            wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_OFFER,
                                   "offer already finished");
            return;
        }
        offer->drag->accept(mime);
    }
}

void offerReceive(wl_client*, wl_resource* resource, const char* mime, int fd)
{
    auto* offer = getUserDataAs<OfferResource>(resource);
    if (offer->device.expired()) {
        close(fd);
        return;
    }
    if (offer->drag && offer->drag->state() == input::DragOffer::State::Finished) {
        close(fd);
        wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_OFFER,
                               "offer already finished");
        return;
    }
    if (offer->drag)
        offer->drag->receive(mime, fd);
    else
        offer->offer->receive(mime, fd);
}

void offerFinish(wl_client*, wl_resource* resource)
{
    auto* offer = getUserDataAs<OfferResource>(resource);
    if (!offer->drag || offer->device.expired() || !offer->drag->finish())
        wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_FINISH,
                               "offer has no accepted, completed drop");
}

void offerActions(wl_client*, wl_resource* resource, uint32_t actions, uint32_t preferred)
{
    auto* offer = getUserDataAs<OfferResource>(resource);
    if (!(actions & ~7u) && !(preferred & ~actions) &&
        !(preferred && (preferred & (preferred - 1))) && offer->drag) {
        if (offer->drag->state() == input::DragOffer::State::Finished) {
            wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_OFFER,
                                   "offer already finished");
            return;
        }
        if (offer->drag->state() == input::DragOffer::State::Dropped &&
            offer->drag->action() == 4 && preferred &&
            !(preferred & offer->drag->sourceActions())) {
            wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_ACTION,
                                   "action is not offered by source");
            return;
        }
        if (!offer->device.expired())
            offer->drag->setActions(actions, preferred);
        return;
    }
    const uint32_t error = actions & ~7u ? WL_DATA_OFFER_ERROR_INVALID_ACTION_MASK
        : ((preferred & ~actions) || (preferred && (preferred & (preferred - 1))))
        ? WL_DATA_OFFER_ERROR_INVALID_ACTION
        : WL_DATA_OFFER_ERROR_INVALID_OFFER;
    wl_resource_post_error(resource, error, "selection does not accept drag actions");
}

const struct wl_data_offer_interface offerImplementation = {
    offerAccept, offerReceive, destroyResource, offerFinish, offerActions};

class DeviceResource
{
public:
    DeviceResource(wl_resource* resourceArg, clipboard::SelectionController* controller,
                   WaylandSeat* seat)
        : resource(resourceArg)
        , controller(controller ? controller->weakPtr() : nullptr)
        , seat(seat)
    {
        if (!controller)
            return;
        auto* drag = seat->seat->drag();
        dragConnections.push_back(drag->ended.connect([this] {
            ownsDrag = false;
        }));
        dragConnections.push_back(drag->entered.connect(
                [this](compositor::Surface* surface, const base::PointF& location) {
                    enter(surface, location);
                }));
        dragConnections.push_back(drag->moved.connect(
                [this](input::Timestamp timestamp, const base::PointF& location) {
                    if (dragOffer && !isClientDestroyed(wl_resource_get_client(resource))) {
                        wl_data_device_send_motion(resource, timePointToMilliseconds(timestamp),
                                                   wl_fixed_from_double(location.x()),
                                                   wl_fixed_from_double(location.y()));
                        wl_client_flush(wl_resource_get_client(resource));
                    }
                }));
        dragConnections.push_back(drag->left.connect([this] {
            if (dragOffer) {
                dragOffer.reset();
                if (!isClientDestroyed(wl_resource_get_client(resource))) {
                    wl_data_device_send_leave(resource);
                    wl_client_flush(wl_resource_get_client(resource));
                }
            }
        }));
        dragConnections.push_back(drag->dropped.connect([this](input::DragOffer* offer) {
            if (dragOffer.get() == offer && !isClientDestroyed(wl_resource_get_client(resource))) {
                wl_data_device_send_drop(resource);
                wl_client_flush(wl_resource_get_client(resource));
            }
        }));
        selectionChanged = controller->selectionChanged.connect([this] {
            deliver();
        });
        recipientChanged = controller->recipientChanged.connect([this] {
            deliver();
        });
    }

    ~DeviceResource()
    {
        dragConnections.clear();
        lifetime.reset();
        for (auto& weak : offers) {
            if (auto offer = weak.lock())
                offer->invalidate();
        }
        for (auto& weak : selectionOffers) {
            if (auto offer = weak.lock())
                offer->invalidate();
        }
        if (ownsDrag && seat && seat->seat->drag()->active())
            seat->seat->drag()->cancel();
    }

    void enter(compositor::Surface* surface, const base::PointF& location)
    {
        auto* surfaceResource = getSurfaceResource(surface);
        auto* client = wl_resource_get_client(resource);
        if (isClientDestroyed(client) || !surfaceResource ||
            wl_resource_get_client(surfaceResource) != client)
            return;
        auto* drag = seat->seat->drag();
        if (!drag->hasSource()) {
            auto* origin = getSurfaceResource(drag->origin());
            if (!origin || wl_resource_get_client(origin) != client)
                return;
        }
        dragOffer = drag->createOffer(wl_resource_get_version(resource) < 3);
        std::erase_if(offers, [](const auto& offer) {
            return offer.expired();
        });
        offers.push_back(dragOffer);
        wl_resource* offer = nullptr;
        if (drag->hasSource()) {
            offer = wl_resource_create(client, &wl_data_offer_interface,
                                       wl_resource_get_version(resource), 0);
            if (!offer) {
                dragOffer.reset();
                wl_client_post_no_memory(client);
                return;
            }
            auto data = std::make_unique<OfferResource>(dragOffer, lifetime);
            setImplementation(offer, &offerImplementation, std::move(data));
            wl_data_device_send_data_offer(resource, offer);
            for (const auto& mime : dragOffer->formats())
                wl_data_offer_send_offer(offer, mime.c_str());
            if (wl_resource_get_version(offer) >= 3) {
                wl_data_offer_send_source_actions(offer, dragOffer->sourceActions());
                wl_data_offer_send_action(offer, dragOffer->action());
                getUserDataAs<OfferResource>(offer)->actionConnection =
                    dragOffer->actionChanged.connect([offer](uint32_t action) {
                        wl_data_offer_send_action(offer, action);
                        wl_client_flush(wl_resource_get_client(offer));
                    });
            }
        }
        wl_data_device_send_enter(
            resource, seat->serialTracker->nextSerial(SerialTracker::OTHER_EVENT), surfaceResource,
            wl_fixed_from_double(location.x()), wl_fixed_from_double(location.y()), offer);
        wl_client_flush(client);
    }

    void deliver()
    {
        if (!controller || isClientDestroyed(wl_resource_get_client(resource)) ||
            controller->recipient() != wl_resource_get_client(resource))
            return;
        wl_resource* offer = nullptr;
        if (controller->source()) {
            offer = wl_resource_create(wl_resource_get_client(resource), &wl_data_offer_interface,
                                       wl_resource_get_version(resource), 0);
            if (!offer) {
                wl_client_post_no_memory(wl_resource_get_client(resource));
                return;
            }
            auto data = std::make_unique<OfferResource>(controller.get(), lifetime);
            const auto formats = data->offer->formats();
            std::erase_if(selectionOffers, [](const auto& offer) {
                return offer.expired();
            });
            selectionOffers.push_back(data->offer);
            setImplementation(offer, &offerImplementation, std::move(data));
            wl_data_device_send_data_offer(resource, offer);
            for (const auto& mime : formats)
                wl_data_offer_send_offer(offer, mime.c_str());
        }
        wl_data_device_send_selection(resource, offer);
        wl_client_flush(wl_resource_get_client(resource));
    }

    WaylandSeat* seat;
    bool ownsDrag = false;
    std::shared_ptr<input::DragOffer> dragOffer;
    std::vector<std::weak_ptr<input::DragOffer>> offers;
    std::vector<std::weak_ptr<clipboard::DataOffer>> selectionOffers;
    std::vector<base::Subscription> dragConnections;
    wl_resource* resource;
    base::WeakPtr<clipboard::SelectionController> controller;
    std::shared_ptr<bool> lifetime = std::make_shared<bool>(true);
    base::Subscription selectionChanged;
    base::Subscription recipientChanged;
};

std::shared_ptr<ExternalSource> getSource(wl_client* client, wl_resource* resource)
{
    if (!resource || wl_resource_get_client(resource) != client ||
        !wl_resource_instance_of(resource, &wl_data_source_interface, &sourceImplementation))
        return nullptr;
    return getUserDataAs<SourceResource>(resource)->source;
}

bool unused(wl_resource* device, const std::shared_ptr<ExternalSource>& source)
{
    if (source && (source->used() || !source->alive())) {
        wl_resource_post_error(device, WL_DATA_DEVICE_ERROR_USED_SOURCE,
                               "source has already been used");
        return false;
    }
    return true;
}

void startDrag(wl_client* client, wl_resource* resource, wl_resource* sourceResource,
               wl_resource* originResource, wl_resource* iconResource, uint32_t serial)
{
    auto* device = getUserDataAs<DeviceResource>(resource);
    auto source = getSource(client, sourceResource);
    if ((sourceResource && !source) || !unused(resource, source))
        return;
    auto* origin = surfaceFromResource(originResource);
    auto* icon = surfaceFromResource(iconResource);
    if (icon && icon->hasRole()) {
        wl_resource_post_error(resource, WL_DATA_DEVICE_ERROR_ROLE, "icon already has a role");
        return;
    }
    auto* seat = device->seat ? device->seat->seat : nullptr;
    const auto button = seat ? seat->dragSerial(origin, serial) : input::NoButton;
    // A touch-down serial drags with the finger still down on the origin.
    std::optional<input::Seat::TouchPoint> touch;
    if (seat && !button
        && device->seat->serialTracker->eventType(serial) == SerialTracker::TOUCH_DOWN)
        touch = seat->touchPointOn(origin);
    if (!seat || !device->controller || !origin ||
        wl_resource_get_client(originResource) != client || (!button && !touch) ||
        !origin->isMapped() || seat->drag()->busy() || seat->cursorOverride() || icon == origin) {
        if (source) {
            source->markUsed();
            source->cancel();
        }
        return;
    }
    device->ownsDrag = true;
    if (touch) {
        seat->drag()->startTouch(origin, icon, source, source ? source->actions : 0, touch->id,
                                 touch->location);
    } else {
        seat->drag()->start(origin, icon, source, source ? source->actions : 0, button);
    }
}

void setSelection(wl_client* client, wl_resource* resource, wl_resource* sourceResource,
                  uint32_t serial)
{
    auto* device = getUserDataAs<DeviceResource>(resource);
    auto source = getSource(client, sourceResource);
    if (sourceResource && !source)
        return;
    // GTK 3 repeats set_selection for the current source when adding SAVE_TARGETS.
    // The protocol permits (but does not require) used_source here. Treat this
    // exact repeat as a no-op; never repurpose or revive a used source.
    if (source && device->controller && device->controller->source() == source)
        return;
    if (!unused(resource, source))
        return;
    if (source && (source->actionsSet || source->dragOnly)) {
        wl_resource_post_error(sourceResource, WL_DATA_SOURCE_ERROR_INVALID_SOURCE,
                               "drag source cannot be a selection");
        return;
    }
    // Security policy refusal is not a protocol error. The source stays unused.
    if (!device->controller || !device->controller->authorize(client, serial))
        return;
    device->controller->setSource(std::move(source));
}

const struct wl_data_device_interface deviceImplementation = {startDrag, setSelection,
                                                              destroyResource};

void createSource(wl_client* client, wl_resource* resource, uint32_t id)
{
    auto* source = wl_resource_create(client, &wl_data_source_interface,
                                      wl_resource_get_version(resource), id);
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
    auto* seat = getUserDataAs<WaylandSeat>(resource);
    auto* requested = waylandSeatFromResource(seatResource);
    auto* device = wl_resource_create(client, &wl_data_device_interface,
                                      wl_resource_get_version(resource), id);
    if (!device) {
        wl_client_post_no_memory(client);
        return;
    }
    // Ineligible seats receive an inert device, not an invented protocol error.
    auto* controller = requested == seat && wl_resource_get_client(seatResource) == client
        ? seat->seat->selection()
        : nullptr;
    auto data =
        std::make_unique<DeviceResource>(device, controller, requested == seat ? seat : nullptr);
    auto* raw = data.get();
    setImplementation(device, &deviceImplementation, std::move(data));
    raw->deliver();
}

const struct wl_data_device_manager_interface managerImplementation = {createSource, getDevice};

void bind(wl_client* client, void* data, uint32_t version, uint32_t id)
{
    auto* resource = wl_resource_create(client, &wl_data_device_manager_interface,
                                        std::min(version, kWlDataDeviceManagerVersion), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &managerImplementation, data, nullptr);
}

} // namespace

std::shared_ptr<clipboard::DataSource> claimDragSource(wl_client* client, wl_resource* resource)
{
    auto source = getSource(client, resource);
    if (!source || source->used() || !source->alive() || source->dragOnly)
        return nullptr;
    source->dragOnly = true;
    return source;
}

wl_global* createDataDeviceManagerGlobal(wl_display* display, WaylandSeat* seat)
{
    return wl_global_create(display, &wl_data_device_manager_interface, kWlDataDeviceManagerVersion,
                            seat, bind);
}

} // namespace Lingmo::Wayland
