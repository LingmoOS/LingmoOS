
#include <memory>
#include <string>
#pragma once
#include <utility>
#include <vector>

#include "base/signal.h"

// The selection's contents: each MIME type and its bytes, in the order offered.
using ClipboardData = std::vector<std::pair<std::string, std::string>>;

// The clipboard through ext-data-control, on a Wayland connection of its own.
class WaylandClipboard
{
public:
    WaylandClipboard();
    ~WaylandClipboard();

    WaylandClipboard(const WaylandClipboard &) = delete;
    WaylandClipboard &operator=(const WaylandClipboard &) = delete;

    bool isAvailable() const;
    // Null until a selection has been read whole.
    const ClipboardData *data() const;
    void setData(ClipboardData data);

    Lingmo::Base::Signal<> availabilityChanged;
    Lingmo::Base::Signal<> dataChanged;

private:
    class Private;
    std::unique_ptr<Private> d;
};

