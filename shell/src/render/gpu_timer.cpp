// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/gpu_timer.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>

#include <utility>

#include "render/egl_context.h"

namespace Lingmo::Render {

GpuTimer::Mark::Mark(std::shared_ptr<std::vector<unsigned int>> pool, unsigned int query)
    : m_pool(std::move(pool))
    , m_query(query)
{
}

GpuTimer::Mark::Mark(Mark &&other) noexcept
    : m_pool(std::move(other.m_pool))
    , m_query(std::exchange(other.m_query, 0))
{
}

GpuTimer::Mark &GpuTimer::Mark::operator=(Mark &&other) noexcept
{
    if (this != &other) {
        if (m_query)
            m_pool->push_back(m_query);
        m_pool = std::move(other.m_pool);
        m_query = std::exchange(other.m_query, 0);
    }
    return *this;
}

GpuTimer::Mark::~Mark()
{
    // Issuing a counter again on a query still in flight is allowed, so the
    // name is reused without waiting for it.
    if (m_query)
        m_pool->push_back(m_query);
}

GpuTimer::GpuTimer(EglContext &context)
{
    if (!context.supportsGl("GL_EXT_disjoint_timer_query")
        || !context.glVersion().starts_with("OpenGL ES 3"))
        return;
    m_genQueries = reinterpret_cast<void *>(eglGetProcAddress("glGenQueriesEXT"));
    m_getQueryObjectui = reinterpret_cast<void *>(eglGetProcAddress("glGetQueryObjectuivEXT"));
    m_getQueryObjectui64 =
            reinterpret_cast<void *>(eglGetProcAddress("glGetQueryObjectui64vEXT"));
    m_queryCounter = reinterpret_cast<void *>(eglGetProcAddress("glQueryCounterEXT"));
    if (!m_genQueries || !m_getQueryObjectui || !m_getQueryObjectui64 || !m_queryCounter)
        m_genQueries = m_getQueryObjectui = m_getQueryObjectui64 = m_queryCounter = nullptr;
}

GpuTimer::Mark GpuTimer::mark()
{
    if (!available())
        return {};
    GLuint query = 0;
    if (m_pool->empty()) {
        reinterpret_cast<PFNGLGENQUERIESEXTPROC>(m_genQueries)(1, &query);
    } else {
        query = m_pool->back();
        m_pool->pop_back();
    }
    if (!query)
        return {};
    reinterpret_cast<PFNGLQUERYCOUNTEREXTPROC>(m_queryCounter)(query, GL_TIMESTAMP_EXT);
    return Mark(m_pool, query);
}

std::optional<GpuTimer::Time> GpuTimer::reached(const Mark &mark)
{
    if (!available() || !mark.isValid())
        return std::nullopt;
    GLuint ready = 0;
    reinterpret_cast<PFNGLGETQUERYOBJECTUIVEXTPROC>(m_getQueryObjectui)(
            mark.m_query, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
    if (!ready)
        return std::nullopt;
    GLuint64 reachedNs = 0;
    reinterpret_cast<PFNGLGETQUERYOBJECTUI64VEXTPROC>(m_getQueryObjectui64)(
            mark.m_query, GL_QUERY_RESULT_EXT, &reachedNs);
    // A disjoint operation, such as a GPU clock change, voids every result
    // since the last check, and the clock offset with it.
    GLint disjoint = 0;
    glGetIntegerv(GL_GPU_DISJOINT_EXT, &disjoint);
    if (disjoint)
        return std::nullopt;
    // The GPU counts on its own clock; read it beside the CPU's to move the
    // result onto the steady clock, as GPUTimingImpl::CalculateTimerOffset does.
    GLint64 gpuNow = 0;
    glGetInteger64v(GL_TIMESTAMP_EXT, &gpuNow);
    const Time cpuNow = std::chrono::steady_clock::now();
    return cpuNow - std::chrono::nanoseconds(gpuNow - GLint64(reachedNs));
}

} // namespace Lingmo::Render
