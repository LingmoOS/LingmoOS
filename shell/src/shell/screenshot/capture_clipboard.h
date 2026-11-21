// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>

#include "base/thread_pool.h"
#include "ui/bitmap.h"

namespace Lingmo::Clipboard {
class SelectionController;
}

namespace Lingmo::Shell {

// A picture on the clipboard as image/png, through the seat's selection
// controller. A whole screen takes about a second to compress, so the selection
// is offered at once and encoded on a worker; a reader that asks before that
// waits for it.
class CaptureClipboard
{
public:
    explicit CaptureClipboard(clipboard::SelectionController *selection);
    ~CaptureClipboard();

    // False when there is no clipboard to take, as while the session is locked.
    bool setImage(const ui::Bitmap &image);

private:
    clipboard::SelectionController *m_selection;
    base::Job m_encode;
};

} // namespace Lingmo::Shell

