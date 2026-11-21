#include "clipboard.h"

#include <algorithm>

#include "waylandclipboard.h"

static const char kMarker[] = "application/x-lingmo-clipboard";

Clipboard::Clipboard()
    : m_waylandClipboard(std::make_unique<WaylandClipboard>())
{
    m_subscriptions.push_back(m_waylandClipboard->dataChanged.connect([this] {
        onDataChanged();
    }));
    m_subscriptions.push_back(m_waylandClipboard->availabilityChanged.connect([this] {
        onDataChanged();
    }));
}

Clipboard::~Clipboard() = default;

void Clipboard::onDataChanged()
{
    if (!m_waylandClipboard->isAvailable())
        return;
    const ClipboardData *data = m_waylandClipboard->data();
    if (!data || data->empty())
        return;

    const auto marker = std::find_if(data->begin(), data->end(), [](const auto &entry) {
        return entry.first == kMarker;
    });
    if (marker != data->end() && marker->second == "1")
        return;

    ClipboardData copy;
    for (const auto &entry : *data) {
        if (entry.first != kMarker)
            copy.push_back(entry);
    }
    copy.emplace_back(kMarker, "1");

    m_waylandClipboard->setData(std::move(copy));
}
