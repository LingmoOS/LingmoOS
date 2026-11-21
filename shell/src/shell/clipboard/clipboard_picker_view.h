// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/thread_pool.h"
#include "ui/bitmap.h"
#include "ui/text_field.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Button;
class ImageView;
class Label;
class ListView;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

class ClipboardHistory;

// The filter line: a FishUI field without its frame. The picker's keys go
// to onKey before the field edits with them.
class ClipboardFilterField : public ui::TextField
{
public:
    std::function<bool(const ui::KeyEvent &event)> onKey;

protected:
    bool keyPress(const ui::KeyEvent &event) override;
};

// What the picker window shows: the filter line, the entries newest first on
// the left, the selected one in full on the right, and the confirmation before
// clearing.
class ClipboardPickerView : public ui::View
{
public:
    static constexpr int kWidth = 720;
    static constexpr int kHeight = 440;
    static constexpr int kPadding = 12;
    // FishUI's bigRadius * 1.5.
    static constexpr double kCardRadius = 18;
    // Rows meet the card's sides one padding in.
    static constexpr double kRowRadius = kCardRadius - kPadding;
    static constexpr int kHeaderHeight = 40;
    static constexpr int kRowHeight = 36;
    static constexpr int kIconSize = 24;
    // Page Up and Page Down move the selection this far.
    static constexpr int kPageRows = 6;

    explicit ClipboardPickerView(ClipboardHistory &history);
    ~ClipboardPickerView() override;

    // As the picker opens: no filter, the first row, no confirmation, and a
    // pointer that has to move before it selects.
    void reset();

    int currentIndex() const { return m_currentIndex; }
    // Clamped to the rows, and scrolled into sight.
    void select(int index);
    bool isConfirmingClear() const { return m_confirmingClear; }
    void setConfirmingClear(bool confirming);

    ClipboardFilterField *filterField() const { return m_filter; }
    ui::ListView *list() const { return m_list; }
    ui::Label *preview() const { return m_previewText; }
    ui::Label *emptyLabel() const { return m_empty; }

    // Escape with no filter.
    std::function<void()> onCloseRequested;
    // Before each paint, so the window's glass follows the theme.
    std::function<void()> onPainted;

    // Called by the rows; not part of the public API.
    bool pointerMoved(ui::PointF windowPosition);
    void rowClicked(int row);
    const ui::Bitmap *thumbnail(const std::string &key);

protected:
    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    class Overlay;

    bool handleKey(const ui::KeyEvent &event);
    void removeCurrent();
    void modelChanged();
    void retranslate();
    void updatePreview();
    void updateEmpty();

    ClipboardHistory &m_history;
    ClipboardFilterField *m_filter;
    ui::ListView *m_list;
    ui::View *m_preview;
    ui::Label *m_previewText;
    ui::ImageView *m_previewImage;
    ui::Label *m_empty;
    Overlay *m_overlay;
    int m_currentIndex = 0;
    bool m_confirmingClear = false;
    // Where the pointer last was, in window coordinates; none since opening.
    bool m_pointerKnown = false;
    ui::PointF m_lastPointer;
    // Row thumbnails by entry key, decoded at the row's thumbnail size.
    std::map<std::string, ui::Bitmap> m_thumbnails;
    std::map<std::string, base::Job> m_thumbnailJobs;
    std::string m_previewKey;
    ui::SizeF m_previewBitmapSize;
    double m_previewScale = 0;
    base::Job m_previewJob;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

