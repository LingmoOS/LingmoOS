namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayland/primary_selection.h"

#include <wayland-client.h>

#include <primary-selection-unstable-v1-client-protocol.h>

#include "wayland/connection.h"

namespace Lingmo::ui::wayland {

struct PrimarySelectionListeners
{
    static void offerMimeType(void* data, zwp_primary_selection_offer_v1* offer, const char* type)
    {
        static_cast<PrimarySelection*>(data)->offerMimeType(offer, type);
    }

    static void dataOffer(void* data, zwp_primary_selection_device_v1*, zwp_primary_selection_offer_v1* offer)
    {
        auto* self = static_cast<PrimarySelection*>(data);
        self->offerAnnounced(offer);
        zwp_primary_selection_offer_v1_add_listener(offer, &PrimarySelectionListeners::offer, self);
    }

    static void selection(void* data, zwp_primary_selection_device_v1*, zwp_primary_selection_offer_v1* offer)
    {
        static_cast<PrimarySelection*>(data)->selectionChanged(offer);
    }

    static void sourceSend(void* data, zwp_primary_selection_source_v1* source, const char* type, int32_t fd)
    {
        static_cast<PrimarySelection*>(data)->sourceSend(source, type, fd);
    }

    static void sourceCancelled(void* data, zwp_primary_selection_source_v1* source)
    {
        static_cast<PrimarySelection*>(data)->sourceCancelled(source);
    }

    static constexpr zwp_primary_selection_offer_v1_listener offer = {offerMimeType};
    static constexpr zwp_primary_selection_device_v1_listener device = {dataOffer, selection};
    static constexpr zwp_primary_selection_source_v1_listener source = {sourceSend, sourceCancelled};
};

PrimarySelection::PrimarySelection(Connection& connection, base::EventLoop& loop,
                                   zwp_primary_selection_device_manager_v1* manager, wl_seat* seat)
    : Selection(connection, loop, ClipboardMode::Selection)
    , m_manager(manager)
    , m_device(zwp_primary_selection_device_manager_v1_get_device(manager, seat))
{
    zwp_primary_selection_device_v1_add_listener(m_device, &PrimarySelectionListeners::device, this);
}

PrimarySelection::~PrimarySelection()
{
    releaseAll();
    zwp_primary_selection_device_v1_destroy(m_device);
}

void* PrimarySelection::createSource(const std::vector<std::string>& mimeTypes)
{
    zwp_primary_selection_source_v1* source = zwp_primary_selection_device_manager_v1_create_source(m_manager);
    zwp_primary_selection_source_v1_add_listener(source, &PrimarySelectionListeners::source, this);
    for (const std::string& type : mimeTypes)
        zwp_primary_selection_source_v1_offer(source, type.c_str());
    return source;
}

void PrimarySelection::destroySource(void* source)
{
    zwp_primary_selection_source_v1_destroy(static_cast<zwp_primary_selection_source_v1*>(source));
}

void PrimarySelection::setSource(void* source)
{
    zwp_primary_selection_device_v1_set_selection(
        m_device, static_cast<zwp_primary_selection_source_v1*>(source), m_connection.inputSerial());
}

void PrimarySelection::receive(void* offer, const char* mimeType, int fd)
{
    zwp_primary_selection_offer_v1_receive(static_cast<zwp_primary_selection_offer_v1*>(offer), mimeType, fd);
}

void PrimarySelection::destroyOffer(void* offer)
{
    zwp_primary_selection_offer_v1_destroy(static_cast<zwp_primary_selection_offer_v1*>(offer));
}

} // namespace Lingmo::ui::wayland
