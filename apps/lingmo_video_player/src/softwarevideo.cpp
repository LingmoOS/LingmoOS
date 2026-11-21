// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "softwarevideo.h"
#include "mpvhandle.h"
#include "player.h"

#include <mpv/render.h>

#include <algorithm>

using namespace Lingmo;

// Runs on an mpv thread whenever a new frame is due.
static void frameUpdate(void* context)
{
    static_cast<MpvHandle*>(context)->notifyFrame();
}

SoftwareVideo::SoftwareVideo(Player& player)
    : m_player(player.weakPtr())
    , m_handle(player.handle())
{
}

SoftwareVideo::~SoftwareVideo()
{
    release();
}

bool SoftwareVideo::ensureContext()
{
    if (m_context)
        return true;
    if (!m_handle->mpv)
        return false;
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char*>(MPV_RENDER_API_TYPE_SW)},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };
    if (mpv_render_context_create(&m_context, m_handle->mpv, params) < 0) {
        m_context = nullptr;
        return false;
    }
    m_handle->setFrameTask([weak = m_weakFactory.getWeakPtr()] {
        if (!weak)
            return;
        weak->m_handle->frameQueued = false;
        weak->frameDue.emit();
    });
    mpv_render_context_set_update_callback(m_context, frameUpdate, m_handle.get());
    if (m_player)
        m_player->setRenderReady();
    return true;
}

const ui::Image& SoftwareVideo::render(int width, int height)
{
    width = std::max(width, 1);
    height = std::max(height, 1);
    if (m_image.width() != width || m_image.height() != height)
        m_image = ui::Image(width, height);
    uint32_t* pixels = m_image.bits();
    bool drawn = false;
    if (m_context) {
        // Updates mpv's idea of what is due before drawing it.
        mpv_render_context_update(m_context);
        int size[2] = {width, height};
        size_t stride = size_t(width) * 4;
        mpv_render_param params[] = {
            {MPV_RENDER_PARAM_SW_SIZE, size},
            // B, G, R, X in memory: 0x00RRGGBB words on little-endian machines.
            {MPV_RENDER_PARAM_SW_FORMAT, const_cast<char*>("bgr0")},
            {MPV_RENDER_PARAM_SW_STRIDE, &stride},
            {MPV_RENDER_PARAM_SW_POINTER, pixels},
            {MPV_RENDER_PARAM_INVALID, nullptr},
        };
        drawn = mpv_render_context_render(m_context, params) >= 0;
    }
    const size_t count = size_t(width) * size_t(height);
    if (drawn) {
        for (size_t i = 0; i < count; ++i)
            pixels[i] |= 0xff000000u;
    } else {
        std::fill(pixels, pixels + count, 0xff000000u);
    }
    return m_image;
}

void SoftwareVideo::release()
{
    if (!m_context)
        return;
    m_handle->setFrameTask(nullptr);
    mpv_render_context_free(m_context);
    m_context = nullptr;
    if (m_player)
        m_player->setRenderLost();
}
