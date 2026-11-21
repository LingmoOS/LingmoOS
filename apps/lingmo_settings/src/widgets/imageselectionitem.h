// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "ui/view.h"

namespace Lingmo::ui {
class ColumnLayout;
class Label;
}

namespace settings {

class Picture;
class SelectionPreview;

// A choice shown as a picture with its name below, ringed in the accent
// colour while checked.
class ImageSelectionItem : public Lingmo::ui::View
{
public:
    enum class PreviewFit { Center, Cover };

    // A picture filling the preview inside the ring's inset.
    ImageSelectionItem(std::string path, std::string text, Lingmo::ui::SizeF previewSize,
                       double previewInset = 4, PreviewFit fit = PreviewFit::Center);
    // Center keeps an icon at its own size. Cover enlarges to fill and clips
    // the artwork to a corner radius concentric with the selection ring.
    ImageSelectionItem(std::unique_ptr<Lingmo::ui::View> icon, std::string text, Lingmo::ui::SizeF previewSize,
                       double previewInset = 4, PreviewFit fit = PreviewFit::Center);

    void setChecked(bool checked);
    bool isChecked() const { return m_checked; }

    Lingmo::ui::SizeF sizeHint() const override;
    double heightForWidth(double width) const override;
    bool acceptsPointer() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override;
    void pointerPress(const Lingmo::ui::PointerEvent& event) override;
    void pointerRelease(const Lingmo::ui::PointerEvent& event) override;

private:
    void build(std::string text);

    Lingmo::ui::ColumnLayout* m_layout;
    SelectionPreview* m_preview;
    Lingmo::ui::Label* m_label;
    bool m_checked = false;
    bool m_pressed = false;
};

} // namespace settings
