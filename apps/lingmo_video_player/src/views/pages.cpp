// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages.h"
#include "mainwindow.h"

#include "base/i18n.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>

using namespace Lingmo::ui;
using views::white;
using namespace std::chrono_literals;
using Lingmo::i18n::tr;

Osd::Osd()
    : m_label(views::addLabel(this, {}, 0, white()))
    , m_action(addChild<views::PillButton>(std::string()))
{
    setName("osd");
    setTracksHover(true);
    views::setTabular(m_label);
    m_action->setVisible(false);
    m_action->onClicked = [this] {
        if (m_callback)
            m_callback();
        hide();
    };
    m_hideTimer.setSingleShot(true);
    m_hideTimer.onTimeout = [this] {
        if (isHovered())
            m_hideTimer.start(m_hideTimer.interval());
        else
            fadeTo(0);
    };
    m_opacity.onUpdate = [this] {
        setOpacity(m_opacity.value());
        setVisible(m_opacity.value() > 0);
    };
    setOpacity(0);
    setVisible(false);
}

void Osd::show(const std::string& message, int timeoutMs, std::string actionText, std::function<void()> action)
{
    m_label->setText(message);
    m_action->setText(actionText);
    m_action->setVisible(!actionText.empty());
    m_callback = std::move(action);
    const int timeout = timeoutMs > 0 ? timeoutMs : actionText.empty() ? 1500 : 5000;
    m_hideTimer.start(std::chrono::milliseconds(timeout));
    requestLayout();
    if (parent())
        parent()->requestLayout();
    fadeTo(1);
}

void Osd::hide()
{
    m_hideTimer.stop();
    fadeTo(0);
}

void Osd::fadeTo(double opacity)
{
    if (opacity > 0)
        setVisible(true);
    m_opacity.setTarget(this, opacity);
}

void Osd::hoverChanged()
{
}

SizeF Osd::sizeHint() const
{
    const bool action = m_action->isVisible();
    double width = std::ceil(m_label->sizeHint().width) + (action ? 8 : 0);
    if (action)
        width += 8 + std::ceil(m_action->sizeHint().width);
    return {width + (action ? 8 : kTextInset * 2), 36};
}

void Osd::layout()
{
    const bool action = m_action->isVisible();
    const SizeF text = m_label->sizeHint();
    const SizeF pill = m_action->sizeHint();
    double rowWidth = std::ceil(text.width) + (action ? 8 + 8 + std::ceil(pill.width) : 0);
    const double height = geometry().height;
    double x = rounding::centeredStart(geometry().width, rowWidth);
    if (action)
        x += 8;
    m_label->setGeometry({x, rounding::centeredStart(height, std::ceil(text.height)), std::ceil(text.width),
                          text.height});
    if (action)
        m_action->setGeometry({x + std::ceil(text.width) + 8, rounding::centeredStart(height, pill.height),
                               std::ceil(pill.width), pill.height});
}

void Osd::paint(Painter& painter)
{
    const RectF rect = localRect();
    painter.drawRoundedRect(rect, {rect.height / 2, 0, Corners::Circular}, views::rgba(0.09, 0.09, 0.1, 0.78),
                            1 / painter.scale(), white(0.1));
}

Flash::Flash()
    : m_icon(addChild<Icon>(views::glyph("play"), 32))
{
    m_icon->setColor(white());
    setVisible(false);
}

void Flash::show(const std::string& icon)
{
    m_icon->setIconName(views::glyph(icon));
    if (!window())
        return;
    if (!m_animation) {
        m_animation = std::make_unique<Animation>(window());
        // 60 ms in, 300 held, 300 out while growing to 1.3.
        m_animation->setDuration(660ms);
        m_animation->setEasing(Animation::Easing::Linear);
        m_animation->onUpdate = [this](double t) {
            const double ms = t * 660;
            if (ms < 60) {
                m_value = ms / 60;
                m_scale = 1;
            } else if (ms < 360) {
                m_value = 1;
                m_scale = 1;
            } else {
                const double fade = (ms - 360) / 300;
                m_value = 1 - fade;
                m_scale = 1 + 0.3 * fade;
            }
            apply();
        };
        m_animation->onFinished = [this] {
            m_value = 0;
            m_scale = 1;
            apply();
        };
    }
    m_animation->stop();
    m_animation->start();
}

void Flash::apply()
{
    setVisible(m_value > 0);
    setOpacity(m_value);
    const RectF rect = localRect();
    Transform transform;
    // Scaled about its centre.
    transform.scale = m_scale;
    transform.translation = {rect.width / 2 * (1 - m_scale), rect.height / 2 * (1 - m_scale)};
    setTransform(transform);
}

void Flash::layout()
{
    m_icon->setGeometry({rounding::centeredStart(72, 32), rounding::centeredStart(72, 32), 32, 32});
}

void Flash::paint(Painter& painter)
{
    painter.drawRoundedRect(localRect(), {36, 0, Corners::Circular}, Color{0, 0, 0, 128});
}

RecentRow::RecentRow(const PlayerSettings::RecentFile& file, double resumeAt, bool last)
    : m_name(views::addLabel(this, file.name, 0, white()))
    , m_folder(views::addLabel(this, file.folder, 9, white(0.5)))
    , m_resume(views::addLabel(this, resumeAt > 0 ? ::Application::formatTime(resumeAt) : std::string(), 0,
                               white(0.5)))
    , m_last(last)
{
    m_name->setElide(Label::Elide::Middle);
    m_folder->setElide(Label::Elide::Middle);
    views::setTabular(m_resume);
    m_resume->setVisible(resumeAt > 0);
}

void RecentRow::layout()
{
    const double width = geometry().width;
    const double height = geometry().height;
    const double left = 16;
    double right = width - 16;
    if (m_resume->isVisible()) {
        const SizeF resume = m_resume->sizeHint();
        const double w = std::ceil(resume.width);
        m_resume->setGeometry({right - w, views::layoutCenter(height, resume.height), w,
                               resume.height});
        right -= w + 12;
    }
    const double nameHeight = std::ceil(m_name->sizeHint().height);
    const double folderHeight = std::ceil(m_folder->sizeHint().height);
    const double top = views::layoutCenter(height, nameHeight + 2 + folderHeight);
    m_name->setGeometry({left, top, std::max(0.0, right - left), m_name->sizeHint().height});
    m_folder->setGeometry({left, top + nameHeight + 2, std::max(0.0, right - left), m_folder->sizeHint().height});
}

void RecentRow::paint(Painter& painter)
{
    const RectF rect = localRect();
    if (m_hovered || m_pressed)
        painter.drawRoundedRect(rect.adjusted(4, 4, -4, -4), {Theme::current().mediumRadius, 0, Corners::Circular},
                                white(m_pressed ? 0.12 : 0.06));
    if (!m_last)
        views::paintSeparator(painter, 16, rect.height - 1 / painter.scale(), rect.width - 32, white(0.12));
}

// The card the recent rows sit in.
class RecentCard : public View
{
protected:
    void layout() override
    {
        double y = 0;
        for (const auto& child : children()) {
            child->setGeometry({0, y, geometry().width, 52});
            y += 52;
        }
    }

    void paint(Painter& painter) override
    {
        const Corners corners = Theme::current().continuousCorners ? Corners::Continuous : Corners::Circular;
        painter.drawRoundedRect(localRect(), {Theme::current().bigRadius, 0, corners}, white(0.08));
    }
};

// The column under the viewport: as tall as the viewport while it fits,
// centred in it.
class EmptyPage::Column : public View
{
public:
    explicit Column(EmptyPage& page)
        : m_page(page)
    {
        LayoutHints hints;
        hints.fillHeight = true;
        setLayoutHints(hints);
    }

    double heightForWidth(double) const override { return m_page.columnHeight(); }

protected:
    void layout() override { m_page.layoutColumn(geometry().width, geometry().height); }

private:
    EmptyPage& m_page;
};

EmptyPage::EmptyPage(MainWindow& owner)
    : m_owner(owner)
    , m_column(setContent<Column>(*this))
    , m_icon(m_column->addChild<ThemeIcon>("multimedia-video-player", 72))
    , m_title(views::addLabel(m_column, {}, 15, white(), 500))
    , m_subtitle(views::addLabel(m_column, {}, 0, white(0.65)))
    , m_openFile(m_column->addChild<views::PillButton>(std::string(), true))
    , m_openUrl(m_column->addChild<views::PillButton>(std::string()))
    , m_recentLabel(views::addLabel(m_column, {}, 0, white(0.5)))
    , m_clear(m_column->addChild<views::Clickable>())
    , m_clearLabel(views::addLabel(m_clear, {}))
    , m_card(m_column->addChild<RecentCard>())
{
    setName("emptyPage");
    setScrollDirections(false, true);
    setClipsChildren(true);
    views::darkenBars(this);
    m_openFile->setName("openFileButton");
    m_openUrl->setName("openUrlButton");
    m_card->setName("recentList");
    m_recentLabel->setElide(Label::Elide::Right);
    m_openFile->onClicked = [this] { m_owner.openFiles(false); };
    m_openUrl->onClicked = [this] { m_owner.openUrlDialog(); };
    m_clear->onClicked = [this] { m_owner.app().settings().clearRecentFiles(); };
    m_clear->onHoverChanged = [this] {
        m_clearLabel->setColor(m_clear->isHoveredNow() ? lighter(ColorId::Highlight, 1.2)
                                                       : ColorRecipe(ColorId::Highlight));
    };
    retranslate();
    reload();
}

void EmptyPage::retranslate()
{
    m_title->setText(tr("Drop a Video Here"));
    m_subtitle->setText(tr("Or open a file or a web address to start watching."));
    m_openFile->setText(tr("Open File…"));
    m_openUrl->setText(tr("Open URL…"));
    m_recentLabel->setText(tr("Recent"));
    m_clearLabel->setText(tr("Clear"));
    m_column->requestLayout();
}

void EmptyPage::reload()
{
    for (RecentRow* row : m_rows)
        m_card->takeChild(row);
    m_rows.clear();
    const std::vector<PlayerSettings::RecentFile> recent = m_owner.app().settings().recentFiles();
    const size_t count = std::min<size_t>(recent.size(), 5);
    for (size_t i = 0; i < count; ++i) {
        RecentRow* row = m_card->addChild<RecentRow>(recent[i], m_owner.app().resume().position(recent[i].url),
                                                     i + 1 == count);
        const std::string url = recent[i].url;
        row->onClicked = [this, url] { m_owner.app().openUrls({url}); };
        m_rows.push_back(row);
    }
    const bool hasRecent = count > 0;
    m_recentLabel->setVisible(hasRecent);
    m_clear->setVisible(hasRecent);
    m_card->setVisible(hasRecent);
    m_clearLabel->setColor(ColorId::Highlight);
    m_column->requestLayout();
}

// The column's own height, without the margin kept above and below it once
// it scrolls.
static double stackHeight(double titleHeight, double subtitleHeight, double recentRow, size_t rows)
{
    double total = 72 + 16 + titleHeight + 6 + subtitleHeight + 20 + 28;
    if (rows > 0)
        total += 32 + recentRow + 6 + 52.0 * double(rows);
    return total;
}

double EmptyPage::columnHeight() const
{
    const auto height = [](Label* label) { return std::ceil(label->sizeHint().height); };
    const double recentRow = m_rows.empty() ? 0 : std::max(height(m_recentLabel), height(m_clearLabel));
    return stackHeight(height(m_title), height(m_subtitle), recentRow, m_rows.size()) + 24 * 2;
}

void EmptyPage::layoutColumn(double pageWidth, double pageHeight)
{
    const double width = std::min(pageWidth - 48, 440.0);
    const auto height = [](Label* label) { return std::ceil(label->sizeHint().height); };
    const bool hasRecent = !m_rows.empty();
    const double recentRow = hasRecent ? std::max(height(m_recentLabel), height(m_clearLabel)) : 0;
    const double cardHeight = 52.0 * double(m_rows.size());
    const double total = stackHeight(height(m_title), height(m_subtitle), recentRow, m_rows.size());
    const double left = rounding::centeredStart(pageWidth, width);
    double y = rounding::centeredStart(pageHeight, total);

    m_icon->setGeometry({left + rounding::centeredStart(width, 72), y, 72, 72});
    y += 72 + 16;
    const auto centred = [&](Label* label) {
        const SizeF size = label->sizeHint();
        const double w = std::ceil(size.width);
        label->setGeometry({left + std::round((width - w) / 2), y, w, size.height});
        y += height(label);
    };
    centred(m_title);
    y += 6;
    centred(m_subtitle);
    y += 20;
    const double openFileWidth = std::ceil(m_openFile->sizeHint().width);
    const double openUrlWidth = std::ceil(m_openUrl->sizeHint().width);
    const double buttonsX = left + std::round((width - openFileWidth - 8 - openUrlWidth) / 2);
    m_openFile->setGeometry({buttonsX, y, openFileWidth, 28});
    m_openUrl->setGeometry({buttonsX + openFileWidth + 8, y, openUrlWidth, 28});
    y += 28;
    if (!hasRecent)
        return;
    y += 32;
    const double clearWidth = std::ceil(m_clearLabel->sizeHint().width);
    const double clearX = left + width - 16 - clearWidth;
    m_recentLabel->setGeometry({left + 16, y + views::layoutCenter(recentRow, height(m_recentLabel)),
                                std::max(0.0, clearX - 5 - left - 32), m_recentLabel->sizeHint().height});
    m_clear->setGeometry({clearX, y + views::layoutCenter(recentRow, height(m_clearLabel)), clearWidth,
                          m_clearLabel->sizeHint().height});
    m_clearLabel->setGeometry({0, 0, clearWidth, m_clearLabel->sizeHint().height});
    y += recentRow + 6;
    m_card->setGeometry({left, y, width, cardHeight});
}

ErrorPage::ErrorPage(MainWindow& owner)
    : m_title(views::addLabel(this, {}, 15, white(), 500))
    , m_detail(views::addLabel(this, {}, 0, white(0.65)))
    , m_another(addChild<views::PillButton>(std::string()))
    , m_next(addChild<views::PillButton>(std::string(), true))
{
    setName("errorPage");
    m_title->setTextAlignment(Label::TextAlignment::Center);
    m_detail->setTextAlignment(Label::TextAlignment::Center);
    m_detail->setWrap(true);
    m_another->onClicked = [&owner] { owner.openFiles(false); };
    m_next->onClicked = [&owner] { owner.playNext(false); };
    retranslate();
}

void ErrorPage::retranslate()
{
    m_title->setText(tr("Unable to Play This File"));
    m_another->setText(tr("Open Another File…"));
    m_next->setText(tr("Next File"));
    requestLayout();
}

void ErrorPage::update(const std::string& text, bool hasNext)
{
    m_detail->setText(text);
    m_next->setVisible(hasNext);
    requestLayout();
}

void ErrorPage::layout()
{
    const double width = std::min(geometry().width - 48, 420.0);
    const double titleHeight = std::ceil(m_title->sizeHint().height);
    const double detailHeight = std::ceil(m_detail->heightForWidth(width));
    const double total = titleHeight + 8 + detailHeight + 8 + 12 + 28;
    const double left = rounding::centeredStart(geometry().width, width);
    double y = rounding::centeredStart(geometry().height, total);
    m_title->setGeometry({left, y, width, m_title->sizeHint().height});
    y += titleHeight + 8;
    m_detail->setGeometry({left, y, width, detailHeight});
    y += detailHeight + 8 + 12;
    const double anotherWidth = std::ceil(m_another->sizeHint().width);
    const double nextWidth = m_next->isVisible() ? std::ceil(m_next->sizeHint().width) + 8 : 0;
    const double x = left + std::round((width - anotherWidth - nextWidth) / 2);
    m_another->setGeometry({x, y, anotherWidth, 28});
    if (m_next->isVisible())
        m_next->setGeometry({x + anotherWidth + 8, y, nextWidth - 8, 28});
}
