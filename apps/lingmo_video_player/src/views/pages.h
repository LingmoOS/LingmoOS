// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "widgets.h"

#include "playersettings.h"

#include "ui/scroll_view.h"

#include <functional>
#include <vector>

class MainWindow;

// A short message over the video, with an optional action.
class Osd : public Lingmo::ui::View
{
public:
    Osd();

    // From the plate's edge to its text when there is no action.
    static constexpr double kTextInset = 12;

    void show(const std::string& message, int timeoutMs, std::string actionText, std::function<void()> action);
    void hide();
    Lingmo::ui::SizeF sizeHint() const override;
    bool acceptsPointer() const override { return isVisible() && m_opacity.target() > 0; }

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;
    void hoverChanged() override;

private:
    void fadeTo(double opacity);

    Lingmo::ui::Label* m_label;
    views::PillButton* m_action;
    std::function<void()> m_callback;
    Lingmo::base::Timer m_hideTimer;
    views::Tween m_opacity{0, std::chrono::milliseconds(180)};
};

// The play or pause glyph a click on the video flashes.
class Flash : public Lingmo::ui::View
{
public:
    Flash();

    void show(const std::string& icon);

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;

private:
    void apply();

    Lingmo::ui::Icon* m_icon;
    std::unique_ptr<Lingmo::ui::Animation> m_animation;
    double m_value = 0;
    double m_scale = 1;
};

class RecentRow : public views::Clickable
{
public:
    explicit RecentRow(const PlayerSettings::RecentFile& file, double resumeAt, bool last);

    Lingmo::ui::SizeF sizeHint() const override { return {0, 52}; }

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;

private:
    Lingmo::ui::Label* m_name;
    Lingmo::ui::Label* m_folder;
    Lingmo::ui::Label* m_resume;
    bool m_last;
};

// How to open a video, and the recently watched ones; scrolls when the
// window is too short for them.
class EmptyPage : public Lingmo::ui::ScrollView
{
public:
    explicit EmptyPage(MainWindow& owner);

    // Re-read on each showing, which leaves out files deleted since.
    void reload();
    void retranslate();

private:
    class Column;

    double columnHeight() const;
    void layoutColumn(double pageWidth, double pageHeight);

    MainWindow& m_owner;
    Column* m_column;
    Lingmo::ui::ThemeIcon* m_icon;
    Lingmo::ui::Label* m_title;
    Lingmo::ui::Label* m_subtitle;
    views::PillButton* m_openFile;
    views::PillButton* m_openUrl;
    Lingmo::ui::Label* m_recentLabel;
    views::Clickable* m_clear;
    Lingmo::ui::Label* m_clearLabel;
    Lingmo::ui::View* m_card;
    std::vector<RecentRow*> m_rows;
};

// The page in place of the picture when a file cannot be played.
class ErrorPage : public Lingmo::ui::View
{
public:
    explicit ErrorPage(MainWindow& owner);

    void update(const std::string& text, bool hasNext);
    void retranslate();

protected:
    void layout() override;

private:
    Lingmo::ui::Label* m_title;
    Lingmo::ui::Label* m_detail;
    views::PillButton* m_another;
    views::PillButton* m_next;
};
