// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "base/event_loop.h"
#include "base/strings.h"

#include "History.h"
#include "KeyboardTranslator.h"
#include "ScreenWindow.h"
#include "Vt102Emulation.h"
#include "tools.h"

using namespace Konsole;

// Stands in for the pty: the program's output goes in, and what the terminal
// answers or types comes out.
class Terminal
{
public:
    Terminal()
        : m_window((set_kb_layout_dir(KB_LAYOUTS_DIR), m_emulation.createWindow()))
    {
        m_emulation.setKeyBindings(std::string());
        m_sentSubscription = m_emulation.sendData.connect([this](const char* data, int length) {
            m_sent.append(data, size_t(length));
        });
        m_commandSubscription = m_emulation.handleCommandFromKeyboard.connect([this](KeyboardTranslator::Command command) {
            m_commands.push_back(command);
        });
    }

    std::string print(const std::string& output)
    {
        m_emulation.receiveData(output.data(), int(output.size()));
        return std::exchange(m_sent, {});
    }

    std::string press(int key, uint32_t modifiers = NoModifier, const std::string& text = std::string())
    {
        KeyEvent event{key, modifiers, text};
        m_emulation.sendKeyEvent(&event, false);
        return std::exchange(m_sent, {});
    }

    // A cell of the screen's first line.
    Character cell(int column)
    {
        const Screen* screen = m_window->screen();
        std::vector<Character> line(size_t(screen->getColumns()));
        screen->getImage(line.data(), int(line.size()), screen->getHistLines(), screen->getHistLines());
        return line.at(size_t(column));
    }

    // A cell of the oldest line in the history.
    Character historyCell(int column)
    {
        const Screen* screen = m_window->screen();
        std::vector<Character> line(size_t(screen->getColumns()));
        screen->getImage(line.data(), int(line.size()), 0, 0);
        return line.at(size_t(column));
    }

    std::string line(int length)
    {
        std::string text;
        for (int column = 0; column < length; ++column)
            Lingmo::base::strings::appendUtf8(text, char32_t(char16_t(cell(column).character)));
        return text;
    }

    std::vector<KeyboardTranslator::Command> takeCommands() { return std::exchange(m_commands, {}); }

    void keepHistory() { m_emulation.setHistory(HistoryTypeBuffer(1000)); }
    int historyLines() const { return m_window->screen()->getHistLines(); }
    int cursorLine() const { return m_window->screen()->getCursorY(); }
    bool clearAll(bool atPrompt) { return m_emulation.clearScreenAndHistory(atPrompt); }
    std::string takeSent() { return std::exchange(m_sent, {}); }

private:
    Lingmo::base::EventLoop m_loop;
    Vt102Emulation m_emulation;
    ScreenWindow* m_window;
    std::string m_sent;
    std::vector<KeyboardTranslator::Command> m_commands;
    Lingmo::base::Subscription m_sentSubscription;
    Lingmo::base::Subscription m_commandSubscription;
};

TEST(EmulationTest, KeysStayAsTheyWereUntilAProgramAsks)
{
    Terminal terminal;
    EXPECT_EQ(terminal.print("\033[?u"), "\033[?0u");
    EXPECT_EQ(terminal.press(Key_Return, ShiftModifier, "\r"), "\033OM");
    EXPECT_EQ(terminal.press(Key_A, ControlModifier, "\x01"), "\x01");
}

TEST(EmulationTest, FlagsArePushedPoppedAndSet)
{
    Terminal terminal;
    // Only disambiguation is supported, so the query reports it alone.
    terminal.print("\033[>5u");
    EXPECT_EQ(terminal.print("\033[?u"), "\033[?1u");
    terminal.print("\033[>0u");
    EXPECT_EQ(terminal.print("\033[?u"), "\033[?0u");
    terminal.print("\033[<u");
    EXPECT_EQ(terminal.print("\033[?u"), "\033[?1u");
    terminal.print("\033[<5u");
    EXPECT_EQ(terminal.print("\033[?u"), "\033[?0u");

    terminal.print("\033[=1u");
    EXPECT_EQ(terminal.print("\033[?u"), "\033[?1u");
    terminal.print("\033[=1;3u");
    EXPECT_EQ(terminal.print("\033[?u"), "\033[?0u");
    terminal.print("\033[=1;2u");
    EXPECT_EQ(terminal.print("\033[?u"), "\033[?1u");

    terminal.print("\033c");
    EXPECT_EQ(terminal.print("\033[?u"), "\033[?0u");
}

TEST(EmulationTest, EachScreenKeepsItsOwnFlags)
{
    Terminal terminal;
    terminal.print("\033[>1u\033[?1049h");
    EXPECT_EQ(terminal.print("\033[?u"), "\033[?0u");
    terminal.print("\033[?1049l");
    EXPECT_EQ(terminal.print("\033[?u"), "\033[?1u");
}

struct KeyCase
{
    const char* name;
    int key;
    uint32_t modifiers;
    std::string text;
    std::string sent;
};

TEST(EmulationTest, KeysAreDisambiguated)
{
    const uint32_t none = NoModifier;
    const uint32_t shift = ShiftModifier;
    const uint32_t ctrl = ControlModifier;
    const uint32_t alt = AltModifier;
    const uint32_t keypad = KeypadModifier;

    const KeyCase cases[] = {
        {"enter", Key_Return, none, "\r", "\r"},
        {"shift+enter", Key_Return, shift, "\r", "\033[13;2u"},
        {"ctrl+enter", Key_Return, ctrl, "\r", "\033[13;5u"},
        {"tab", Key_Tab, none, "\t", "\t"},
        {"shift+tab", Key_Backtab, shift, "", "\033[9;2u"},
        {"backspace", Key_Backspace, none, "\b", "\x7f"},
        {"alt+backspace", Key_Backspace, alt, "\b", "\033[127;3u"},
        {"escape", Key_Escape, none, "\x1b", "\033[27u"},
        {"a", Key_A, none, "a", "a"},
        {"shift+a", Key_A, shift, "A", "A"},
        {"ctrl+a", Key_A, ctrl, "\x01", "\033[97;5u"},
        {"ctrl+shift+c", Key_C, ctrl | shift, "\x03", "\033[99;6u"},
        {"alt+b", Key_B, alt, "b", "\033[98;3u"},
        {"ctrl+shift+1", Key_Exclam, ctrl | shift, "!", "\033[49;6u"},
        {"ctrl+space", Key_Space, ctrl, " ", "\033[32;5u"},
        {"ctrl+cyrillic", 0x0424, ctrl, "\xd1\x84", "\033[1092;5u"},
        {"altgr text", Key_At, GroupSwitchModifier, "@", "@"},
        {"up", Key_Up, none, "", "\033[A"},
        {"ctrl+up", Key_Up, ctrl, "", "\033[1;5A"},
        {"f1", Key_F1, none, "", "\033[P"},
        {"shift+f1", Key_F1, shift, "", "\033[1;2P"},
        {"f3", Key_F3, none, "", "\033[13~"},
        {"f13", Key_F13, none, "", "\033[57376u"},
        {"ctrl+page down", Key_PageDown, ctrl, "", "\033[6;5~"},
        {"delete", Key_Delete, none, "\x7f", "\033[3~"},
        {"keypad enter", Key_Enter, keypad, "\r", "\033[57414u"},
        {"keypad 1", Key_1, keypad, "1", "1"},
        {"ctrl+keypad 1", Key_1, keypad | ctrl, "1", "\033[57400;5u"},
    };

    for (const KeyCase& key : cases) {
        Terminal terminal;
        terminal.print("\033[>1u");
        EXPECT_EQ(terminal.press(key.key, key.modifiers, key.text), key.sent) << key.name;
    }
}

TEST(EmulationTest, PastedTextGoesOutAsItIs)
{
    Terminal terminal;
    terminal.print("\033[>1u");
    EXPECT_EQ(terminal.press(0, NoModifier, "ls -l\r"), "ls -l\r");
}

TEST(EmulationTest, ScrollingKeysStillScroll)
{
    Terminal terminal;
    terminal.print("\033[>1u");
    EXPECT_EQ(terminal.press(Key_PageUp, ShiftModifier), "");
    const std::vector<KeyboardTranslator::Command> commands = terminal.takeCommands();
    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands.front(), KeyboardTranslator::ScrollPageUpCommand);
}

TEST(EmulationTest, PrefixedSequencesPrintNothing)
{
    Terminal terminal;
    terminal.print("a\033[=5;1ub\033[<uc\033[>4;2md");
    EXPECT_EQ(terminal.line(4), "abcd");
}

TEST(EmulationTest, ClearingTheScreenKeepsOnlyWrittenLinesInHistory)
{
    Terminal terminal;
    terminal.keepHistory();
    terminal.print("a\r\nb\r\n$ ");
    terminal.print("\033[H\033[2J$ ");
    EXPECT_EQ(terminal.historyLines(), 3);

    // The prompt alone at the top goes up once, the blank lines below it never.
    terminal.print("\033[H\033[2J$ ");
    terminal.print("\033[H\033[2J$ ");
    EXPECT_EQ(terminal.historyLines(), 5);

    terminal.print("\033[H\033[2J\033[H\033[2J");
    EXPECT_EQ(terminal.historyLines(), 6);
}

TEST(EmulationTest, ClearAllAtAPromptAsksTheShellToRedraw)
{
    Terminal terminal;
    terminal.keepHistory();
    terminal.print("a\r\nb\r\n$ \033[H\033[2J$ ls");
    EXPECT_EQ(terminal.historyLines(), 3);

    EXPECT_TRUE(terminal.clearAll(true));
    EXPECT_EQ(terminal.takeSent(), "\x0C");
    EXPECT_EQ(terminal.historyLines(), 0);
    EXPECT_EQ(terminal.line(4), "    ");

    // The shell's redraw leaves nothing behind in the history.
    terminal.print("\033[H\033[2J$ ls");
    EXPECT_EQ(terminal.historyLines(), 0);
    EXPECT_EQ(terminal.line(4), "$ ls");
}

TEST(EmulationTest, ClearAllUnderAProgramKeepsTheCursorLine)
{
    Terminal terminal;
    terminal.keepHistory();
    terminal.print("a\r\nb\r\nc");

    EXPECT_TRUE(terminal.clearAll(false));
    EXPECT_EQ(terminal.takeSent(), "");
    EXPECT_EQ(terminal.historyLines(), 0);
    EXPECT_EQ(terminal.cursorLine(), 0);
    EXPECT_EQ(terminal.line(1), "c");
}

TEST(EmulationTest, ClearAllLeavesTheAlternateScreenAlone)
{
    Terminal terminal;
    terminal.print("\033[?1049hx");
    EXPECT_FALSE(terminal.clearAll(true));
    EXPECT_EQ(terminal.takeSent(), "");
    EXPECT_EQ(terminal.line(1), "x");
}

TEST(EmulationTest, SubParametersStayWithTheirAttribute)
{
    Terminal terminal;
    terminal.print("\033[4:3mA\033[4:0mB\033[58;5;1mC\033[38:2::255:0:0mD\033[m");
    EXPECT_EQ(terminal.line(4), "ABCD");

    EXPECT_TRUE(terminal.cell(0).rendition & RE_UNDERLINE);
    EXPECT_FALSE(terminal.cell(0).rendition & RE_ITALIC);
    EXPECT_FALSE(terminal.cell(1).rendition & RE_UNDERLINE);
    // An underline colour sets no other attribute.
    EXPECT_EQ(terminal.cell(2).rendition, terminal.cell(1).rendition);
    EXPECT_TRUE(terminal.cell(3).foregroundColor == CharacterColor(COLOR_SPACE_RGB, 0xff0000));
}

TEST(EmulationTest, CombiningSequencesInHistorySurviveAPrune)
{
    Terminal terminal;
    terminal.keepHistory();
    terminal.print("e\xcc\x81\r\n");
    for (int i = 0; i < 60; ++i)
        terminal.print("\r\n");
    EXPECT_GT(terminal.historyLines(), 0);

    // Thousands of distinct sequences written over one short line, each
    // gone from the screen by the next, push the table past its pruning size.
    for (char32_t base = 0x100; base < 0x180; ++base) {
        std::string text = "\r";
        for (char32_t mark = 0x300; mark < 0x340; ++mark) {
            Lingmo::base::strings::appendUtf8(text, base);
            Lingmo::base::strings::appendUtf8(text, mark);
        }
        terminal.print(text);
    }

    const Character kept = terminal.historyCell(0);
    EXPECT_TRUE(kept.rendition & RE_EXTENDED_CHAR);
    ushort length = 0;
    const uint* chars = ExtendedCharTable::instance.lookupExtendedChar(kept.character, length);
    ASSERT_TRUE(chars);
    EXPECT_EQ(length, ushort(2));
    EXPECT_EQ(chars[0], uint('e'));
    EXPECT_EQ(chars[1], uint(0x301));
}

// The key names the shipped keytab uses, read case-insensitively into key codes.
TEST(EmulationTest, KeytabKeyNamesParseToKeyCodes)
{
    EXPECT_EQ(keyCodeFromName("PgUp"), Key_PageUp);
    EXPECT_EQ(keyCodeFromName("pgdown"), Key_PageDown);
    EXPECT_EQ(keyCodeFromName("Escape"), Key_Escape);
    EXPECT_EQ(keyCodeFromName("f12"), Key_F12);
    EXPECT_EQ(keyCodeFromName("a"), Key_A);
    EXPECT_EQ(keyCodeFromName("prior"), Key_unknown);
}
