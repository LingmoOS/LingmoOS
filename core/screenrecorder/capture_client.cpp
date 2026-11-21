// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "capture_client.h"

#include <algorithm>
#include <cstring>

#include <wayland-client.h>

#include "ext-foreign-toplevel-list-v1-client-protocol.h"
#include "ext-image-capture-source-v1-client-protocol.h"
#include "ext-image-copy-capture-v1-client-protocol.h"
#include "linux-dmabuf-v1-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"

// Only a release tells the compositor; a plain destroy leaves the server
// object until the connection closes.
static void releaseOutput(const CaptureClient::Output &output)
{
    if (output.xdgOutput)
        zxdg_output_v1_destroy(output.xdgOutput);
    if (wl_output_get_version(output.output) >= WL_OUTPUT_RELEASE_SINCE_VERSION)
        wl_output_release(output.output);
    else
        wl_output_destroy(output.output);
}

struct CaptureClientListeners {
    static void global(void *data, wl_registry *registry, uint32_t name, const char *interface,
                       uint32_t version)
    {
        auto *self = static_cast<CaptureClient *>(data);
        if (!strcmp(interface, wl_output_interface.name)) {
            auto output = std::make_unique<CaptureClient::Output>();
            output->globalName = name;
            output->output = static_cast<wl_output *>(
                    wl_registry_bind(registry, name, &wl_output_interface, std::min(version, 4u)));
            wl_output_add_listener(output->output, &s_outputListener, output.get());
            if (self->m_xdgOutputManager)
                watchOutput(self, output.get());
            self->m_outputs.push_back(std::move(output));
        } else if (!strcmp(interface, wl_shm_interface.name)) {
            // Version 2 has release, so the compositor can drop it with us.
            self->m_shm = static_cast<wl_shm *>(
                    wl_registry_bind(registry, name, &wl_shm_interface, std::min(version, 2u)));
        } else if (!strcmp(interface, zxdg_output_manager_v1_interface.name)) {
            self->m_xdgOutputManager = static_cast<zxdg_output_manager_v1 *>(wl_registry_bind(
                    registry, name, &zxdg_output_manager_v1_interface, std::min(version, 3u)));
            for (auto &output : self->m_outputs)
                watchOutput(self, output.get());
        } else if (!strcmp(interface, ext_output_image_capture_source_manager_v1_interface.name)) {
            self->m_outputSources = static_cast<ext_output_image_capture_source_manager_v1 *>(
                    wl_registry_bind(registry, name,
                                     &ext_output_image_capture_source_manager_v1_interface, 1));
        } else if (!strcmp(interface,
                           ext_foreign_toplevel_image_capture_source_manager_v1_interface.name)) {
            self->m_toplevelSources =
                    static_cast<ext_foreign_toplevel_image_capture_source_manager_v1 *>(
                            wl_registry_bind(
                                    registry, name,
                                    &ext_foreign_toplevel_image_capture_source_manager_v1_interface,
                                    1));
        } else if (!strcmp(interface, ext_image_copy_capture_manager_v1_interface.name)) {
            self->m_copyManager = static_cast<ext_image_copy_capture_manager_v1 *>(wl_registry_bind(
                    registry, name, &ext_image_copy_capture_manager_v1_interface, 1));
        } else if (!strcmp(interface, zwp_linux_dmabuf_v1_interface.name)) {
            // Version 3: create_immed, without the feedback objects of 4.
            self->m_dmabuf = static_cast<zwp_linux_dmabuf_v1 *>(wl_registry_bind(
                    registry, name, &zwp_linux_dmabuf_v1_interface, std::min(version, 3u)));
        } else if (!strcmp(interface, ext_foreign_toplevel_list_v1_interface.name)) {
            self->m_toplevelList = static_cast<ext_foreign_toplevel_list_v1 *>(wl_registry_bind(
                    registry, name, &ext_foreign_toplevel_list_v1_interface, 1));
            ext_foreign_toplevel_list_v1_add_listener(self->m_toplevelList, &s_listListener, self);
        }
    }

    static void globalRemove(void *data, wl_registry *, uint32_t name)
    {
        auto *self = static_cast<CaptureClient *>(data);
        auto it = std::find_if(self->m_outputs.begin(), self->m_outputs.end(),
                               [name](const auto &output) {
                                   return output->globalName == name;
                               });
        if (it == self->m_outputs.end())
            return;
        releaseOutput(**it);
        self->m_outputs.erase(it);
    }

    static void watchOutput(CaptureClient *self, CaptureClient::Output *output)
    {
        if (output->xdgOutput)
            return;
        output->xdgOutput =
                zxdg_output_manager_v1_get_xdg_output(self->m_xdgOutputManager, output->output);
        zxdg_output_v1_add_listener(output->xdgOutput, &s_xdgOutputListener, output);
    }

    static void toplevel(void *data, ext_foreign_toplevel_list_v1 *,
                         ext_foreign_toplevel_handle_v1 *handle)
    {
        auto *self = static_cast<CaptureClient *>(data);
        self->dropClosedToplevels();
        auto toplevel = std::make_unique<CaptureClient::Toplevel>();
        toplevel->handle = handle;
        ext_foreign_toplevel_handle_v1_add_listener(handle, &s_handleListener, toplevel.get());
        self->m_toplevels.push_back(std::move(toplevel));
    }

    static void listFinished(void *data, ext_foreign_toplevel_list_v1 *list)
    {
        auto *self = static_cast<CaptureClient *>(data);
        ext_foreign_toplevel_list_v1_destroy(list);
        self->m_toplevelList = nullptr;
    }

    static void handleClosed(void *data, ext_foreign_toplevel_handle_v1 *handle)
    {
        auto *toplevel = static_cast<CaptureClient::Toplevel *>(data);
        ext_foreign_toplevel_handle_v1_destroy(handle);
        toplevel->handle = nullptr;
    }

    static void handleIdentifier(void *data, ext_foreign_toplevel_handle_v1 *,
                                 const char *identifier)
    {
        static_cast<CaptureClient::Toplevel *>(data)->identifier = identifier;
    }

    static const wl_registry_listener s_registryListener;
    static const wl_output_listener s_outputListener;
    static const zxdg_output_v1_listener s_xdgOutputListener;
    static const ext_foreign_toplevel_list_v1_listener s_listListener;
    static const ext_foreign_toplevel_handle_v1_listener s_handleListener;
};

const wl_registry_listener CaptureClientListeners::s_registryListener = {
        CaptureClientListeners::global,
        CaptureClientListeners::globalRemove,
};

const wl_output_listener CaptureClientListeners::s_outputListener = {
        [](void *, wl_output *, int32_t, int32_t, int32_t, int32_t, int32_t, const char *,
           const char *, int32_t) {},
        [](void *, wl_output *, uint32_t, int32_t, int32_t, int32_t) {},
        [](void *, wl_output *) {},
        [](void *, wl_output *, int32_t) {},
        [](void *data, wl_output *, const char *name) {
            static_cast<CaptureClient::Output *>(data)->name = name;
        },
        [](void *, wl_output *, const char *) {},
};

const zxdg_output_v1_listener CaptureClientListeners::s_xdgOutputListener = {
        [](void *data, zxdg_output_v1 *, int32_t x, int32_t y) {
            static_cast<CaptureClient::Output *>(data)->logicalGeometry.moveTopLeft(
                    Lingmo::Base::Point(x, y));
        },
        [](void *data, zxdg_output_v1 *, int32_t width, int32_t height) {
            static_cast<CaptureClient::Output *>(data)->logicalGeometry.setSize(width, height);
        },
        [](void *, zxdg_output_v1 *) {},
        [](void *, zxdg_output_v1 *, const char *) {},
        [](void *, zxdg_output_v1 *, const char *) {},
};

const ext_foreign_toplevel_list_v1_listener CaptureClientListeners::s_listListener = {
        CaptureClientListeners::toplevel,
        CaptureClientListeners::listFinished,
};

const ext_foreign_toplevel_handle_v1_listener CaptureClientListeners::s_handleListener = {
        CaptureClientListeners::handleClosed,
        [](void *, ext_foreign_toplevel_handle_v1 *) {},
        [](void *, ext_foreign_toplevel_handle_v1 *, const char *) {},
        [](void *, ext_foreign_toplevel_handle_v1 *, const char *) {},
        CaptureClientListeners::handleIdentifier,
};

CaptureClient::CaptureClient(wl_display *display)
    : m_display(display)
{
}

CaptureClient::~CaptureClient()
{
    for (auto &toplevel : m_toplevels) {
        if (toplevel->handle)
            ext_foreign_toplevel_handle_v1_destroy(toplevel->handle);
    }
    if (m_toplevelList)
        ext_foreign_toplevel_list_v1_destroy(m_toplevelList);
    for (const auto &output : m_outputs)
        releaseOutput(*output);
    if (m_copyManager)
        ext_image_copy_capture_manager_v1_destroy(m_copyManager);
    if (m_dmabuf)
        zwp_linux_dmabuf_v1_destroy(m_dmabuf);
    if (m_toplevelSources)
        ext_foreign_toplevel_image_capture_source_manager_v1_destroy(m_toplevelSources);
    if (m_outputSources)
        ext_output_image_capture_source_manager_v1_destroy(m_outputSources);
    if (m_xdgOutputManager)
        zxdg_output_manager_v1_destroy(m_xdgOutputManager);
    if (m_shm) {
        if (wl_shm_get_version(m_shm) >= WL_SHM_RELEASE_SINCE_VERSION)
            wl_shm_release(m_shm);
        else
            wl_shm_destroy(m_shm);
    }
    if (m_registry)
        wl_registry_destroy(m_registry);
    if (m_display)
        wl_display_flush(m_display);
}

bool CaptureClient::initialize()
{
    if (!m_display)
        return false;
    if (!m_registry) {
        m_registry = wl_display_get_registry(m_display);
        wl_registry_add_listener(m_registry, &CaptureClientListeners::s_registryListener, this);
        // Globals first, then what the bound objects announce about themselves.
        roundtrip();
        roundtrip();
    }
    return canCaptureOutputs();
}

bool CaptureClient::canCaptureOutputs() const
{
    return m_shm && m_outputSources && m_copyManager;
}

bool CaptureClient::canCaptureWindows() const
{
    return m_shm && m_toplevelSources && m_copyManager && m_toplevelList;
}

const CaptureClient::Output *CaptureClient::outputNamed(const std::string &name) const
{
    for (const auto &output : m_outputs) {
        if (output->name == name)
            return output.get();
    }
    return nullptr;
}

ext_image_capture_source_v1 *CaptureClient::createOutputSource(const std::string &outputName)
{
    const Output *output = outputNamed(outputName);
    if (!output || !m_outputSources)
        return nullptr;
    return ext_output_image_capture_source_manager_v1_create_source(m_outputSources,
                                                                    output->output);
}

ext_image_capture_source_v1 *CaptureClient::createWindowSource(const std::string &identifier)
{
    if (!m_toplevelSources || identifier.empty())
        return nullptr;
    // A window mapped since the last dispatch may not have been announced yet.
    for (int attempt = 0; attempt < 2; ++attempt) {
        for (const auto &toplevel : m_toplevels) {
            if (toplevel->handle && toplevel->identifier == identifier)
                return ext_foreign_toplevel_image_capture_source_manager_v1_create_source(
                        m_toplevelSources, toplevel->handle);
        }
        roundtrip();
    }
    return nullptr;
}

void CaptureClient::flush()
{
    if (m_display)
        wl_display_flush(m_display);
}

void CaptureClient::roundtrip()
{
    wl_display_roundtrip(m_display);
    dropClosedToplevels();
}

void CaptureClient::dropClosedToplevels()
{
    m_toplevels.erase(std::remove_if(m_toplevels.begin(), m_toplevels.end(),
                                     [](const auto &toplevel) {
                                         return !toplevel->handle;
                                     }),
                      m_toplevels.end());
}
