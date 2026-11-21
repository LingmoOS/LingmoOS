// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// The mpv functions the player calls, forwarding to libmpv once something
// first needs it. Linked, libmpv and the two hundred libraries it pulls in
// would load with every window, though an empty one plays nothing.

#include <dlfcn.h>
#include <string>

#include <mpv/client.h>
#include <mpv/render.h>

static void* library()
{
    static void* const handle = [] {
        const std::string name = "libmpv.so." + std::to_string(MPV_CLIENT_API_VERSION >> 16);
        return dlopen(name.c_str(), RTLD_LAZY | RTLD_LOCAL);
    }();
    return handle;
}

template<typename Function>
static Function* resolve(const char* name)
{
    void* handle = library();
    return handle ? reinterpret_cast<Function*>(dlsym(handle, name)) : nullptr;
}

// Each function looks itself up once; without the library, mpv_create() and
// mpv_render_context_create() fail, and nothing else is reached.
#define MPV_FUNCTION(name) static auto* const real = resolve<decltype(::name)>(#name)

extern "C" {

mpv_handle* mpv_create(void)
{
    MPV_FUNCTION(mpv_create);
    return real ? real() : nullptr;
}

int mpv_initialize(mpv_handle* ctx)
{
    MPV_FUNCTION(mpv_initialize);
    return real ? real(ctx) : MPV_ERROR_GENERIC;
}

void mpv_destroy(mpv_handle* ctx)
{
    MPV_FUNCTION(mpv_destroy);
    if (real)
        real(ctx);
}

void mpv_terminate_destroy(mpv_handle* ctx)
{
    MPV_FUNCTION(mpv_terminate_destroy);
    if (real)
        real(ctx);
}

const char* mpv_error_string(int error)
{
    MPV_FUNCTION(mpv_error_string);
    return real ? real(error) : "libmpv could not be loaded";
}

void mpv_free_node_contents(mpv_node* node)
{
    MPV_FUNCTION(mpv_free_node_contents);
    if (real)
        real(node);
}

int mpv_set_option(mpv_handle* ctx, const char* name, mpv_format format, void* data)
{
    MPV_FUNCTION(mpv_set_option);
    return real ? real(ctx, name, format, data) : MPV_ERROR_GENERIC;
}

int mpv_set_option_string(mpv_handle* ctx, const char* name, const char* data)
{
    MPV_FUNCTION(mpv_set_option_string);
    return real ? real(ctx, name, data) : MPV_ERROR_GENERIC;
}

int mpv_command(mpv_handle* ctx, const char** args)
{
    MPV_FUNCTION(mpv_command);
    return real ? real(ctx, args) : MPV_ERROR_GENERIC;
}

int mpv_command_node_async(mpv_handle* ctx, uint64_t reply_userdata, mpv_node* args)
{
    MPV_FUNCTION(mpv_command_node_async);
    return real ? real(ctx, reply_userdata, args) : MPV_ERROR_GENERIC;
}

int mpv_get_property(mpv_handle* ctx, const char* name, mpv_format format, void* data)
{
    MPV_FUNCTION(mpv_get_property);
    return real ? real(ctx, name, format, data) : MPV_ERROR_GENERIC;
}

int mpv_set_property_async(mpv_handle* ctx, uint64_t reply_userdata, const char* name, mpv_format format,
                           void* data)
{
    MPV_FUNCTION(mpv_set_property_async);
    return real ? real(ctx, reply_userdata, name, format, data) : MPV_ERROR_GENERIC;
}

int mpv_observe_property(mpv_handle* mpv, uint64_t reply_userdata, const char* name, mpv_format format)
{
    MPV_FUNCTION(mpv_observe_property);
    return real ? real(mpv, reply_userdata, name, format) : MPV_ERROR_GENERIC;
}

int mpv_request_log_messages(mpv_handle* ctx, const char* min_level)
{
    MPV_FUNCTION(mpv_request_log_messages);
    return real ? real(ctx, min_level) : MPV_ERROR_GENERIC;
}

mpv_event* mpv_wait_event(mpv_handle* ctx, double timeout)
{
    MPV_FUNCTION(mpv_wait_event);
    static mpv_event shutdown{MPV_EVENT_SHUTDOWN, 0, 0, nullptr};
    return real ? real(ctx, timeout) : &shutdown;
}

void mpv_wakeup(mpv_handle* ctx)
{
    MPV_FUNCTION(mpv_wakeup);
    if (real)
        real(ctx);
}

void mpv_set_wakeup_callback(mpv_handle* ctx, void (*cb)(void* d), void* d)
{
    MPV_FUNCTION(mpv_set_wakeup_callback);
    if (real)
        real(ctx, cb, d);
}

int mpv_render_context_create(mpv_render_context** res, mpv_handle* mpv, mpv_render_param* params)
{
    MPV_FUNCTION(mpv_render_context_create);
    return real ? real(res, mpv, params) : MPV_ERROR_GENERIC;
}

void mpv_render_context_free(mpv_render_context* ctx)
{
    MPV_FUNCTION(mpv_render_context_free);
    if (real)
        real(ctx);
}

int mpv_render_context_render(mpv_render_context* ctx, mpv_render_param* params)
{
    MPV_FUNCTION(mpv_render_context_render);
    return real ? real(ctx, params) : MPV_ERROR_GENERIC;
}

void mpv_render_context_report_swap(mpv_render_context* ctx)
{
    MPV_FUNCTION(mpv_render_context_report_swap);
    if (real)
        real(ctx);
}

void mpv_render_context_set_update_callback(mpv_render_context* ctx, mpv_render_update_fn callback,
                                            void* callback_ctx)
{
    MPV_FUNCTION(mpv_render_context_set_update_callback);
    if (real)
        real(ctx, callback, callback_ctx);
}

uint64_t mpv_render_context_update(mpv_render_context* ctx)
{
    MPV_FUNCTION(mpv_render_context_update);
    return real ? real(ctx) : 0;
}

} // extern "C"
