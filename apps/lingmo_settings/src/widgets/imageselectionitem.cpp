// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/imageselectionitem.h"

#include <algorithm>

#include "ui/label.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"

using namespace Lingmo::ui;

namespace settings {

// The preview: a picture inside the ring's inset or a view of its own size
// centred, and the ring while checked. The ring sits the inset outside the
// picture; its radius grows by as much, so its corners stay concentric with
// the picture's.
class SelectionPreview : public View
{
public:
    SelectionPreview(SizeF size, double inset, ImageSelectionItem::PreviewFit fit = ImageSelectionItem::PreviewFit::Center)
        : m_size(size)
        , m_inset(inset)
        , m_fit(fit)
    {
        setLayoutHints({.preferredWidth = size.width, .preferredHeight = size.height});
    }

    void setPicture(std::string path)
    {
        if (m_fit == ImageSelectionItem::PreviewFit::Cover) {
            m_crop = addChild<View>();
            m_crop->setClipsChildren(true);
            m_picture = m_crop->addChild<Picture>(std::move(path));
        } else {
            m_picture = addChild<Picture>(std::move(path));
        }
    }

    void setIcon(std::unique_ptr<View> icon)
    {
        m_icon = icon.get();
        if (m_fit == ImageSelectionItem::PreviewFit::Cover) {
            m_crop = addChild<View>();
            m_crop->setClipsChildren(true);
            m_crop->addChild(std::move(icon));
        } else {
            addChild(std::move(icon));
        }
    }

    void setChecked(bool checked)
    {
        m_checked = checked;
        requestPaint();
    }

    SizeF sizeHint() const override { return m_size; }

protected:
    void layout() override
    {
        const RectF bounds = localRect();
        if (m_picture) {
            if (m_crop) {
                const double edge = 2;
                const RectF content = bounds.shrunk(Margins::all(edge));
                m_crop->setGeometry(content);
                m_crop->setChildClipRadius(std::max(0.0, Theme::current().bigRadius - edge));
                const double side = std::max(content.width, content.height);
                m_picture->setCornerRadius(0);
                m_picture->setGeometry({(content.width - side) / 2, (content.height - side) / 2, side, side});
            } else {
                m_picture->setCornerRadius(Theme::current().bigRadius);
                m_picture->setGeometry(bounds.shrunk(Margins::all(m_inset)));
            }
        }
        if (m_icon) {
            const SizeF icon = m_icon->sizeHint();
            if (m_crop) {
                // Keep a fixed inner edge in both states. The image fills it
                // with aspect-preserving overscan; only this host rounds it.
                const double edge = 2;
                const auto content = bounds.shrunk(Margins::all(edge));
                m_crop->setGeometry(content);
                m_crop->setChildClipRadius(std::max(0.0, Theme::current().bigRadius - edge));
                const double scale = std::max(content.width / std::max(1.0, icon.width),
                                              content.height / std::max(1.0, icon.height));
                const SizeF enlarged{icon.width * scale, icon.height * scale};
                m_icon->setGeometry({(content.width - enlarged.width) / 2,
                                     (content.height - enlarged.height) / 2,
                                     enlarged.width, enlarged.height});
            } else {
                m_icon->setGeometry({rounding::centeredStart(bounds.width, icon.width),
                                     rounding::centeredStart(bounds.height, icon.height), icon.width, icon.height});
            }
        }
    }

    void paint(Painter& painter) override
    {
        if (!m_checked)
            return;
        const Theme& theme = Theme::current();
        painter.drawRoundedRect(localRect(), {theme.bigRadius, -m_inset, m_crop ? Corners::Circular : themeCorners()}, Color{0, 0, 0, 0}, 2,
                                theme.highlight);
    }

private:
    SizeF m_size;
    double m_inset;
    ImageSelectionItem::PreviewFit m_fit;
    View* m_crop = nullptr;
    Picture* m_picture = nullptr;
    View* m_icon = nullptr;
    bool m_checked = false;
};

ImageSelectionItem::ImageSelectionItem(std::string path, std::string text, SizeF previewSize,
                                       double previewInset, PreviewFit fit)
{
    m_layout = addChild<ColumnLayout>();
    m_preview = m_layout->addChild<SelectionPreview>(previewSize, previewInset, fit);
    m_preview->setPicture(std::move(path));
    build(std::move(text));
}

ImageSelectionItem::ImageSelectionItem(std::unique_ptr<View> icon, std::string text, SizeF previewSize,
                                       double previewInset, PreviewFit fit)
{
    m_layout = addChild<ColumnLayout>();
    m_preview = m_layout->addChild<SelectionPreview>(previewSize, previewInset, fit);
    m_preview->setIcon(std::move(icon));
    build(std::move(text));
}

void ImageSelectionItem::build(std::string text)
{
    m_layout->setSpacing(metrics::s);
    m_label = m_layout->addChild<Label>(std::move(text));
    m_label->setVisible(!m_label->text().empty());
    m_label->setLayoutHints({.horizontalAlignment = Alignment::Center});
}

void ImageSelectionItem::setChecked(bool checked)
{
    if (checked == m_checked)
        return;
    m_checked = checked;
    m_preview->setChecked(checked);
    requestLayout();
}

// The item is as large as its layout, as the Qt version's was.
SizeF ImageSelectionItem::sizeHint() const
{
    return m_layout->sizeHint();
}

double ImageSelectionItem::heightForWidth(double width) const
{
    return m_layout->heightForWidth(width);
}

void ImageSelectionItem::layout()
{
    const Theme& theme = Theme::current();
    m_label->setColor(m_checked ? theme.highlight : theme.text);
    m_layout->setGeometry(localRect());
}

void ImageSelectionItem::pointerPress(const PointerEvent& event)
{
    m_pressed = event.button == MouseButton::Left;
}

void ImageSelectionItem::pointerRelease(const PointerEvent& event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    if (localRect().contains(event.position) && onClicked)
        onClicked();
}

} // namespace settings
