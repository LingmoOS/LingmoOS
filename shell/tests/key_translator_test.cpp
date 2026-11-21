// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <linux/input-event-codes.h>

#include "input/key_translator.h"
#include "input/xkb_tracker.h"

using namespace Lingmo::Input;

namespace {

Timestamp at(int ms)
{
    return Timestamp(std::chrono::milliseconds(ms));
}

struct Delivered {
    uint32_t code;
    bool pressed;

    bool operator==(const Delivered &) const = default;
};

std::ostream &operator<<(std::ostream &stream, const Delivered &key)
{
    return stream << key.code << (key.pressed ? " down" : " up");
}

Delivered down(uint32_t code)
{
    return {code, true};
}

Delivered up(uint32_t code)
{
    return {code, false};
}

class KeyTranslatorTest : public testing::Test
{
protected:
    XkbTracker keymap;
    std::vector<Delivered> delivered;
    bool terminal = false;
    bool passthrough = false;
    bool buttonsHeld = false;
    KeyTranslator translator {&keymap, [this](Timestamp, uint32_t code, bool pressed) {
                                  delivered.push_back({code, pressed});
                              }};
    int time = 0;

    void SetUp() override
    {
        translator.setConvention(ShortcutConvention::Command);
        translator.setTerminalProvider([this] {
            return terminal;
        });
        translator.setPassthroughProvider([this] {
            return passthrough;
        });
        translator.setPointerButtonsProvider([this] {
            return buttonsHeld;
        });
    }

    void press(uint32_t code) { translator.onKey(at(++time), code, true); }

    void release(uint32_t code) { translator.onKey(at(++time), code, false); }

    void tap(uint32_t code)
    {
        press(code);
        release(code);
    }

    std::vector<Delivered> take() { return std::exchange(delivered, {}); }
};

} // namespace

TEST_F(KeyTranslatorTest, ThePcConventionPassesEveryKeyThroughAsItIs)
{
    translator.setConvention(ShortcutConvention::Pc);
    press(KEY_LEFTALT);
    tap(KEY_TAB);
    release(KEY_LEFTALT);
    tap(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTALT), down(KEY_TAB), up(KEY_TAB),
                                      up(KEY_LEFTALT), down(KEY_LEFTMETA), up(KEY_LEFTMETA)}),
              take());
}

TEST_F(KeyTranslatorTest, CommandAloneReachesNoClient)
{
    tap(KEY_LEFTMETA);
    tap(KEY_LEFTALT);
    tap(KEY_LEFTCTRL);
    EXPECT_TRUE(take().empty());
}

TEST_F(KeyTranslatorTest, CommandGoesOutAsCtrlWithTheKeyThatNeedsIt)
{
    press(KEY_LEFTMETA);
    EXPECT_TRUE(take().empty());
    tap(KEY_C);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>(
                      {down(KEY_LEFTCTRL), down(KEY_C), up(KEY_C), up(KEY_LEFTCTRL)}),
              take());
}

TEST_F(KeyTranslatorTest, ShiftGoesOutAtOnce)
{
    press(KEY_LEFTSHIFT);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTSHIFT)}), take());
    press(KEY_LEFTMETA);
    tap(KEY_Z);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTCTRL), down(KEY_Z), up(KEY_Z)}), take());
}

TEST_F(KeyTranslatorTest, TextKeysGoOutWithCtrlUnchanged)
{
    // No text navigation: Command+Left and Command+Backspace are Ctrl+Left and
    // Ctrl+Backspace, which a file view can bind as it likes.
    press(KEY_LEFTMETA);
    tap(KEY_LEFT);
    tap(KEY_BACKSPACE);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTCTRL), down(KEY_LEFT), up(KEY_LEFT),
                                      down(KEY_BACKSPACE), up(KEY_BACKSPACE),
                                      up(KEY_LEFTCTRL)}),
              take());
}

TEST_F(KeyTranslatorTest, AKeyReleasesWhatItWasSentAs)
{
    // The convention changing in between does not strand a key.
    press(KEY_LEFTMETA);
    press(KEY_A);
    translator.setConvention(ShortcutConvention::Pc);
    release(KEY_A);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>(
                      {down(KEY_LEFTCTRL), down(KEY_A), up(KEY_A), up(KEY_LEFTCTRL)}),
              take());
}

TEST_F(KeyTranslatorTest, OptionGoesOutAsAltWithTheKeyThatNeedsIt)
{
    press(KEY_LEFTALT);
    EXPECT_TRUE(take().empty());
    tap(KEY_LEFT);
    release(KEY_LEFTALT);
    EXPECT_EQ(std::vector<Delivered>(
                      {down(KEY_LEFTALT), down(KEY_LEFT), up(KEY_LEFT), up(KEY_LEFTALT)}),
              take());
}

TEST_F(KeyTranslatorTest, ControlGoesOutAsCtrl)
{
    press(KEY_LEFTCTRL);
    tap(KEY_A);
    tap(KEY_E);
    release(KEY_LEFTCTRL);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTCTRL), down(KEY_A), up(KEY_A), down(KEY_E),
                                      up(KEY_E), up(KEY_LEFTCTRL)}),
              take());
}

TEST_F(KeyTranslatorTest, TerminalsCopyWithCtrlShiftAndKeepControlForThePrograms)
{
    terminal = true;
    press(KEY_LEFTMETA);
    tap(KEY_C);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTCTRL), down(KEY_LEFTSHIFT), down(KEY_C),
                                      up(KEY_C), up(KEY_LEFTSHIFT), up(KEY_LEFTCTRL)}),
              take());
    press(KEY_LEFTMETA);
    tap(KEY_V);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTCTRL), down(KEY_LEFTSHIFT), down(KEY_V),
                                      up(KEY_V), up(KEY_LEFTSHIFT), up(KEY_LEFTCTRL)}),
              take());

    press(KEY_LEFTCTRL);
    tap(KEY_C);
    tap(KEY_A);
    release(KEY_LEFTCTRL);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTCTRL), down(KEY_C), up(KEY_C), down(KEY_A),
                                      up(KEY_A), up(KEY_LEFTCTRL)}),
              take());
}

TEST_F(KeyTranslatorTest, TerminalsOpenAndCloseTabsAndPickThemByNumber)
{
    terminal = true;
    press(KEY_LEFTMETA);
    tap(KEY_T);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTCTRL), down(KEY_LEFTSHIFT), down(KEY_T),
                                      up(KEY_T), up(KEY_LEFTSHIFT), up(KEY_LEFTCTRL)}),
              take());

    press(KEY_LEFTMETA);
    tap(KEY_3);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>(
                      {down(KEY_LEFTALT), down(KEY_3), up(KEY_3), up(KEY_LEFTALT)}),
              take());

    // Elsewhere Command+3 is Ctrl+3, which browsers pick tabs with.
    terminal = false;
    press(KEY_LEFTMETA);
    tap(KEY_3);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>(
                      {down(KEY_LEFTCTRL), down(KEY_3), up(KEY_3), up(KEY_LEFTCTRL)}),
              take());
}

TEST_F(KeyTranslatorTest, TerminalsGetNoOtherCommandLetters)
{
    // As Ctrl these would end the shell and suspend the program.
    terminal = true;
    press(KEY_LEFTMETA);
    tap(KEY_D);
    tap(KEY_Z);
    release(KEY_LEFTMETA);
    EXPECT_TRUE(take().empty());

    press(KEY_LEFTSHIFT);
    press(KEY_LEFTMETA);
    tap(KEY_D);
    release(KEY_LEFTMETA);
    release(KEY_LEFTSHIFT);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTSHIFT), up(KEY_LEFTSHIFT)}), take());

    // Other keys still go out as Ctrl.
    press(KEY_LEFTMETA);
    tap(KEY_LEFT);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>(
                      {down(KEY_LEFTCTRL), down(KEY_LEFT), up(KEY_LEFT), up(KEY_LEFTCTRL)}),
              take());

    // Other applications get Command+D as Ctrl+D.
    terminal = false;
    press(KEY_LEFTMETA);
    tap(KEY_D);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>(
                      {down(KEY_LEFTCTRL), down(KEY_D), up(KEY_D), up(KEY_LEFTCTRL)}),
              take());
}

TEST_F(KeyTranslatorTest, OtherApplicationsGetTheirShortcutsAsCtrl)
{
    // No rule depends on the application: browser and file manager keys go
    // out as Ctrl like any other.
    press(KEY_LEFTMETA);
    tap(KEY_LEFTBRACE);
    tap(KEY_1);
    tap(KEY_DOT);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTCTRL), down(KEY_LEFTBRACE),
                                      up(KEY_LEFTBRACE), down(KEY_1), up(KEY_1),
                                      down(KEY_DOT), up(KEY_DOT), up(KEY_LEFTCTRL)}),
              take());
}

TEST_F(KeyTranslatorTest, CommandAndControlShareOneCtrl)
{
    press(KEY_LEFTMETA);
    press(KEY_LEFTCTRL);
    tap(KEY_X);
    release(KEY_LEFTCTRL);
    tap(KEY_X);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTCTRL), down(KEY_X), up(KEY_X), down(KEY_X),
                                      up(KEY_X), up(KEY_LEFTCTRL)}),
              take());
}

TEST_F(KeyTranslatorTest, PassthroughSendsCommandAsMetaAtOnce)
{
    passthrough = true;
    press(KEY_LEFTMETA);
    tap(KEY_C);
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>(
                      {down(KEY_LEFTMETA), down(KEY_C), up(KEY_C), up(KEY_LEFTMETA)}),
              take());
}

TEST_F(KeyTranslatorTest, AClickTakesTheHeldModifiers)
{
    press(KEY_LEFTMETA);
    translator.flushModifiers(at(++time));
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTCTRL)}), take());
    release(KEY_LEFTMETA);
    EXPECT_EQ(std::vector<Delivered>({up(KEY_LEFTCTRL)}), take());

    // During a drag a modifier goes out as it goes down.
    buttonsHeld = true;
    press(KEY_LEFTALT);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_LEFTALT)}), take());
}

TEST_F(KeyTranslatorTest, ControlClickIsASecondaryClickWithoutCtrl)
{
    press(KEY_LEFTCTRL);
    EXPECT_EQ(uint32_t(BTN_RIGHT), translator.translateButton(BTN_LEFT, true));
    translator.flushModifiers(at(++time));
    EXPECT_TRUE(take().empty());
    release(KEY_LEFTCTRL);
    EXPECT_EQ(uint32_t(BTN_RIGHT), translator.translateButton(BTN_LEFT, false));

    press(KEY_LEFTMETA);
    EXPECT_EQ(uint32_t(BTN_LEFT), translator.translateButton(BTN_LEFT, true));
    EXPECT_EQ(uint32_t(BTN_LEFT), translator.translateButton(BTN_LEFT, false));
}

TEST_F(KeyTranslatorTest, AltGrStaysACharacterModifier)
{
    ASSERT_TRUE(keymap.updateKeyboardLayout("de"));
    press(KEY_RIGHTALT);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_RIGHTALT)}), take());
    tap(KEY_Q);
    release(KEY_RIGHTALT);
    EXPECT_EQ(std::vector<Delivered>({down(KEY_Q), up(KEY_Q), up(KEY_RIGHTALT)}), take());
}

TEST_F(KeyTranslatorTest, DeliveredKeysIncludeTheSentModifiers)
{
    press(KEY_LEFTMETA);
    press(KEY_C);
    std::vector<uint32_t> keys = translator.deliveredKeys();
    std::sort(keys.begin(), keys.end());
    EXPECT_EQ(std::vector<uint32_t>({KEY_LEFTCTRL, KEY_C}), keys);
}

TEST(IsTerminal, KnownTerminalsAreRecognised)
{
    EXPECT_TRUE(isTerminal("lingmo-terminal"));
    EXPECT_TRUE(isTerminal("org.gnome.Console"));
    EXPECT_FALSE(isTerminal("google-chrome"));
    EXPECT_FALSE(isTerminal("code"));
}
