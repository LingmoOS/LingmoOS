// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/clipboard/clipboard_picker_view.h"

#include <algorithm>
#include <cmath>

#include "base/i18n.h"
#include "shell/clipboard/clipboard_history.h"
#include "ui/application.h"
#include "ui/button.h"
#include "ui/icon.h"
#include "ui/image_view.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/list_view.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "toolkit/compositor_colors.h"

using Lingmo::i18n::tr;

namespace Lingmo::Shell {

// The row thumbnails' sourceSize: twice the icon, whatever the scale.
static constexpr int kThumbnailPixels = ClipboardPickerView::kIconSize * 2;

// The picker's surface colour, under the hairlines' ink and the
// confirmation's plate.
static ui::Color surfaceColor()
{
    return ui::color(toolkit::CompositorColor::ClipboardSurface);
}

// Ink.on(surfaceColor, 0.1): the surfaces are dark in dark mode only.
static ui::Color hairlineColor()
{
    return ui::color(toolkit::CompositorColor::ClipboardHairline);
}

// The largest size of the same aspect ratio that fits in the box.
static ui::SizeF fitted(double width, double height, double boxWidth, double boxHeight)
{
    if (width <= 0 || height <= 0)
        return {};
    const double ratio = std::min(boxWidth / width, boxHeight / height);
    return {width * ratio, height * ratio};
}

// The entry's picture, read and decoded on a worker; shrunk to fit `box`
// (device pixels) when larger, keeping its aspect ratio.
static ui::Bitmap decodePicture(const std::string &bodyPath, base::Size box)
{
    std::string bytes;
    for (auto &[type, data] : ClipboardStore::readBody(bodyPath)) {
        if (type.rfind("image/", 0) == 0) {
            bytes = std::move(data);
            break;
        }
    }
    ui::Bitmap bitmap = ui::Bitmap::decode(bytes);
    if (bitmap.isNull() || box.isEmpty())
        return bitmap;
    if (bitmap.width() <= box.width() && bitmap.height() <= box.height())
        return bitmap;
    return bitmap.scaled(ui::scaledSize(bitmap.size(), box, ui::Bitmap::Scaling::Fit));
}

static void showBitmap(ui::ImageView *view, const ui::Bitmap &bitmap)
{
    view->setPixels(bitmap.width(), bitmap.height(), reinterpret_cast<const uint32_t *>(bitmap.constBits()),
                    bitmap.stride(), true);
}

bool ClipboardFilterField::keyPress(const ui::KeyEvent &event)
{
    if (onKey && onKey(event))
        return true;
    return ui::TextField::keyPress(event);
}

namespace {

// One row of the list: the selection plate, a picture's thumbnail or a
// file's icon, and the title.
class PickerRow : public ui::View
{
public:
    explicit PickerRow(ClipboardPickerView *picker)
        : m_picker(picker)
        , m_thumbnail(addChild<ui::ImageView>())
        , m_fileIcon(addChild<ui::ImageView>())
        , m_title(addChild<ui::Label>())
    {
        m_thumbnail->setFillMode(ui::ImageView::FillMode::Stretch);
        m_fileIcon->setFillMode(ui::ImageView::FillMode::PreserveAspectFit);
        m_title->setVerticallyCentered(true);
        m_title->setElide(ui::Label::Elide::Right);
        m_title->setFadesWhenDisabled(false);
        setTracksHover(true);
    }

    void bind(ClipboardHistoryModel &model, int row, bool selected)
    {
        m_row = row;
        m_selected = selected;
        const ClipboardEntry &entry = model.entry(row);
        m_kind = entry.kind;
        m_title->setText(model.title(row));
        const ui::Theme &theme = ui::Theme::current();
        m_title->setColor(selected ? ui::ColorId::HighlightedText : ui::ColorId::Text);
        m_title->setFontSize(theme.fontSize());
        m_title->setFontFamily(theme.fontFamily);

        m_thumbnail->setVisible(m_kind == ClipboardEntry::Kind::Image);
        if (m_kind == ClipboardEntry::Kind::Image) {
            const ui::Bitmap *bitmap = m_picker->thumbnail(entry.key);
            const std::string key = bitmap ? entry.key : std::string();
            if (key != m_thumbnailKey) {
                m_thumbnailKey = key;
                if (bitmap) {
                    showBitmap(m_thumbnail, *bitmap);
                    m_thumbnailSize = {double(bitmap->width()), double(bitmap->height())};
                } else {
                    m_thumbnail->setSource({});
                    m_thumbnailSize = {};
                }
            }
        }

        m_fileIcon->setVisible(m_kind == ClipboardEntry::Kind::Files);
        if (m_kind == ClipboardEntry::Kind::Files) {
            // The full-colour icon, as the file manager's rows (themeSize: 96);
            // the small sizes are monochrome glyphs.
            const double scale = window() ? window()->scale() : 1;
            std::string path = ui::themeIconPath(model.iconName(row), 96, scale);
            if (path.empty())
                path = ui::themeIconPath("text-x-generic", 96, scale);
            if (path != m_fileIcon->source()) {
                m_fileIcon->setSourceSize({double(ClipboardPickerView::kIconSize),
                                           double(ClipboardPickerView::kIconSize)});
                m_fileIcon->setSource(path);
            }
        }
        requestLayout();
        requestPaint();
    }

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const double size = ClipboardPickerView::kIconSize;
        const double left = ClipboardPickerView::kPadding;
        const double width = geometry().width - 2 * left;
        const double iconY = (geometry().height - size) / 2;
        const ui::SizeF fit = fitted(m_thumbnailSize.width, m_thumbnailSize.height, size, size);
        m_thumbnail->setGeometry({left + (size - fit.width) / 2, iconY + (size - fit.height) / 2, fit.width,
                                  fit.height});
        m_fileIcon->setGeometry({left, iconY, size, size});
        // A Row with 8 between the icon and the title.
        const double textLeft = m_kind == ClipboardEntry::Kind::Text ? left : left + size + 8;
        m_title->setGeometry({textLeft, 0, std::max(0.0, width - (textLeft - left)), geometry().height});
    }

    void paint(ui::Painter &painter) override
    {
        if (m_selected) {
            painter.drawRoundedRect(localRect(), {ClipboardPickerView::kRowRadius, 0, ui::themeCorners()},
                                    ui::Theme::current().highlight);
        }
    }

    void hoverMove(const ui::PointerEvent &event) override
    {
        if (m_picker->pointerMoved(mapToWindow(event.position)))
            m_picker->select(m_row);
    }

    void pointerPress(const ui::PointerEvent &event) override
    {
        m_pressed = event.button == ui::MouseButton::Left;
    }

    void pointerRelease(const ui::PointerEvent &event) override
    {
        const bool clicked = m_pressed && localRect().contains(event.position);
        m_pressed = false;
        if (clicked)
            m_picker->rowClicked(m_row);
    }

private:
    ClipboardPickerView *m_picker;
    ui::ImageView *m_thumbnail;
    ui::ImageView *m_fileIcon;
    ui::Label *m_title;
    int m_row = -1;
    bool m_selected = false;
    bool m_pressed = false;
    ClipboardEntry::Kind m_kind = ClipboardEntry::Kind::Text;
    std::string m_thumbnailKey;
    ui::SizeF m_thumbnailSize;
};

} // namespace

// Shift+Delete asks first, inside the card: a plate over everything that
// swallows the pointer, a question and two buttons.
class ClipboardPickerView::Overlay : public ui::View
{
public:
    explicit Overlay(ClipboardPickerView *picker)
        : m_picker(picker)
        , m_question(addChild<ui::Label>(tr("Clear the clipboard history?")))
        , m_cancel(addChild<ui::Button>(tr("Cancel")))
        , m_clear(addChild<ui::Button>(tr("Clear")))
    {
        m_cancel->onClicked = [this] { m_picker->setConfirmingClear(false); };
        m_clear->onClicked = [this] { clear(); };
    }

    void clear()
    {
        m_picker->m_history.clear();
        m_picker->setConfirmingClear(false);
    }

    void retranslate()
    {
        m_question->setText(tr("Clear the clipboard history?"));
        m_cancel->setText(tr("Cancel"));
        m_clear->setText(tr("Clear"));
        requestLayout();
    }

    void updateFont()
    {
        const ui::Theme &theme = ui::Theme::current();
        m_question->setFontSize(theme.fontSize() * 1.2);
        m_question->setFontFamily(theme.fontFamily);
        requestLayout();
    }

    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

protected:
    void layout() override
    {
        // A Column of spacing 16 centred in the card: the question over a
        // Row of the buttons, spacing 12, each centred horizontally.
        const ui::SizeF question = m_question->sizeHint();
        const ui::SizeF cancel = m_cancel->sizeHint();
        const ui::SizeF clear = m_clear->sizeHint();
        const double rowWidth = cancel.width + 12 + clear.width;
        const double rowHeight = std::max(cancel.height, clear.height);
        const double columnWidth = std::max(question.width, rowWidth);
        const double columnHeight = question.height + 16 + rowHeight;
        const double left = (geometry().width - columnWidth) / 2;
        const double top = (geometry().height - columnHeight) / 2;
        m_question->setGeometry({left + (columnWidth - question.width) / 2, top, question.width, question.height});
        const double rowLeft = left + (columnWidth - rowWidth) / 2;
        const double rowTop = top + question.height + 16;
        m_cancel->setGeometry({rowLeft, rowTop, cancel.width, cancel.height});
        m_clear->setGeometry({rowLeft + cancel.width + 12, rowTop, clear.width, clear.height});
    }

    void paint(ui::Painter &painter) override
    {
        painter.drawRoundedRect(localRect(), {kCardRadius, 0, ui::themeCorners()}, surfaceColor().withAlpha(0.92));
    }

    bool keyPress(const ui::KeyEvent &event) override
    {
        if (event.key == ui::key::Escape) {
            m_picker->setConfirmingClear(false);
            return true;
        }
        if (event.key == ui::key::Return || event.key == ui::key::KeypadEnter) {
            clear();
            return true;
        }
        return false;
    }

private:
    ClipboardPickerView *m_picker;
    ui::Label *m_question;
    ui::Button *m_cancel;
    ui::Button *m_clear;
};

ClipboardPickerView::ClipboardPickerView(ClipboardHistory &history)
    : m_history(history)
    , m_filter(addChild<ClipboardFilterField>())
    , m_list(addChild<ui::ListView>())
    , m_preview(addChild<ui::View>())
    , m_previewText(m_preview->addChild<ui::Label>())
    , m_previewImage(m_preview->addChild<ui::ImageView>())
    , m_empty(addChild<ui::Label>())
    , m_overlay(addChild<Overlay>(this))
{
    // The whole header row is the field, for the pointer too.
    m_filter->setFrameVisible(false);
    m_filter->setTextMargins(kPadding, kPadding);
    m_filter->setPlaceholder(tr("Search Clipboard"));
    m_filter->onKey = [this](const ui::KeyEvent &event) { return handleKey(event); };
    m_filter->onTextChanged = [this] {
        m_history.model().setFilterText(m_filter->text());
        m_currentIndex = 0;
        m_list->setScrollPosition({0, 0});
        modelChanged();
    };

    m_list->setClipsChildren(true);
    m_list->setElastic(false);
    m_list->setVerticalBarPolicy(ui::ScrollView::BarPolicy::AlwaysOff);
    m_list->setRowHeight(kRowHeight);
    m_list->createItem = [this] { return std::make_unique<PickerRow>(this); };
    m_list->bindItem = [this](ui::View *item, size_t row) {
        static_cast<PickerRow *>(item)->bind(m_history.model(), int(row), int(row) == m_currentIndex);
    };

    m_preview->setClipsChildren(true);
    m_previewText->setWrap(true);
    m_previewText->setWrapAnywhere(true);
    m_previewText->setElide(ui::Label::Elide::Right);
    m_previewText->setFadesWhenDisabled(false);
    m_previewImage->setFillMode(ui::ImageView::FillMode::Stretch);
    m_previewImage->setVisible(false);

    m_empty->setTextAlignment(ui::Label::TextAlignment::Center);
    m_empty->setVerticallyCentered(true);
    m_empty->setFadesWhenDisabled(false);

    m_overlay->setVisible(false);

    ClipboardHistoryModel &model = m_history.model();
    m_subscriptions.push_back(model.modelReset.connect([this] { modelChanged(); }));
    m_subscriptions.push_back(model.dataChanged.connect([this](int, int) { m_list->reload(); }));
    m_subscriptions.push_back(ui::Application::instance()->languageChanged.connect([this] { retranslate(); }));
    modelChanged();
}

// The filter, the selection and a question being asked stay.
void ClipboardPickerView::retranslate()
{
    m_filter->setPlaceholder(tr("Search Clipboard"));
    m_overlay->retranslate();
    m_list->reload();
    updatePreview();
    updateEmpty();
}

ClipboardPickerView::~ClipboardPickerView() = default;

void ClipboardPickerView::reset()
{
    const ui::Theme &theme = ui::Theme::current();
    m_filter->setFontSize(theme.fontSize() * 1.4);
    m_previewText->setFontSize(theme.fontSize());
    m_previewText->setFontFamily(theme.fontFamily);
    m_previewText->setColor(ui::ColorId::Text);
    m_empty->setFontSize(theme.fontSize() * 1.2);
    m_empty->setFontFamily(theme.fontFamily);
    m_empty->setColor(ui::ColorId::DisabledText);
    m_overlay->updateFont();

    setConfirmingClear(false);
    // Through onTextChanged: the filter goes, the first row is selected and
    // the list is at its top.
    if (!m_filter->text().empty())
        m_filter->setText({});
    m_history.model().setFilterText({});
    m_currentIndex = 0;
    m_pointerKnown = false;
    m_previewScale = 0;
    m_list->setScrollPosition({0, 0});
    modelChanged();
    m_filter->setFocus();
    requestLayout();
}

void ClipboardPickerView::select(int index)
{
    const int count = m_history.model().count();
    if (count == 0)
        return;
    const int clamped = std::clamp(index, 0, count - 1);
    if (clamped != m_currentIndex) {
        m_currentIndex = clamped;
        m_list->reload();
        updatePreview();
    }
    m_list->scrollToRow(size_t(clamped));
}

void ClipboardPickerView::setConfirmingClear(bool confirming)
{
    if (confirming == m_confirmingClear && m_overlay->isVisible() == confirming)
        return;
    m_confirmingClear = confirming;
    m_overlay->setVisible(confirming);
    m_filter->setEnabled(!confirming);
    if (confirming)
        m_overlay->setFocus();
    else
        m_filter->setFocus();
}

bool ClipboardPickerView::pointerMoved(ui::PointF windowPosition)
{
    if (!m_pointerKnown) {
        m_pointerKnown = true;
        m_lastPointer = windowPosition;
        return false;
    }
    if (windowPosition.x == m_lastPointer.x && windowPosition.y == m_lastPointer.y)
        return false;
    m_lastPointer = windowPosition;
    return true;
}

void ClipboardPickerView::rowClicked(int row)
{
    m_history.paste(row);
}

const ui::Bitmap *ClipboardPickerView::thumbnail(const std::string &key)
{
    if (const auto it = m_thumbnails.find(key); it != m_thumbnails.end())
        return it->second.isNull() ? nullptr : &it->second;
    if (m_thumbnailJobs.count(key))
        return nullptr;
    const std::string path = m_history.store().bodyPath(key);
    m_thumbnailJobs[key] = base::runInThread(
            [path] { return decodePicture(path, base::Size(kThumbnailPixels, kThumbnailPixels)); },
            [this, key](ui::Bitmap bitmap) {
                m_thumbnailJobs.erase(key);
                m_thumbnails[key] = std::move(bitmap);
                m_list->reload();
            });
    return nullptr;
}

bool ClipboardPickerView::handleKey(const ui::KeyEvent &event)
{
    const bool shift = event.modifiers & ui::ShiftModifier;
    switch (event.key) {
    case ui::key::Escape:
        if (!m_filter->text().empty())
            m_filter->setText({});
        else if (onCloseRequested)
            onCloseRequested();
        return true;
    case ui::key::Up:
        select(m_currentIndex - 1);
        return true;
    case ui::key::Down:
        select(m_currentIndex + 1);
        return true;
    case ui::key::PageUp:
        select(m_currentIndex - kPageRows);
        return true;
    case ui::key::PageDown:
        select(m_currentIndex + kPageRows);
        return true;
    case ui::key::Home:
        select(0);
        return true;
    case ui::key::End:
        select(m_history.model().count() - 1);
        return true;
    case ui::key::Return:
    case ui::key::KeypadEnter:
        if (m_history.model().count() > 0) {
            if (shift)
                m_history.copy(m_currentIndex);
            else
                m_history.paste(m_currentIndex);
        }
        return true;
    case ui::key::Delete:
        if (shift) {
            if (!m_history.model().historyEmpty())
                setConfirmingClear(true);
        } else {
            removeCurrent();
        }
        return true;
    default:
        return false;
    }
}

void ClipboardPickerView::removeCurrent()
{
    ClipboardHistoryModel &model = m_history.model();
    if (model.count() == 0)
        return;
    m_history.remove(m_currentIndex);
    select(std::min(m_currentIndex, model.count() - 1));
}

void ClipboardPickerView::modelChanged()
{
    // Thumbnails of entries the history dropped go with them.
    const auto dropped = [this](const std::string &key) { return m_history.store().indexOf(key) < 0; };
    std::erase_if(m_thumbnails, [&dropped](const auto &item) { return dropped(item.first); });
    std::erase_if(m_thumbnailJobs, [&dropped](const auto &item) { return dropped(item.first); });

    ClipboardHistoryModel &model = m_history.model();
    m_list->setRowCount(size_t(model.count()));
    m_list->reload();
    updatePreview();
    updateEmpty();
}

void ClipboardPickerView::updateEmpty()
{
    ClipboardHistoryModel &model = m_history.model();
    m_empty->setVisible(model.count() == 0);
    m_empty->setText(model.historyEmpty() ? tr("Clipboard Is Empty") : tr("No Results"));
}

void ClipboardPickerView::updatePreview()
{
    ClipboardHistoryModel &model = m_history.model();
    const bool valid = m_currentIndex >= 0 && m_currentIndex < model.count();
    const ClipboardEntry *entry = valid ? &model.entry(m_currentIndex) : nullptr;
    const bool image = entry && entry->kind == ClipboardEntry::Kind::Image;
    m_previewText->setVisible(entry && !image);
    m_previewText->setText(entry && !image ? model.detail(m_currentIndex) : std::string());
    m_previewImage->setVisible(image && m_previewKey == entry->key && m_previewImage->status()
                                                                            == ui::ImageView::Status::Ready);
    if (!image) {
        m_previewJob.cancel();
        m_previewKey.clear();
        requestLayout();
        return;
    }
    const double scale = window() ? window()->scale() : 1;
    if (entry->key == m_previewKey && scale == m_previewScale)
        return;
    m_previewKey = entry->key;
    m_previewScale = scale;
    m_previewImage->setVisible(false);
    // sourceSize: the pane in device pixels.
    const base::Size box(int(std::lround(m_preview->geometry().width * scale)),
                         int(std::lround(m_preview->geometry().height * scale)));
    const std::string path = m_history.store().bodyPath(entry->key);
    m_previewJob = base::runInThread([path, box] { return decodePicture(path, box); },
                                     [this](ui::Bitmap bitmap) {
                                         if (bitmap.isNull())
                                             return;
                                         showBitmap(m_previewImage, bitmap);
                                         m_previewImage->setVisible(true);
                                         m_previewBitmapSize = {double(bitmap.width()), double(bitmap.height())};
                                         requestLayout();
                                     });
}

void ClipboardPickerView::layout()
{
    const double width = geometry().width;
    const double height = geometry().height;
    m_filter->setGeometry({double(kPadding), double(kPadding), width - 2 * kPadding, double(kHeaderHeight)});

    // The body below the divider, one padding in.
    const double bodyX = kPadding;
    const double bodyY = kPadding + kHeaderHeight + 1 + kPadding;
    const double bodyWidth = width - 2 * kPadding;
    const double bodyHeight = height - bodyY - kPadding;
    // The list and the preview each keep one padding from the separator.
    const double listWidth = std::floor((bodyWidth - 1) / 2) - kPadding;
    m_list->setGeometry({bodyX, bodyY, listWidth, bodyHeight});
    const double previewX = listWidth + kPadding + 1 + kPadding;
    m_preview->setGeometry({bodyX + previewX, bodyY, bodyWidth - previewX, bodyHeight});

    // The first line on the first row's centre line, as many lines as fit.
    const double fontHeight = m_previewText->ascent() + m_previewText->descent();
    const double topPadding = std::max(0.0, std::round((kRowHeight - fontHeight) / 2));
    const double lineHeight = m_previewText->sizeHint().height;
    const int lines = lineHeight > 0 ? std::max(1, int(std::floor((bodyHeight - topPadding) / lineHeight))) : 0;
    m_previewText->setMaximumLineCount(lines);
    m_previewText->setGeometry({0, topPadding, bodyWidth - previewX, bodyHeight - topPadding});

    // Fitted to the pane and on its top, centred across.
    const ui::SizeF fit = fitted(m_previewBitmapSize.width, m_previewBitmapSize.height, bodyWidth - previewX,
                                 bodyHeight);
    m_previewImage->setGeometry({(bodyWidth - previewX - fit.width) / 2, 0, fit.width, fit.height});

    m_empty->setGeometry({bodyX, bodyY, bodyWidth, bodyHeight});
    m_overlay->setGeometry({0, 0, width, height});
}

void ClipboardPickerView::paint(ui::Painter &painter)
{
    if (onPainted)
        onPainted();
    const double width = geometry().width;
    const ui::Color ink = hairlineColor();
    // The divider under the header and the separator between the panes, as
    // FishUI.RoundedRectangles one logical pixel thick.
    painter.drawRoundedRect({double(kPadding), double(kPadding + kHeaderHeight), width - 2 * kPadding, 1}, {}, ink);
    const double bodyY = kPadding + kHeaderHeight + 1 + kPadding;
    const double listWidth = std::floor((width - 2 * kPadding - 1) / 2) - kPadding;
    painter.drawRoundedRect({kPadding + listWidth + kPadding, bodyY, 1, geometry().height - bodyY - kPadding}, {},
                            ink);
}

} // namespace Lingmo::Shell
