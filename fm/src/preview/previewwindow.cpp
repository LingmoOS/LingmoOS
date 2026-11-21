// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "preview/previewwindow.h"
#include "preview/imagepreview.h"
#include "views/style.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/url.h"
#include "ui/application.h"
#include "ui/color_provider.h"
#include "ui/glyph_run.h"
#include "ui/header_button.h"
#include "ui/icon.h"
#include "ui/image_view.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/progress_bar.h"
#include "ui/cell_layout.h"
#include "ui/text_area.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "ui/window_header.h"

#include <algorithm>
#include <cmath>

using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;

namespace fm {

using ui::rounding::centeredStart;

static constexpr double kSideMargin = 16;
static constexpr double kHeaderHeight = 40;
static constexpr double kMinimumImageWidth = 400;
static constexpr double kMinimumImageHeight = 200;
static constexpr int kTextColumns = 100;
static constexpr double kTextMargin = 20;

// The theme's fixed-width font, else "monospace".
static std::string fixedFamily()
{
    const ui::Theme& theme = ui::Theme::current();
    return theme.fixedFontFamily.empty() ? std::string("monospace") : theme.fixedFontFamily;
}

// The keys Quick Look keeps from its contents.
static bool isWindowKey(uint32_t key, uint32_t modifiers, bool multiple)
{
    if (key == ui::key::Escape)
        return true;
    if ((key == ui::key::Space && modifiers == ui::NoModifier)
        || (key == ui::key::letter('w') && modifiers == ui::ControlModifier))
        return true;
    if (modifiers == ui::NoModifier && multiple && (key == ui::key::Left || key == ui::key::Right))
        return true;
    return key == ui::key::Left || key == ui::key::Right || key == ui::key::Up || key == ui::key::Down;
}

// "Open with …": a pill in the text colour's alpha, its label dimmed while
// the window is inactive.
class OpenButton : public ui::View
{
public:
    OpenButton()
    {
        setTracksHover(true);
        m_label = addChild<ui::Label>();
        m_label->setElide(ui::Label::Elide::Right);
        m_label->setTextAlignment(ui::Label::TextAlignment::Center);
    }

    void setText(std::string text)
    {
        m_label->setText(std::move(text));
        requestLayout();
    }

    void setActive(bool active)
    {
        m_label->setOpacity(active ? 1.0 : 0.6);
    }

    ui::SizeF sizeHint() const override { return {std::min(220.0, m_label->sizeHint().width + 24), 26}; }
    bool acceptsPointer() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override
    {
        const ui::SizeF hint = m_label->sizeHint();
        m_label->setColor(ui::Theme::current().text);
        m_label->setGeometry({12, centeredStart(geometry().height, hint.height), std::max(0.0, geometry().width - 24),
                              hint.height});
    }

    void paint(ui::Painter& painter) override
    {
        const ui::Color tint = ui::color(ui::ColorId::HeaderButtonTint);
        const double alpha = m_pressed ? 0.21 : isHovered() ? 0.14 : 0.07;
        painter.drawRoundedRect(localRect(), {geometry().height / 2, 0, ui::Corners::Circular},
                                style::withAlpha(tint, alpha));
    }

    void hoverChanged() override { requestPaint(); }

    void pointerPress(const ui::PointerEvent&) override
    {
        m_pressed = true;
        requestPaint();
    }

    void pointerRelease(const ui::PointerEvent& event) override
    {
        m_pressed = false;
        requestPaint();
        if (localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    ui::Label* m_label;
    bool m_pressed = false;
};

// The header's contents before the close button.
class PreviewHeaderRow : public ui::View
{
public:
    explicit PreviewHeaderRow(PreviewController& controller)
        : m_controller(controller)
    {
        previous = addChild<ui::HeaderButton>(std::string(), 32);
        previous->setGlyph("chevron-left", 16);
        previous->onClicked = [this] { m_controller.step(-1); };
        next = addChild<ui::HeaderButton>(std::string(), 32);
        next->setGlyph("chevron-right", 16);
        next->onClicked = [this] { m_controller.step(1); };
        position = addChild<ui::Label>();
        title = addChild<ui::Label>();
        title->setWeight(700);
        title->setElide(ui::Label::Elide::Middle);
        open = addChild<OpenButton>();
        open->onClicked = [this] { m_controller.openCurrent(); };
    }

    void update(const PreviewFile& shown, bool pageMode, bool active)
    {
        const bool multiple = m_controller.items().size() > 1;
        m_multiple = multiple;
        previous->setVisible(multiple);
        next->setVisible(multiple);
        position->setVisible(multiple);
        previous->setEnabled(m_controller.currentIndex() > 0);
        next->setEnabled(m_controller.currentIndex() + 1 < int(m_controller.items().size()));
        // TRANSLATORS: the shown file's place among those previewed, such as "3 of 10".
        position->setText(format(tr("{index} of {count}"), {{"index", std::to_string(m_controller.currentIndex() + 1)},
                                                            {"count", std::to_string(m_controller.items().size())}}));
        position->setColor(ui::ColorId::Text);
        title->setVisible(pageMode);
        title->setText(shown.name);
        title->setColor(ui::ColorId::Text);
        // A single compact preview has no header row to hold it.
        open->setVisible(shown.application.has_value() && (pageMode || multiple));
        if (shown.application)
            open->setText(format(tr("Open with {application}"), {{"application", shown.application->name}}));
        open->setActive(active);
        requestLayout();
    }

    ui::HeaderButton* previous;
    ui::HeaderButton* next;
    ui::Label* position;
    ui::Label* title;
    OpenButton* open;

protected:
    void layout() override
    {
        const double height = geometry().height;
        double x = 4;
        if (m_multiple) {
            previous->setGeometry({x, centeredStart(height, 32), 32, 32});
            x += 32;
            next->setGeometry({x, centeredStart(height, 32), 32, 32});
            x += 32 + 4;
            const ui::SizeF hint = position->sizeHint();
            position->setGeometry({x, centeredStart(height, hint.height), hint.width, hint.height});
            x += hint.width;
        }
        double right = geometry().width - 4;
        if (open->isVisible()) {
            const ui::SizeF hint = open->sizeHint();
            open->setGeometry({right - hint.width, centeredStart(height, hint.height), hint.width, hint.height});
            right -= hint.width;
        }
        const double left = m_multiple ? x + 8 : 12;
        const ui::SizeF hint = title->sizeHint();
        title->setGeometry({left, centeredStart(height, hint.height), std::max(0.0, right - 8 - left), hint.height});
    }

private:
    PreviewController& m_controller;
    bool m_multiple = false;
};

// The compact card: the file's icon, its name and size.
class PreviewCard : public ui::View
{
public:
    PreviewCard()
    {
        icon = addChild<ui::ThemeIcon>(std::string(), 96);
        icon->setImageProviderLookup(true);
        name = addChild<ui::Label>();
        name->setWeight(700);
        name->setFontSize(16);
        name->setElide(ui::Label::Elide::Middle);
        detail = addChild<ui::Label>();
    }

    void setFile(const PreviewFile& file)
    {
        icon->setIconName(file.icon.empty() ? "application-octet-stream" : file.icon);
        name->setText(file.name);
        if (file.folder) {
            const std::string count = trn("{n} item", "{n} items", file.itemCount);
            // TRANSLATORS: a folder's size and how many items it holds, such as "4 KB, 3 items".
            detail->setText(format(tr("{size}, {count}"), {{"size", file.size}, {"count", count}}));
        } else {
            detail->setText(file.size);
        }
        requestLayout();
    }

    ui::ThemeIcon* icon;
    ui::Label* name;
    ui::Label* detail;

protected:
    // A RowLayout of the icon and a column, spacing 20 and 6, centred.
    void layout() override
    {
        const ui::Theme& theme = ui::Theme::current();
        name->setColor(theme.text);
        detail->setColor(theme.text);
        const double height = geometry().height;
        icon->setGeometry({0, centeredStart(height, 96), 96, 96});
        const double x = 96 + 20;
        const double width = std::max(0.0, geometry().width - x);
        const double nameHeight = name->sizeHint().height;
        const double detailHeight = detail->heightForWidth(width);
        const double column = nameHeight + 6 + detailHeight;
        const double y = centeredStart(height, column);
        name->setGeometry({x, y, width, nameHeight});
        detail->setGeometry({x, y + nameHeight + 6, width, detailHeight});
    }
};

// A text area that leaves Quick Look's keys to the window.
class PreviewTextArea : public ui::TextArea
{
public:
    std::function<bool(const ui::KeyEvent&)> isWindowKey;

protected:
    bool keyPress(const ui::KeyEvent& event) override
    {
        if (isWindowKey && isWindowKey(event))
            return false;
        return TextArea::keyPress(event);
    }
};

// The text on an opaque page, wrapped and selectable,
// and a notice below it when the file goes on.
class PreviewTextPage : public ui::ScrollView
{
public:
    PreviewTextPage()
    {
        m_column = setContent<TextColumn>(this);
    }

    void retranslate()
    {
        m_column->notice->setText(tr("The rest of this file is not shown."));
        m_column->requestLayout();
    }

    void setFile(const PreviewFile& file)
    {
        // TextEdit rounds the font to half points.
        const double points = std::round(ui::Theme::current().fontPointSize * 2) / 2;
        m_column->text->setFont(fixedFamily(), points * 4 / 3, 400);
        if (file.text != m_text) {
            m_text = file.text;
            m_column->text->setText(m_text);
            m_column->text->setCursorPosition(0);
            setScrollPosition({0, 0});
        }
        m_column->notice->setVisible(file.truncated);
        m_column->requestLayout();
        requestLayout();
    }

    PreviewTextArea* textArea() const { return m_column->text; }

protected:
    void paint(ui::Painter& painter) override
    {
        painter.fillRect(localRect(), ui::Theme::current().secondBackground);
    }

private:
    class TextColumn : public ui::View
    {
    public:
        explicit TextColumn(PreviewTextPage*)
        {
            text = addChild<PreviewTextArea>();
            text->setReadOnly(true);
            text->setFrameVisible(false);
            text->setWrapMode(ui::TextArea::WrapMode::Wrap);
            text->setPadding(ui::Margins::all(kTextMargin));
            notice = addChild<ui::Label>(tr("The rest of this file is not shown."));
            notice->setWrap(true);
            notice->setTextAlignment(ui::Label::TextAlignment::Center);
        }

        double heightForWidth(double width) const override
        {
            double height = text->heightForWidth(width);
            if (notice->isVisible())
                height += notice->heightForWidth(width - 2 * kTextMargin) + kTextMargin;
            return height;
        }

        PreviewTextArea* text;
        ui::Label* notice;

    protected:
        void layout() override
        {
            const double width = geometry().width;
            const double textHeight = text->heightForWidth(width);
            text->setGeometry({0, 0, width, textHeight});
            notice->setColor(ui::Theme::current().disabledText);
            notice->setGeometry({kTextMargin, textHeight, std::max(0.0, width - 2 * kTextMargin),
                                 notice->heightForWidth(width - 2 * kTextMargin)});
        }
    };

    TextColumn* m_column;
    std::string m_text;
};

// The image page: the viewer, the busy indicator for a slow decode, and the
// failure notice.
class PreviewImagePage : public ui::View
{
public:
    PreviewImagePage()
    {
        viewer = addChild<ImagePreview>();
        busy = addChild<ui::BusyIndicator>();
        busy->setVisible(false);
        failed = addChild<ui::Label>(tr("This image could not be displayed."));
        failed->setVisible(false);
        viewer->onStatusChanged = [this] { updateStatus(); };
        m_busyDelay.setSingleShot(true);
        m_busyDelay.setInterval(std::chrono::milliseconds(300));
        m_busyDelay.onTimeout = [this] {
            m_elapsed = true;
            updateStatus();
        };
    }

    void setImage(const PreviewFile& file, ui::SizeF prepared, double ratio)
    {
        viewer->setImage(file, prepared, ratio);
        updateStatus();
    }

    ImagePreview* viewer;
    ui::BusyIndicator* busy;
    ui::Label* failed;

protected:
    void layout() override
    {
        viewer->setGeometry(localRect());
        const ui::RectF rect = localRect();
        busy->setGeometry({centeredStart(rect.width, 22), centeredStart(rect.height, 22), 22, 22});
        const ui::SizeF hint = failed->sizeHint();
        failed->setColor(ui::Theme::current().text);
        failed->setGeometry({centeredStart(rect.width, hint.width), centeredStart(rect.height, hint.height), hint.width,
                             hint.height});
    }

private:
    void updateStatus()
    {
        const bool loading = viewer->isLoading();
        if (loading && !m_wasLoading) {
            m_elapsed = false;
            m_busyDelay.start();
        }
        if (!loading)
            m_busyDelay.stop();
        m_wasLoading = loading;
        busy->setVisible(loading && m_elapsed);
        busy->setRunning(loading && m_elapsed);
        failed->setVisible(viewer->hasFailed());
    }

    base::Timer m_busyDelay;
    bool m_elapsed = false;
    bool m_wasLoading = false;
};

// The window's content: the renderer under the header, which runs over it.
class PreviewRoot : public ui::View
{
public:
    explicit PreviewRoot(PreviewWindow* preview)
        : m_preview(preview)
    {
        card = addChild<PreviewCard>();
        image = addChild<PreviewImagePage>();
        text = addChild<PreviewTextPage>();
        text->textArea()->isWindowKey = [this](const ui::KeyEvent& event) {
            return isWindowKey(event.key, event.modifiers, m_preview->multiple());
        };
        message = addChild<ui::Label>();
        message->setWrap(true);
        message->setTextAlignment(ui::Label::TextAlignment::Center);
        header = addChild<ui::WindowHeader>(kHeaderHeight);
        header->setMinimizeVisible(false);
        header->setMaximizeVisible(false);
        header->setButtonMargin(4);
        header->setButtonsCentered(true);
        row = header->setContent<PreviewHeaderRow>(preview->m_controller);
        preloader = addChild<ui::ImageView>();
        preloader->setFillMode(ui::ImageView::FillMode::PreserveAspectFit);
        preloader->setAutoTransform(true);
        preloader->setOpacity(0);
    }

    void show(const PreviewFile& file, ui::SizeF prepared)
    {
        const bool imageMode = file.kind == PreviewFile::Kind::Image;
        const bool textMode = file.kind == PreviewFile::Kind::Text;
        m_pageMode = imageMode || textMode;
        card->setVisible(!m_pageMode);
        image->setVisible(imageMode);
        text->setVisible(textMode);
        if (imageMode)
            image->setImage(file, prepared, m_preview->displayRatio());
        else
            image->setImage(PreviewFile(), {}, 1);
        if (textMode)
            text->setFile(file);
        if (!m_pageMode)
            card->setFile(file);
        message->setVisible(!file.message.empty());
        message->setText(file.message);
        updateHeader();
        requestLayout();
    }

    void updateHeader()
    {
        row->update(m_preview->m_shown, m_pageMode, window() && window()->isActive());
    }

    void retranslate()
    {
        image->failed->setText(tr("This image could not be displayed."));
        text->retranslate();
        updateHeader();
        requestLayout();
    }

    bool acceptsFocus() const override { return true; }
    bool acceptsPointer() const override { return true; }

    PreviewCard* card;
    PreviewImagePage* image;
    PreviewTextPage* text;
    ui::Label* message;
    ui::WindowHeader* header;
    PreviewHeaderRow* row;
    ui::ImageView* preloader;

protected:
    void layout() override
    {
        const ui::RectF rect = localRect();
        const bool multiple = m_preview->multiple();
        message->setColor(ui::Theme::current().text);
        const double messageHeight = message->isVisible() ? message->heightForWidth(rect.width - 20) : 0;
        message->setGeometry({10, rect.height - 10 - messageHeight, std::max(0.0, rect.width - 20), messageHeight});

        const double top = (multiple || m_pageMode ? kHeaderHeight : 0) + (m_pageMode ? 0 : multiple ? 8 : 16);
        const double left = m_pageMode ? 0 : kSideMargin;
        const double right = m_pageMode ? 0 : multiple ? kSideMargin : 40;
        const double bottom = (m_pageMode ? 0 : 16) + (message->isVisible() ? messageHeight + 20 : 0);
        const ui::RectF content {left, top, std::max(0.0, rect.width - left - right),
                                 std::max(0.0, rect.height - top - bottom)};
        card->setGeometry(content);
        image->setGeometry(content);
        text->setGeometry(content);
        header->setGeometry({0, 0, rect.width, kHeaderHeight});
        preloader->setGeometry({0, 0, 1, 1});
        if (window())
            window()->setBackgroundBlur(rect, ui::Theme::current().windowRadius);
        m_preview->updateGlass();
    }

    bool keyPress(const ui::KeyEvent& event) override
    {
        if (m_preview->keyPressed(event.key, event.modifiers, event.repeat))
            return true;
        if (image->isVisible() && (event.modifiers & ui::ControlModifier)) {
            ImagePreview* viewer = image->viewer;
            const ui::PointF center = viewer->center();
            if (event.key == ui::key::letter('=') || event.key == ui::key::letter('+')) {
                viewer->zoomBy(1.5, center, true);
                return true;
            }
            if (event.key == ui::key::letter('-')) {
                viewer->zoomBy(1 / 1.5, center, true);
                return true;
            }
            if (event.key == ui::key::letter('0')) {
                viewer->zoomToFit();
                return true;
            }
            if (event.key == ui::key::letter('9')) {
                viewer->zoomToActualSize();
                return true;
            }
        }
        if (text->isVisible() && event.modifiers == ui::ControlModifier && event.key == ui::key::letter('c')) {
            text->textArea()->copy();
            return true;
        }
        return false;
    }

private:
    PreviewWindow* m_preview;
    bool m_pageMode = false;
};

PreviewWindow::PreviewWindow(PreviewController& controller)
    : m_controller(controller)
{
    m_stall.setSingleShot(true);
    m_stall.setInterval(std::chrono::milliseconds(300));
    // A slow decode is shown anyway, with the busy indicator.
    m_stall.onTimeout = [this] {
        if (m_pending)
            present(*m_pending);
    };

    m_subscriptions.push_back(m_controller.showRequested.connect([this] {
        m_opening = true;
        stage(m_controller.file());
    }));
    m_subscriptions.push_back(m_controller.changed.connect([this] {
        if (!m_controller.isVisible()) {
            m_pending.reset();
            m_opening = false;
            m_stall.stop();
            destroy();
        } else if (m_root) {
            m_root->updateHeader();
            m_root->requestLayout();
        }
    }));
    m_subscriptions.push_back(m_controller.fileChanged.connect([this] {
        if (m_window && !m_controller.isLoading())
            stage(m_controller.file());
    }));
    m_subscriptions.push_back(ui::Application::instance()->languageChanged.connect([this] { retranslate(); }));
}

PreviewWindow::~PreviewWindow()
{
    destroy();
}

// The item shown stays; its description is read again for its size and type.
void PreviewWindow::retranslate()
{
    if (m_root) {
        m_root->retranslate();
        m_window->setTitle(m_shown.name.empty() ? std::string(tr("Quick Look")) : m_shown.name);
    }
    m_controller.reload();
}

double PreviewWindow::displayRatio() const
{
    // The requesting window's: until this window is on screen it does not know its own.
    if (m_controller.sourceRatio() > 0)
        return m_controller.sourceRatio();
    if (m_window && m_window->scale() > 0)
        return m_window->scale();
    return 1;
}

ui::SizeF PreviewWindow::screenSize() const
{
    if (m_window) {
        const ui::SizeF size = m_window->screenSize();
        if (size.width > 0 && size.height > 0)
            return size;
    }
    for (ui::Window* window : ui::Application::instance()->windows()) {
        const ui::SizeF size = window->screenSize();
        if (size.width > 0 && size.height > 0)
            return size;
    }
    return {1280, 800};
}

// 100 columns of the fixed font and the page's margins, within 75% of the screen.
double PreviewWindow::textPageWidth() const
{
    const ui::Theme& theme = ui::Theme::current();
    const ui::FontMetrics metrics(ui::FontSpec {fixedFamily(), theme.fontPointSize}, 1, false);
    return std::min(std::floor(screenSize().width * 0.75),
                    std::ceil(metrics.horizontalAdvanceF("0") * kTextColumns) + 2 * kTextMargin);
}

// An image at its own pixels, shrunk to fit a large share of the screen.
ui::SizeF PreviewWindow::sizeFor(const PreviewFile& file) const
{
    const ui::SizeF screen = screenSize();
    if (file.kind == PreviewFile::Kind::Text)
        return {textPageWidth(), std::floor(screen.height * 0.8)};
    if (file.kind != PreviewFile::Kind::Image)
        return {480, multiple() ? 168.0 : 144.0};
    if (!file.isImage())
        return {720, 540};
    const double ratio = displayRatio();
    const double w = file.width / ratio;
    const double h = file.height / ratio;
    const double scale =
            std::min({1.0, std::floor(screen.width * 0.75) / w, (std::floor(screen.height * 0.8) - kHeaderHeight) / h});
    return {std::max(kMinimumImageWidth, std::round(w * scale)),
            std::max(kMinimumImageHeight, std::round(h * scale)) + kHeaderHeight};
}

// The decode the image page asks for when fitted into a window of the size.
ui::SizeF PreviewWindow::fittedPixels(const PreviewFile& file, ui::SizeF size) const
{
    const double ratio = displayRatio();
    const double zoom =
            std::min({1.0, size.width * ratio / file.width, (size.height - kHeaderHeight) * ratio / file.height});
    return {std::ceil(file.width * zoom), std::ceil(file.height * zoom)};
}

void PreviewWindow::stage(const PreviewFile& file)
{
    if (!file.isImage() || file.animated || !m_root) {
        present(file);
        return;
    }
    const ui::SizeF pixels = fittedPixels(file, sizeFor(file));
    const double ratio = m_window->scale() > 0 ? m_window->scale() : 1;
    m_pending = file;
    ui::ImageView* preloader = m_root->preloader;
    preloader->onStatusChanged = [this](ui::ImageView::Status status) {
        if (m_pending && status != ui::ImageView::Status::Loading)
            present(*m_pending);
    };
    preloader->setSourceSize({pixels.width / ratio, pixels.height / ratio});
    preloader->setSource(base::url::toLocalFile(file.url).value_or(std::string()));
    m_root->requestLayout();
    if (preloader->status() != ui::ImageView::Status::Loading)
        present(file);
    else if (!m_stall.isActive())
        m_stall.start();
}

// By value: callers pass *m_pending, which is reset below.
void PreviewWindow::present(PreviewFile file)
{
    m_pending.reset();
    m_stall.stop();
    const ui::SizeF size = sizeFor(file);
    const ui::SizeF prepared = file.isImage() && !file.animated ? fittedPixels(file, size) : ui::SizeF();
    m_shown = std::move(file);
    if (!m_window)
        open();
    // The contents first, then one resize: size, title and picture change in one frame.
    m_root->show(m_shown, prepared);
    m_window->setTitle(m_shown.name.empty() ? std::string(tr("Quick Look")) : m_shown.name);
    m_window->setMinimumSize(size);
    m_window->setMaximumSize(size);
    m_window->resize(size);
    m_root->preloader->onStatusChanged = nullptr;
    m_root->preloader->setSource({});
    if (m_opening) {
        m_opening = false;
        m_window->show();
        m_root->setFocus();
        m_window->activate();
    }
}

void PreviewWindow::open()
{
    m_window = std::make_unique<ui::Window>();
    // Floats above applications, and leaves the source application's menu in the status bar.
    m_window->setSkipTaskbar(true);
    m_window->setAlwaysOnTop(true);
    m_window->setResizeAroundCenter(true);
    // Each Space maps a new window; it opens where the last one was left.
    m_window->setRestoreId("quickLook");
    m_window->setBackgroundColor(ui::Color {0, 0, 0, 0});
    m_glassDarkMode.reset();
    updateGlass();
    auto root = std::make_unique<PreviewRoot>(this);
    m_root = root.get();
    m_window->setContentView(std::move(root));
    m_window->onCloseRequested = [this] {
        base::EventLoop::current()->post([this] { m_controller.close(); });
    };
    m_window->onActiveChanged = [this] {
        if (m_root)
            m_root->updateHeader();
    };
}

void PreviewWindow::updateGlass()
{
    const bool darkMode = ui::Theme::current().darkMode;
    if (!m_window || m_glassDarkMode == darkMode)
        return;
    m_glassDarkMode = darkMode;
    ui::GlassMaterial glass;
    glass.blurSigma = 4.4;
    glass.displacement = 21.6;
    glass.chroma = 0.03;
    if (darkMode) {
        glass.tintRed = 0.18;
        glass.tintGreen = 0.18;
        glass.tintBlue = 0.20;
    } else {
        glass.tintRed = glass.tintGreen = glass.tintBlue = 1;
    }
    glass.tintAlpha = 0.7;
    m_window->setBackgroundGlass(glass);
}

void PreviewWindow::destroy()
{
    if (!m_window)
        return;
    m_root = nullptr;
    // Not from inside the window's own event handling.
    std::shared_ptr<ui::Window> gone(std::move(m_window));
    base::EventLoop::current()->post([gone] {});
}

bool PreviewWindow::keyPressed(uint32_t key, uint32_t modifiers, bool repeat)
{
    if ((key == ui::key::Space && modifiers == ui::NoModifier)
        || (key == ui::key::letter('w') && modifiers == ui::ControlModifier)) {
        if (!repeat)
            m_controller.close();
        return true;
    }
    if (key == ui::key::Escape) {
        m_controller.close();
        return true;
    }
    if (multiple() && modifiers == ui::NoModifier && (key == ui::key::Left || key == ui::key::Right)) {
        m_controller.step(key == ui::key::Left ? -1 : 1);
        return true;
    }
    if (key == ui::key::Left || key == ui::key::Right || key == ui::key::Up || key == ui::key::Down) {
        if (onNavigate)
            onNavigate(key, modifiers);
        return true;
    }
    return false;
}

} // namespace fm
