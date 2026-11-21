// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_RENDER_CURSOR_SINK_H_
#define LINGMO_RENDER_CURSOR_SINK_H_


#include <cstdint>
#include "base/geometry.h"

namespace Lingmo::Render {

// Premultiplied RGBA8888 in increasing address order, matching the compositor's
// own texture upload format.
struct CursorImage {
    // Premultiplied ARGB32, which is the plane's ARGB8888 in memory.
    const uint8_t* pixels = nullptr;
    base::Size size;
    int stride = 0;
    base::Point hotspot;
};

// Somewhere the cursor can be shown without compositing it, normally a KMS
// cursor plane.
//
// Every method may fail or be unavailable; the cursor then goes back into the
// scene, which is always correct and only costs a repaint per motion event.
class CursorSink
{
public:
    virtual ~CursorSink() = default;

    // Returns false when the sink cannot show this image - too large, no
    // cursor plane, a driver rejection - and the caller must composite it.
    virtual bool setCursorImage(const CursorImage& image) = 0;

    virtual void hideCursor() = 0;

    // Where the hotspot lands: in logical desktop coordinates for the cursor's
    // own sink, which maps them onto each output, and in output pixels for an
    // output. Fractions are kept until the output rounds them.
    virtual void moveCursor(const base::PointF& position) = 0;
};

} // namespace Lingmo::Render

#endif // LINGMO_RENDER_CURSOR_SINK_H_
