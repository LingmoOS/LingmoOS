// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/screenshot/capture_clipboard.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <unistd.h>

#include "clipboard/selection.h"
#include "clipboard/stored_source.h"

namespace Lingmo::Shell {

static const char kPngType[] = "image/png";

// The picture's bytes once encoded; readers before that are held.
class PngSource final : public clipboard::StoredSource
{
public:
    ~PngSource() override
    {
        for (int fd : m_waiting)
            close(fd);
    }

    void send(const std::string &mime, int fd) override
    {
        if (m_ready || mime != kPngType) {
            StoredSource::send(mime, fd);
            return;
        }
        m_waiting.push_back(fd);
    }

    // Empty when the encoding failed: the readers get nothing.
    void setPng(std::string png)
    {
        m_ready = true;
        if (!png.empty())
            payloads[kPngType] = std::move(png);
        for (int fd : std::exchange(m_waiting, {}))
            StoredSource::send(kPngType, fd);
    }

private:
    bool m_ready = false;
    std::vector<int> m_waiting;
};

CaptureClipboard::CaptureClipboard(clipboard::SelectionController *selection)
    : m_selection(selection)
{
}

CaptureClipboard::~CaptureClipboard() = default;

bool CaptureClipboard::setImage(const ui::Bitmap &image)
{
    if (image.isNull() || !m_selection)
        return false;
    auto source = std::make_shared<PngSource>();
    source->offer(kPngType);
    if (!m_selection->setSource(source))
        return false;
    m_encode = base::runInThread([image] { return image.encodePng().value_or(std::string()); },
                                 [weak = std::weak_ptr<PngSource>(source)](std::string png) {
                                     if (const auto source = weak.lock())
                                         source->setPng(std::move(png));
                                 });
    return true;
}

} // namespace Lingmo::Shell
