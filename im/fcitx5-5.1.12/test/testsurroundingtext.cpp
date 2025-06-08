/*
 * SPDX-FileCopyrightText: 2016-2016 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "fcitx-utils/log.h"
#include "fcitx/surroundingtext.h"

using namespace fcitx;

int main() {
    SurroundingText surroundingText;
    FCITX_ASSERT(!surroundingText.isValid());
    surroundingText.setText("abcd", 1, 1);

    FCITX_ASSERT(surroundingText.isValid());
    FCITX_ASSERT(surroundingText.text() == "abcd");
    surroundingText.deleteText(-1, 2);
    FCITX_ASSERT(surroundingText.text() == "cd");
    FCITX_ASSERT(surroundingText.anchor() == 0);
    FCITX_ASSERT(surroundingText.cursor() == 0);
    FCITX_ASSERT(surroundingText.isValid());
    surroundingText.invalidate();
    FCITX_ASSERT(!surroundingText.isValid());

    surroundingText.setText("あ", 3, 3);
    FCITX_ASSERT(!surroundingText.isValid());

    surroundingText.setText("あ", 1, 1);
    FCITX_ASSERT(surroundingText.isValid());
    surroundingText.setCursor(2, 2);
    FCITX_ASSERT(!surroundingText.isValid());

    surroundingText.setText("abcd", 1, 1);
    SurroundingText other;
    FCITX_ASSERT(!other.isValid());
    other = surroundingText;
    FCITX_ASSERT(other.isValid());
    FCITX_ASSERT(other.text() == "abcd");
    FCITX_ASSERT(other.cursor() == 1);
    FCITX_ASSERT(other.anchor() == 1);

    FCITX_ASSERT(surroundingText.text() == "abcd");
    FCITX_ASSERT(surroundingText.cursor() == 1);
    FCITX_ASSERT(surroundingText.anchor() == 1);
    return 0;
}
