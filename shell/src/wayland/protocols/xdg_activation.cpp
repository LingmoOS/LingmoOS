// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/protocols/xdg_activation.h"

#include <xdg-activation-v1-server-protocol.h>

#include "base/uuid.h"
#include "compositor/surface.h"
#include "input/seat.h"
#include "wayland/protocols/wl_seat.h"
#include "wayland/resource_reference.h"
#include "wayland/serial_tracker.h"
#include "wayland/server_util.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

namespace Lingmo::Wayland {
namespace {

constexpr size_t kMaxTokens = 128;
constexpr auto kTokenLifetime = std::chrono::seconds(10);

struct TokenRequest {
    WaylandXdgActivation *activation;
    ResourceReference seat;
    ResourceReference surface;
    uint32_t serial = 0;
    bool surfaceWasSet = false;
    bool committed = false;

    bool writable(wl_resource *resource) const
    {
        if (!committed)
            return true;
        wl_resource_post_error(resource, XDG_ACTIVATION_TOKEN_V1_ERROR_ALREADY_USED,
                               "activation token has already been committed");
        return false;
    }
};

void destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void setSerial(wl_client *, wl_resource *resource, uint32_t serial, wl_resource *seat)
{
    auto *request = getUserDataAs<TokenRequest>(resource);
    if (request->writable(resource)) {
        request->serial = serial;
        request->seat.reset(seat);
    }
}

void setAppId(wl_client *, wl_resource *resource, const char *)
{
    // app_id is a startup-notification hint, not proof of the target's identity.
    getUserDataAs<TokenRequest>(resource)->writable(resource);
}

void setSurface(wl_client *, wl_resource *resource, wl_resource *surface)
{
    auto *request = getUserDataAs<TokenRequest>(resource);
    if (request->writable(resource)) {
        request->surfaceWasSet = true;
        request->surface.reset(surface);
    }
}

void commit(wl_client *client, wl_resource *resource)
{
    auto *request = getUserDataAs<TokenRequest>(resource);
    if (!request->writable(resource))
        return;
    request->committed = true;
    try {
        const auto token =
                request->activation->createToken(client, request->seat.get(), request->serial,
                                                 request->surface.get(), request->surfaceWasSet);
        xdg_activation_token_v1_send_done(resource, token.c_str());
    } catch (const std::bad_alloc &) {
        wl_client_post_no_memory(client);
    }
}

const struct xdg_activation_token_v1_interface tokenImplementation = {setSerial, setAppId,
                                                                      setSurface, commit, destroy};

void getToken(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *token = wl_resource_create(client, &xdg_activation_token_v1_interface, 1, id);
    if (!token) {
        wl_client_post_no_memory(client);
        return;
    }
    try {
        auto request = std::make_unique<TokenRequest>();
        request->activation = getUserDataAs<WaylandXdgActivation>(resource);
        setImplementation(token, &tokenImplementation, std::move(request));
    } catch (const std::bad_alloc &) {
        wl_resource_destroy(token);
        wl_client_post_no_memory(client);
    }
}

void activate(wl_client *client, wl_resource *resource, const char *token, wl_resource *surface)
{
    try {
        getUserDataAs<WaylandXdgActivation>(resource)->activate(token,
                                                                surfaceFromResource(surface));
    } catch (const std::bad_alloc &) {
        wl_client_post_no_memory(client);
    }
}

const struct xdg_activation_v1_interface activationImplementation = {destroy, getToken, activate};

void bind(wl_client *client, void *data, uint32_t, uint32_t id)
{
    auto *resource = wl_resource_create(client, &xdg_activation_v1_interface, 1, id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &activationImplementation, data, nullptr);
}

} // namespace

WaylandXdgActivation::WaylandXdgActivation(wm::WindowManager *manager, WaylandSeat *seat)
    : m_manager(manager)
    , m_seat(seat)
{
}

WaylandXdgActivation::~WaylandXdgActivation()
{
    for (const auto &[surface, deadline] : m_pending)
        surface->removeSurfaceObserver(this);
}

void WaylandXdgActivation::prune()
{
    const auto now = Clock::now();
    std::erase_if(m_tokens, [now](const auto &entry) {
        return entry.second <= now;
    });
    for (auto it = m_pending.begin(); it != m_pending.end();) {
        if (it->second <= now) {
            it->first->removeSurfaceObserver(this);
            it = m_pending.erase(it);
        } else {
            ++it;
        }
    }
}

std::string WaylandXdgActivation::createToken(wl_client *client, wl_resource *seat, uint32_t serial,
                                              wl_resource *surface, bool surfaceWasSet)
{
    prune();
    const auto token = base::Uuid::generate().toString();
    auto *focused = m_seat->seat->focusedSurface();
    auto *focusedResource = getSurfaceResource(focused);
    if (!seat || waylandSeatFromResource(seat) != m_seat || !focusedResource
        || wl_resource_get_client(focusedResource) != client || m_tokens.size() >= kMaxTokens)
        return token;
    if (surfaceWasSet) {
        auto *source = surface ? surfaceFromResource(surface) : nullptr;
        if (!source
            || (source != focused
                && (!m_manager->windowForSurface(source)
                    || m_manager->windowForSurface(source)
                            != m_manager->windowForSurface(focused))))
            return token;
    }
    if (m_seat->serialTracker->consumeActivationSerial(serial, client))
        m_tokens.emplace(token, Clock::now() + kTokenLifetime);
    return token;
}

std::string WaylandXdgActivation::createShellToken()
{
    prune();
    const auto token = base::Uuid::generate().toString();
    if (m_tokens.size() < kMaxTokens)
        m_tokens.emplace(token, Clock::now() + kTokenLifetime);
    return token;
}

bool WaylandXdgActivation::activateMapped(compositor::Surface *surface)
{
    auto *window = m_manager->windowForSurface(surface);
    if (!window)
        return false;
    if (window->rootSurface() == surface && !window->isPopup() && !window->isShellWindow()) {
        // Switching to a hidden application brings all of its windows back.
        m_manager->unhideApplication(window);
        window->setHidden(false);
        if (window->state()->isMinimized())
            window->restore();
        m_manager->activation()->activate(window);
    }
    return true;
}

void WaylandXdgActivation::activate(const char *token, compositor::Surface *surface)
{
    prune();
    const auto it = m_tokens.find(token);
    if (it == m_tokens.end())
        return;
    const auto deadline = it->second;
    m_tokens.erase(it);
    if (!surface || activateMapped(surface) || m_pending.size() >= kMaxTokens)
        return;
    m_pending[surface] = deadline;
    surface->addSurfaceObserver(this);
}

void WaylandXdgActivation::onSurfaceDestroying(compositor::Surface *surface)
{
    m_pending.erase(surface);
    surface->removeSurfaceObserver(this);
}

void WaylandXdgActivation::onSurfaceCommit(compositor::Surface *surface)
{
    prune();
    if (m_pending.contains(surface) && activateMapped(surface))
        onSurfaceDestroying(surface);
}

wl_global *createXdgActivationGlobal(wl_display *display, WaylandXdgActivation *activation)
{
    return wl_global_create(display, &xdg_activation_v1_interface, 1, activation, bind);
}

} // namespace Lingmo::Wayland
