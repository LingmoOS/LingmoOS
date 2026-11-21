// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <vector>

namespace Lingmo::Render {

class EglContext;

// When the GPU reached a point in the command stream, from a timestamp query
// read back once it is available, as ui/gl/gpu_timing.cc does. Unlike a native
// fence, neither the query nor its result makes the driver submit at once.
class GpuTimer
{
public:
    using Time = std::chrono::steady_clock::time_point;

    // A query issued into the command stream; its name goes back to the pool
    // when the mark is dropped, read or not.
    class Mark
    {
    public:
        Mark() = default;
        Mark(Mark &&other) noexcept;
        Mark &operator=(Mark &&other) noexcept;
        ~Mark();

        bool isValid() const { return m_query != 0; }

    private:
        friend class GpuTimer;
        Mark(std::shared_ptr<std::vector<unsigned int>> pool, unsigned int query);
        std::shared_ptr<std::vector<unsigned int>> m_pool;
        unsigned int m_query = 0;
    };

    explicit GpuTimer(EglContext &context);

    // Whether the GL has GL_EXT_disjoint_timer_query and GLES 3's GL_TIMESTAMP read.
    bool available() const { return m_queryCounter != nullptr; }

    // Marks everything issued so far. The context must be current.
    Mark mark();

    // When the GPU passed `mark`, on the steady clock; empty until it has, and
    // when the GPU's clock jumped meanwhile. The context must be current.
    std::optional<Time> reached(const Mark &mark);

private:
    std::shared_ptr<std::vector<unsigned int>> m_pool =
            std::make_shared<std::vector<unsigned int>>();
    void *m_genQueries = nullptr;
    void *m_queryCounter = nullptr;
    void *m_getQueryObjectui = nullptr;
    void *m_getQueryObjectui64 = nullptr;
};

} // namespace Lingmo::Render
