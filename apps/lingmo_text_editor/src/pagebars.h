// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGEBARS_H
#define PAGEBARS_H

#include "controls.h"

#include "base/signal.h"
#include "base/timer.h"
#include "ui/text_field.h"
#include "ui/view.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
class Label;
class Menu;
class ProgressBar;
}

class Document;
class EditorPage;

// A text field that hands Return, Shift+Return and Escape to its owner.
class CardField : public Lingmo::ui::TextField
{
public:
    std::function<void(bool shift)> onReturn;
    std::function<void()> onEscape;

protected:
    bool keyPress(const Lingmo::ui::KeyEvent &event) override;
};

// One line under the text: where the cursor is, and the document's
// indentation, language, line endings and encoding, each a menu to change it.
class StatusBar : public Lingmo::ui::View
{
public:
    explicit StatusBar(EditorPage &page);
    ~StatusBar() override;

    // The document or the view changed what the bar shows.
    void update();
    void openEncodingMenu(Lingmo::ui::PointF windowPosition);

    Lingmo::ui::SizeF sizeHint() const override { return {0, 28}; }

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter &painter) override;

private:
    RoundButton *addItem(const std::string &name);
    void openIndentMenu(Lingmo::ui::PointF position);
    void openLanguageMenu(Lingmo::ui::PointF position);
    void openLineEndingMenu(Lingmo::ui::PointF position);

    EditorPage &m_page;
    RoundButton *m_position;
    RoundButton *m_zoom;
    RoundButton *m_overwrite;
    RoundButton *m_readOnly;
    RoundButton *m_indentation;
    RoundButton *m_language;
    RoundButton *m_lineEnding;
    RoundButton *m_encoding;
    std::vector<RoundButton *> m_items;
    std::unique_ptr<Lingmo::ui::Menu> m_menu;
};

// A problem with the document, shown above its text with what can be done
// about it: changed or deleted on disk, unreadable characters, permissions.
class InfoBar : public Lingmo::ui::View
{
public:
    explicit InfoBar(EditorPage &page);

    void update();
    const std::string &message() const { return m_message; }

    double heightForWidth(double width) const override;

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter &painter) override;

private:
    bool warning() const;

    EditorPage &m_page;
    Lingmo::ui::Label *m_label;
    std::vector<TextButton *> m_buttons;
    std::vector<std::function<void()>> m_actions;
    std::string m_message;
    int m_notice = 0;
};

// Find, and with the disclosure open, replace: a card floating over the top
// right of the text, which stays where it is. Matches are found while typing;
// the options sit inside the field.
class FindBar : public FloatingCard
{
public:
    explicit FindBar(EditorPage &page);
    ~FindBar() override;

    void open(const std::string &text, bool replace);
    void close(bool focusEditor = true);
    void find(bool backwards);
    // The search follows the field and the options.
    void update();
    void refresh();
    void retranslate();
    bool replacing() const { return m_replacing; }

    double implicitHeight() const;

protected:
    void layout() override;

private:
    RoundButton *addToggle(const std::string &label);
    void setTexts();
    void setReplacing(bool replacing);

    EditorPage &m_page;
    bool m_replacing = false;
    bool m_inSelection = false;
    RoundButton *m_disclosure;
    CardField *m_field;
    Lingmo::ui::Label *m_count;
    RoundButton *m_caseToggle;
    RoundButton *m_wordsToggle;
    RoundButton *m_regexToggle;
    RoundButton *m_selectionToggle;
    RoundButton *m_previous;
    RoundButton *m_next;
    RoundButton *m_close;
    CardField *m_replaceField;
    TextButton *m_replace;
    TextButton *m_replaceAll;
    Lingmo::base::Timer m_wrapped;
};

// "Line" or "line:column" in a small card where the find card floats; Enter
// jumps, Escape goes back to the text.
class GoToLineBar : public FloatingCard
{
public:
    explicit GoToLineBar(EditorPage &page);

    void open();
    void close();
    void accept();
    void refresh();

    static constexpr double implicitWidth = 300;
    double implicitHeight() const { return 32 + padding * 2; }

protected:
    void layout() override;

private:
    bool target(int *line, int *column) const;
    bool valid() const;

    EditorPage &m_page;
    CardField *m_field;
    RoundButton *m_close;
};

// "Opening …", its progress and Cancel, over the editor while a file loads.
class LoadingView : public Lingmo::ui::View
{
public:
    explicit LoadingView(EditorPage &page);

    void update();
    void retranslate();

protected:
    void layout() override;

private:
    EditorPage &m_page;
    Lingmo::ui::Label *m_label;
    Lingmo::ui::ProgressBar *m_progress;
    TextButton *m_cancel;
};

#endif
