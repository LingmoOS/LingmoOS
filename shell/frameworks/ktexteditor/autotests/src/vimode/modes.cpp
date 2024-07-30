/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Miquel Sabaté Solà <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "modes.h"
#include <inputmode/kateviinputmode.h>
#include <katebuffer.h>
#include <kateconfig.h>
#include <katedocument.h>
#include <katerenderer.h>
#include <kateview.h>
#include <vimode/emulatedcommandbar/emulatedcommandbar.h>

#include <QMainWindow>
#include <QTest>

using namespace KTextEditor;

QTEST_MAIN(ModesTest)

// BEGIN: Normal mode.

void ModesTest::NormalMotionsTests()
{
    // Test moving around an empty document (nothing should happen)
    DoTest("", "jkhl", "");
    DoTest("", "ggG$0", "");

    // Testing "l"
    DoTest("bar", "lx", "br");
    DoTest("bar", "2lx", "ba");
    DoTest("0123456789012345", "13lx", "012345678901245");
    DoTest("bar", "10lx", "ba");

    // Testing "h"
    DoTest("bar", "llhx", "br");
    DoTest("bar", "10l10hx", "ar");
    DoTest("0123456789012345", "13l10hx", "012456789012345");
    DoTest("bar", "ll5hx", "ar");

    // Testing "j"
    DoTest("bar\nbar", "jx", "bar\nar");
    DoTest("bar\nbar", "10jx", "bar\nar");
    DoTest("bar\nbara", "lljx", "bar\nbaa");
    DoTest("0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n0\n1\n2\n3\n4\n5\n", "13jx", "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n0\n1\n2\n\n4\n5\n");

    // Testing "k"
    DoTest("bar\nbar", "jx", "bar\nar");
    DoTest("bar\nbar\nbar", "jj100kx", "ar\nbar\nbar");
    DoTest("0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n0\n1\n2\n3\n4\n5\n", "13j10kx", "0\n1\n2\n\n4\n5\n6\n7\n8\n9\n0\n1\n2\n3\n4\n5\n");

    // Testing "w"
    DoTest("bar", "wx", "ba");
    DoTest("foo bar", "wx", "foo ar");
    DoTest("foo bar", "lwx", "foo ar");
    DoTest("quux(foo, bar, baz);", "wxwxwxwx2wx", "quuxfoo ar baz;");
    DoTest("foo\nbar\nbaz", "wxwx", "foo\nar\naz");
    DoTest("1 2 3\n4 5 6", "ld3w", "1\n4 5 6");
    DoTest("foo\nbar baz", "gU2w", "FOO\nBAR baz");
    DoTest("FOO\nBAR BAZ", "gu2w", "foo\nbar BAZ");
    DoTest("bar(\n123", "llwrX", "barX\n123");

    // Testing "W"
    DoTest("bar", "Wx", "ba");
    DoTest("foo bar", "Wx", "foo ar");
    DoTest("foo bar", "2lWx", "foo ar");
    DoTest("quux(foo, bar, baz);", "WxWx", "quux(foo, ar, az);");
    DoTest("foo\nbar\nbaz", "WxWx", "foo\nar\naz");
    DoTest(" foo(bar xyz", "Wx", " oo(bar xyz");

    // Testing "b"
    DoTest("bar", "lbx", "ar");
    DoTest("foo bar baz", "2wbx", "foo ar baz");
    DoTest("foo bar", "w20bx", "oo bar");
    DoTest("quux(foo, bar, baz);", "2W4l2bx2bx", "quux(foo, ar, az);");
    DoTest("foo\nbar\nbaz", "WWbx", "foo\nar\nbaz");
    DoTest("  foo", "lbrX", "X foo");
    DoTest("  foo", "llbrX", "X foo");

    // Testing "B"
    DoTest("bar", "lBx", "ar");
    DoTest("foo bar baz", "2wBx", "foo ar baz");
    DoTest("foo bar", "w20Bx", "oo bar");
    DoTest("quux(foo, bar, baz);", "2W4lBBx", "quux(foo, ar, baz);");
    DoTest("foo\nbar", "WlBx", "foo\nar");

    // Testing "e"
    DoTest("quux(foo, bar, baz);", "exex2ex10ex", "quu(fo, bar baz)");
    DoTest("", "ce", "");
    DoTest(" ", "lceX", "X");
    DoTest("", "cE", "");

    // Testing "E"
    DoTest("quux(foo, bar, baz);", "ExEx10Ex", "quux(foo bar baz)");

    // Testing "$"
    DoTest("foo\nbar\nbaz", "$x3$x", "fo\nbar\nba");

    // Testing "0"
    DoTest(" foo", "$0x", "foo");

    // Testing "#" & "*"
    DoTest("1 1 1", "2#x", "1  1");
    DoTest("foo bar foo bar", "#xlll#x", "foo ar oo bar");
    DoTest("(foo (bar (foo( bar))))", "#xll#x", "(foo (ar (oo( bar))))");
    DoTest("(foo (bar (foo( bar))))", "*x", "(foo (bar (oo( bar))))");
    DoTest("foo bar foobar foo", "*rX", "foo bar foobar Xoo"); // Whole word only.
    DoTest("foo bar foobar foo", "$#rX", "Xoo bar foobar foo"); // Whole word only.
    DoTest("fOo foo fOo", "*rX", "fOo Xoo fOo"); // Case insensitive.
    DoTest("fOo foo fOo", "$#rX", "fOo Xoo fOo"); // Case insensitive.
    DoTest("fOo foo fOo", "*ggnrX", "fOo Xoo fOo"); // Flag that the search to repeat is case insensitive.
    DoTest("fOo foo fOo", "$#ggNrX", "fOo Xoo fOo"); // Flag that the search to repeat is case insensitive.
    DoTest("bar foo", "$*rX", "bar Xoo");
    DoTest("bar foo", "$#rX", "bar Xoo");
    // Test that calling # on the last, blank line of a document does not go into an infinite loop.
    DoTest("foo\n", "j#", "foo\n");

    // Testing "-"
    DoTest("0\n1\n2\n3\n4\n5", "5j-x2-x", "0\n1\n\n3\n\n5");
    DoTest("foo\n\t \t bar\nbaz", "3j-r.", "foo\n\t \t .ar\nbaz");

    // Testing "^"
    DoTest(" foo bar", "$^x", " oo bar");

    // Testing "gg"
    DoTest("1\n2\n3\n4\n5", "4jggx", "\n2\n3\n4\n5");

    // Testing "G"
    DoTest("1\n2\n3\n4\n5", "Gx", "1\n2\n3\n4\n");

    // Testing "ge"
    DoTest("quux(foo, bar, baz);", "9lgexgex$gex", "quux(fo bar, ba);");
    DoTest("foo", "llgerX", "Xoo");
    DoTest("   foo", "$gerX", "X  foo");
    DoTest("   foo foo", "$2gerX", "X  foo foo");

    // Testing "gE"
    DoTest("quux(foo, bar, baz);", "9lgExgEx$gEx", "quux(fo bar baz);");
    DoTest("   foo", "$gErX", "X  foo");
    DoTest("   foo foo", "$2gErX", "X  foo foo");
    DoTest("   !foo$!\"", "$gErX", "X  !foo$!\"");
    DoTest("   !foo$!\"", "$2gErX", "X  !foo$!\"");

    // Testing "|"
    DoTest("123456789", "3|rx4|rx8|rx1|rx", "x2xx567x9");

    // Testing "`"
    DoTest("foo\nbar\nbaz", "lmaj`arx", "fxo\nbar\nbaz");

    // Testing "'"
    DoTest("foo\nbar\nbaz", "lmaj'arx", "xoo\nbar\nbaz");

    // Testing "%"
    DoTest("foo{\n}\n", "$d%", "foo\n");
    DoTest("FOO{\nBAR}BAZ", "lllgu%", "FOO{\nbar}BAZ");
    DoTest("foo{\nbar}baz", "lllgU%", "foo{\nBAR}baz");
    DoTest("foo{\nbar\n}", "llly%p", "foo{{\nbar\n}\nbar\n}");
    // Regression bug for test where yanking with % would actually move the cursor.
    DoTest("a()", "y%x", "()");
    // Regression test for the bug I added fixing the bug above ;)
    DoTest("foo(bar)", "y%P", "foo(bar)foo(bar)");
    // Regression test for adjacent brackets
    DoTest("foo((bar)baz)", "y%P", "foo((bar)baz)foo((bar)baz)");
    // Regression test for "empty" bracket ranges
    DoTest("()123", "%%llx", "()23");

    // Testing percentage "<N>%"
    DoTest("10%\n20%\n30%\n40%\n50%\n60%\n70%\n80%\n90%\n100%", "20%dd", "10%\n30%\n40%\n50%\n60%\n70%\n80%\n90%\n100%");

    DoTest("10%\n20%\n30%\n40%\n50%\n60%\n70%\n80%\n90%\n100%", "50%dd", "10%\n20%\n30%\n40%\n60%\n70%\n80%\n90%\n100%");

    DoTest("10%\n20%\n30%\n40%\n50%\n60%\n70\n80%\n90%\n100%", "65%dd", "10%\n20%\n30%\n40%\n50%\n60%\n80%\n90%\n100%");

    DoTest("10%\n20%\n30%\n40%\n50%\n60%\n70%\n80%\n90%\n100%", "5j10%dd", "20%\n30%\n40%\n50%\n60%\n70%\n80%\n90%\n100%");

    // ctrl-left and ctrl-right.
    DoTest("foo bar xyz", "\\ctrl-\\rightrX", "foo Xar xyz");
    DoTest("foo bar xyz", "$\\ctrl-\\leftrX", "foo bar Xyz");

    // Enter / Return / "+"
    DoTest("foo\n\t \t bar", "\\enterr.", "foo\n\t \t .ar");
    DoTest("foo\n\t \t bar", "\\returnr.", "foo\n\t \t .ar");
    DoTest("foo\n\t \t bar", "+r.", "foo\n\t \t .ar");

    // TEXT OBJECTS
    DoTest("foo \"bar baz ('first', 'second' or 'third')\"", "8w2lci'", "foo \"bar baz ('first', '' or 'third')\"");

    DoTest("foo \"bar baz ('first', 'second' or 'third')\"", "8w2lca'", "foo \"bar baz ('first',  or 'third')\"");

    DoTest("foo \"bar baz ('first', 'second' or 'third')\"", "8w2lci(", "foo \"bar baz ()\"");

    DoTest("foo \"bar baz ('first', 'second' or 'third')\"", "8w2lci(", "foo \"bar baz ()\"");

    DoTest("foo \"bar baz ('first', 'second' or 'third')\"", "8w2lcib", "foo \"bar baz ()\"");
    // Quick test that bracket object works in visual mode.
    DoTest("foo \"bar baz ('first', 'second' or 'third')\"", "8w2lvibd", "foo \"bar baz ()\"");
    DoTest("foo \"bar baz ('first', 'second' or 'third')\"", "8w2lvabd", "foo \"bar baz \"");

    DoTest("foo \"bar baz ('first', 'second' or 'third')\"", "8w2lca)", "foo \"bar baz \"");

    DoTest("foo \"bar baz ('first', 'second' or 'third')\"", "8w2lci\"", "foo \"\"");

    DoTest("foo \"bar baz ('first', 'second' or 'third')\"", "8w2lda\"", "foo ");

    DoTest("foo \"bar [baz ({'first', 'second'} or 'third')]\"", "9w2lci[", "foo \"bar []\"");

    DoTest("foo \"bar [baz ({'first', 'second'} or 'third')]\"", "9w2lci]", "foo \"bar []\"");

    DoTest("foo \"bar [baz ({'first', 'second'} or 'third')]\"", "9w2lca[", "foo \"bar \"");

    DoTest("foo \"bar [baz ({'first', 'second'} or 'third')]\"", "9w2lci{", "foo \"bar [baz ({} or 'third')]\"");

    DoTest("foo \"bar [baz ({'first', 'second'} or 'third')]\"", "7w2lca}", "foo \"bar [baz ( or 'third')]\"");

    DoTest("{foo { bar { (baz) \"asd\" }} {1} {2} {3} {4} {5} }", "ldiB", "{}");

    // Inner/ A Word.
    DoTest("", "diw", "");
    DoTest(" ", "diw", "");
    DoTest("  ", "diw", "");
    DoTest("foo", "daw", "");
    DoTest("foo", "ldaw", "");
    DoTest("foo", "cawxyz\\esc", "xyz");
    DoTest("foo bar baz", "daw", "bar baz");
    DoTest("foo bar baz", "cawxyz\\esc", "xyzbar baz");
    DoTest("foo bar baz", "wdaw", "foo baz");
    DoTest("foo bar baz", "wldaw", "foo baz");
    DoTest("foo bar baz", "wlldaw", "foo baz");
    DoTest("foo bar baz", "wcawxyz\\esc", "foo xyzbaz");
    DoTest("foo bar baz", "wwdaw", "foo bar");
    DoTest("foo bar baz   ", "wwdaw", "foo bar ");
    DoTest("foo bar baz", "wwcawxyz\\esc", "foo barxyz");
    DoTest("foo bar baz\n123", "jdaw", "foo bar baz\n");
    DoTest("foo bar baz\n123", "jcawxyz\\esc", "foo bar baz\nxyz");
    DoTest("foo bar baz\n123", "wwdaw", "foo bar\n123");
    DoTest("foo bar baz\n123", "wwcawxyz\\esc", "foo barxyz\n123");
    DoTest("foo bar      baz\n123", "wwdaw", "foo bar\n123");
    DoTest("foo bar      baz\n123", "wwcawxyz\\esc", "foo barxyz\n123");
    DoTest("foo bar baz \n123", "wwdaw", "foo bar \n123");
    DoTest("foo bar baz \n123", "wwcawxyz\\esc", "foo bar xyz\n123");
    DoTest("foo bar      baz \n123", "wwdaw", "foo bar      \n123");
    DoTest("foo bar      baz \n123", "wwcawxyz\\esc", "foo bar      xyz\n123");
    DoTest("foo    bar", "llldaw", "foo");
    DoTest("foo    bar", "lllcawxyz\\esc", "fooxyz");
    DoTest("foo    bar", "lllldaw", "foo");
    DoTest("foo    bar", "llllcawxyz\\esc", "fooxyz");
    DoTest("    bar", "daw", "");
    DoTest("    bar", "ldaw", "");
    DoTest("    bar", "llldaw", "");
    DoTest("    bar", "lllldaw", "    ");
    DoTest("    bar", "cawxyz\\esc", "xyz");
    DoTest("    bar", "lcawxyz\\esc", "xyz");
    DoTest("    bar", "lllcawxyz\\esc", "xyz");
    DoTest("foo   ", "llldaw", "foo   ");
    DoTest("foo   ", "lllldaw", "foo   ");
    DoTest("foo   ", "llllldaw", "foo   ");
    DoTest("foo   ", "lllcawxyz\\esc", "foo  ");
    DoTest("foo   ", "llllcawxyz\\esc", "foo  ");
    DoTest("foo   ", "lllllcawxyz\\esc", "foo  ");
    DoTest("foo   \nbar", "llldaw", "foo");
    DoTest("foo   \nbar", "lllldaw", "foo");
    DoTest("foo   \nbar", "llllldaw", "foo");
    DoTest("foo   \nbar", "lllcawxyz\\esc", "fooxyz");
    DoTest("foo   \nbar", "llllcawxyz\\esc", "fooxyz");
    DoTest("foo   \nbar", "lllllcawxyz\\esc", "fooxyz");
    DoTest("foo   \n   bar", "jdaw", "foo   \n");
    DoTest("foo   \n   bar", "jldaw", "foo   \n");
    DoTest("foo   \n   bar", "jlldaw", "foo   \n");
    DoTest("foo   \n   bar", "jcawxyz\\esc", "foo   \nxyz");
    DoTest("foo   \n   bar", "jlcawxyz\\esc", "foo   \nxyz");
    DoTest("foo   \n   bar", "jllcawxyz\\esc", "foo   \nxyz");
    DoTest("foo bar", "2daw", "");
    DoTest("foo bar", "2cawxyz\\esc", "xyz");
    DoTest("foo bar baz", "2daw", "baz");
    DoTest("foo bar baz", "2cawxyz\\esc", "xyzbaz");
    DoTest("foo bar baz", "3daw", "");
    DoTest("foo bar baz", "3cawxyz\\esc", "xyz");
    DoTest("foo bar\nbaz", "2daw", "\nbaz");
    DoTest("foo bar\nbaz", "2cawxyz\\esc", "xyz\nbaz");
    DoTest("foo bar\nbaz 123", "3daw", "123");
    DoTest("foo bar\nbaz 123", "3cawxyz\\esc", "xyz123");
    DoTest("foo bar \nbaz 123", "3daw", "123");
    DoTest("foo bar \nbaz 123", "3cawxyz\\esc", "xyz123");
    DoTest("foo bar baz", "lll2daw", "foo");
    DoTest("foo bar baz", "lll2cawxyz\\esc", "fooxyz");
    DoTest("   bar baz", "2daw", "");
    DoTest("   bar baz", "2cawxyz\\esc", "xyz");
    DoTest("   bar baz 123", "2daw", " 123");
    DoTest("   bar baz 123", "2cawxyz\\esc", "xyz 123");
    DoTest("   bar baz\n123", "3daw", "");
    DoTest("   bar baz\n123", "3cawxyz\\esc", "xyz");
    DoTest("   bar baz\n  123", "3daw", "");
    DoTest("   bar baz\n  123", "3cawxyz\\esc", "xyz");
    DoTest("   bar baz\n  123", "2daw", "\n  123");
    DoTest("   bar baz\n  123", "2cawxyz\\esc", "xyz\n  123");
    DoTest("   bar baz\n  123 456 789", "j2daw", "   bar baz\n 789");
    DoTest("   bar baz\n  123 456 789", "j2cawxyz\\esc", "   bar baz\nxyz 789");
    DoTest("foo\nbar\n", "2daw", "");
    DoTest("bar baz\n123 \n456\n789 abc \njkl", "j4daw", "bar baz\njkl");
    DoTest("bar baz\n123 \n456\n789 abc \njkl", "j4cawxyz\\esc", "bar baz\nxyzjkl");
    DoTest("   bar baz\n  123 \n456\n789 abc \njkl", "j4daw", "   bar baz\njkl");
    DoTest("   bar baz\n  123 456 789", "j2cawxyz\\esc", "   bar baz\nxyz 789");
    DoTest("foo b123r xyz", "wdaw", "foo xyz");
    DoTest("foo b123r xyz", "wldaw", "foo xyz");
    DoTest("foo b123r xyz", "wlldaw", "foo xyz");
    DoTest("foo b123r xyz", "wllldaw", "foo xyz");
    DoTest("foo b123r xyz", "wlllldaw", "foo xyz");
    DoTest("1 2 3 4 5 6", "daw", "2 3 4 5 6");
    DoTest("1 2 3 4 5 6", "ldaw", "1 3 4 5 6");
    DoTest("1 2 3 4 5 6", "lldaw", "1 3 4 5 6");
    DoTest("1 2 3 4 5 6", "llldaw", "1 2 4 5 6");
    DoTest("!foo!", "ldaw", "!!");
    DoTest("! foo !", "ldaw", "! !");
    DoTest("! foo !", "lldaw", "! !");
    DoTest("! foo (", "l2daw", "!");
    DoTest("! foo(\n123", "l2daw", "!\n123");
    DoTest("  !foo(\n123", "lll2daw", "  !\n123");
    DoTest("  !!foo(\n123", "llll2daw", "  !!\n123");
    DoTest("  !foo( \n123", "lll2daw", "  !\n123");
    DoTest("  !!!!(", "llldaw", "  ");
    DoTest("  !!!!(", "lll2daw", "  !!!!(");
    DoTest("  !!!!(\n!!!", "lll2daw", "");
    DoTest("  !!!!(\n!!!", "llll2daw", "");

    // Inner/ A WORD
    // Behave the same as a Word if there are no non-word chars.
    DoTest("", "diW", "");
    DoTest(" ", "diW", "");
    DoTest("  ", "diW", "");
    DoTest("foo", "daW", "");
    DoTest("foo", "ldaW", "");
    DoTest("foo", "caWxyz\\esc", "xyz");
    DoTest("foo bar baz", "daW", "bar baz");
    DoTest("foo bar baz", "caWxyz\\esc", "xyzbar baz");
    DoTest("foo bar baz", "wdaW", "foo baz");
    DoTest("foo bar baz", "wldaW", "foo baz");
    DoTest("foo bar baz", "wlldaW", "foo baz");
    DoTest("foo bar baz", "wcaWxyz\\esc", "foo xyzbaz");
    DoTest("foo bar baz", "wwdaW", "foo bar");
    DoTest("foo bar baz   ", "wwdaW", "foo bar ");
    DoTest("foo bar baz", "wwcaWxyz\\esc", "foo barxyz");
    DoTest("foo bar baz\n123", "jdaW", "foo bar baz\n");
    DoTest("foo bar baz\n123", "jcaWxyz\\esc", "foo bar baz\nxyz");
    DoTest("foo bar baz\n123", "wwdaW", "foo bar\n123");
    DoTest("foo bar baz\n123", "wwcaWxyz\\esc", "foo barxyz\n123");
    DoTest("foo bar      baz\n123", "wwdaW", "foo bar\n123");
    DoTest("foo bar      baz\n123", "wwcaWxyz\\esc", "foo barxyz\n123");
    DoTest("foo bar baz \n123", "wwdaW", "foo bar \n123");
    DoTest("foo bar baz \n123", "wwcaWxyz\\esc", "foo bar xyz\n123");
    DoTest("foo bar      baz \n123", "wwdaW", "foo bar      \n123");
    DoTest("foo bar      baz \n123", "wwcaWxyz\\esc", "foo bar      xyz\n123");
    DoTest("foo    bar", "llldaW", "foo");
    DoTest("foo    bar", "lllcaWxyz\\esc", "fooxyz");
    DoTest("foo    bar", "lllldaW", "foo");
    DoTest("foo    bar", "llllcaWxyz\\esc", "fooxyz");
    DoTest("    bar", "daW", "");
    DoTest("    bar", "ldaW", "");
    DoTest("    bar", "llldaW", "");
    DoTest("    bar", "lllldaW", "    ");
    DoTest("    bar", "caWxyz\\esc", "xyz");
    DoTest("    bar", "lcaWxyz\\esc", "xyz");
    DoTest("    bar", "lllcaWxyz\\esc", "xyz");
    DoTest("foo   ", "llldaW", "foo   ");
    DoTest("foo   ", "lllldaW", "foo   ");
    DoTest("foo   ", "llllldaW", "foo   ");
    DoTest("foo   ", "lllcaWxyz\\esc", "foo  ");
    DoTest("foo   ", "llllcaWxyz\\esc", "foo  ");
    DoTest("foo   ", "lllllcaWxyz\\esc", "foo  ");
    DoTest("foo   \nbar", "llldaW", "foo");
    DoTest("foo   \nbar", "lllldaW", "foo");
    DoTest("foo   \nbar", "llllldaW", "foo");
    DoTest("foo   \nbar", "lllcaWxyz\\esc", "fooxyz");
    DoTest("foo   \nbar", "llllcaWxyz\\esc", "fooxyz");
    DoTest("foo   \nbar", "lllllcaWxyz\\esc", "fooxyz");
    DoTest("foo   \n   bar", "jdaW", "foo   \n");
    DoTest("foo   \n   bar", "jldaW", "foo   \n");
    DoTest("foo   \n   bar", "jlldaW", "foo   \n");
    DoTest("foo   \n   bar", "jcaWxyz\\esc", "foo   \nxyz");
    DoTest("foo   \n   bar", "jlcaWxyz\\esc", "foo   \nxyz");
    DoTest("foo   \n   bar", "jllcaWxyz\\esc", "foo   \nxyz");
    DoTest("foo bar", "2daW", "");
    DoTest("foo bar", "2caWxyz\\esc", "xyz");
    DoTest("foo bar baz", "2daW", "baz");
    DoTest("foo bar baz", "2caWxyz\\esc", "xyzbaz");
    DoTest("foo bar baz", "3daW", "");
    DoTest("foo bar baz", "3caWxyz\\esc", "xyz");
    DoTest("foo bar\nbaz", "2daW", "\nbaz");
    DoTest("foo bar\nbaz", "2caWxyz\\esc", "xyz\nbaz");
    DoTest("foo bar\nbaz 123", "3daW", "123");
    DoTest("foo bar\nbaz 123", "3caWxyz\\esc", "xyz123");
    DoTest("foo bar \nbaz 123", "3daW", "123");
    DoTest("foo bar \nbaz 123", "3caWxyz\\esc", "xyz123");
    DoTest("foo bar baz", "lll2daW", "foo");
    DoTest("foo bar baz", "lll2caWxyz\\esc", "fooxyz");
    DoTest("   bar baz", "2daW", "");
    DoTest("   bar baz", "2caWxyz\\esc", "xyz");
    DoTest("   bar baz 123", "2daW", " 123");
    DoTest("   bar baz 123", "2caWxyz\\esc", "xyz 123");
    DoTest("   bar baz\n123", "3daW", "");
    DoTest("   bar baz\n123", "3caWxyz\\esc", "xyz");
    DoTest("   bar baz\n  123", "3daW", "");
    DoTest("   bar baz\n  123", "3caWxyz\\esc", "xyz");
    DoTest("   bar baz\n  123", "2daW", "\n  123");
    DoTest("   bar baz\n  123", "2caWxyz\\esc", "xyz\n  123");
    DoTest("   bar baz\n  123 456 789", "j2daW", "   bar baz\n 789");
    DoTest("   bar baz\n  123 456 789", "j2caWxyz\\esc", "   bar baz\nxyz 789");
    DoTest("foo\nbar\n", "2daW", "");
    DoTest("bar baz\n123 \n456\n789 abc \njkl", "j4daW", "bar baz\njkl");
    DoTest("bar baz\n123 \n456\n789 abc \njkl", "j4caWxyz\\esc", "bar baz\nxyzjkl");
    DoTest("   bar baz\n  123 \n456\n789 abc \njkl", "j4daW", "   bar baz\njkl");
    DoTest("   bar baz\n  123 456 789", "j2caWxyz\\esc", "   bar baz\nxyz 789");
    DoTest("foo b123r xyz", "wdaW", "foo xyz");
    DoTest("foo b123r xyz", "wldaW", "foo xyz");
    DoTest("foo b123r xyz", "wlldaW", "foo xyz");
    DoTest("foo b123r xyz", "wllldaW", "foo xyz");
    DoTest("foo b123r xyz", "wlllldaW", "foo xyz");
    DoTest("1 2 3 4 5 6", "daW", "2 3 4 5 6");
    DoTest("1 2 3 4 5 6", "ldaW", "1 3 4 5 6");
    DoTest("1 2 3 4 5 6", "lldaW", "1 3 4 5 6");
    DoTest("1 2 3 4 5 6", "llldaW", "1 2 4 5 6");
    // Now with non-word characters.
    DoTest("fo(o", "daW", "");
    DoTest("fo(o", "ldaW", "");
    DoTest("fo(o", "lldaW", "");
    DoTest("fo(o", "llldaW", "");
    DoTest("fo(o )!)!)ffo", "2daW", "");
    DoTest("fo(o", "diW", "");
    DoTest("fo(o", "ldiW", "");
    DoTest("fo(o", "lldiW", "");
    DoTest("fo(o", "llldiW", "");
    DoTest("foo \"\"B!!", "fBdaW", "foo");

    // Inner / Sentence text object ("is")
    DoTest("", "cis", "");
    DoTest("hello", "cis", "");
    DoTest("hello", "flcis", "");
    DoTest("hello. bye", "cisX", "X bye");
    DoTest("hello. bye", "f.cisX", "X bye");
    DoTest("hello.  bye", "fbcisX", "hello.  X");
    DoTest("hello\n\nbye.", "cisX", "X\n\nbye.");
    DoTest("Hello. Bye.\n", "GcisX", "Hello. Bye.\nX");
    DoTest("hello. by.. another.", "cisX", "X by.. another.");
    DoTest("hello. by.. another.", "fbcisX", "hello. X another.");
    DoTest("hello. by.. another.\n", "GcisX", "hello. by.. another.\nX");
    DoTest("hello. yay\nis this a string?!?.. another.\n", "fycisX", "hello. X another.\n");
    DoTest("hello. yay\nis this a string?!?.. another.\n", "jcisX", "hello. X another.\n");
    DoTest("This is a sentence.\nThis is another sentence.", "jdis", "This is a sentence.\n");

    // Around / Sentence text object ("as")
    DoTest("", "cas", "");
    DoTest("hello", "cas", "");
    DoTest("hello", "flcas", "");
    DoTest("hello. bye", "casX", "Xbye");
    DoTest("hello. bye", "f.casX", "Xbye");
    DoTest("hello. bye.", "fbcasX", "hello.X");
    DoTest("hello. bye", "fbcasX", "hello.X");
    DoTest("hello\n\nbye.", "casX", "X\n\nbye.");
    DoTest("Hello. Bye.\n", "GcasX", "Hello. Bye.\nX");
    DoTest("hello. by.. another.", "casX", "Xby.. another.");
    DoTest("hello. by.. another.", "fbcasX", "hello. Xanother.");
    DoTest("hello. by.. another.\n", "GcasX", "hello. by.. another.\nX");
    DoTest("hello. yay\nis this a string?!?.. another.\n", "fycasX", "hello. Xanother.\n");
    DoTest("hello. yay\nis this a string?!?.. another.\n", "jcasX", "hello. Xanother.\n");
    DoTest("hello. yay\nis this a string?!?.. \t       another.\n", "jcasX", "hello. Xanother.\n");

    // Inner / Paragraph text object ("ip")
    DoTest("", "cip", "");
    DoTest("\nhello", "cipX", "X\nhello");
    DoTest("\nhello\n\nanother. text.", "jcipX", "\nX\n\nanother. text.");
    DoTest("\nhello\n\n\nanother. text.", "jjcipX", "\nhello\nX\nanother. text.");
    DoTest("\nhello\n\n\nanother. text.", "jjjcipX", "\nhello\nX\nanother. text.");
    DoTest("\nhello\n\n\nanother. text.", "jjjjcipX", "\nhello\n\n\nX");
    DoTest("hello\n\n", "jcipX", "hello\nX");
    DoTest("hello\n\n", "jjcipX", "hello\nX");

    // Around / Paragraph text object ("ap")
    DoTest("", "cap", "");
    DoTest("\nhello", "capX", "X");
    DoTest("\nhello\n\nanother.text.", "jcapX", "\nX\nanother.text.");
    DoTest("\nhello\n\nanother.text.\n\n\nAnother.", "jjjcapX", "\nhello\n\nX\nAnother.");
    DoTest("\nhello\n\nanother.text.\n\n\nAnother.", "jjjjjcapX", "\nhello\n\nanother.text.\nX");
    DoTest("hello\n\n\n", "jjcapX", "hello\n\n\n");
    DoTest("hello\n\nasd", "jjjcapX", "hello\nX");

    DoTest("{\nfoo\n}", "jdiB", "{\n}");
    DoTest("{\n}", "diB", "{\n}");
    DoTest("{\nfoo}", "jdiB", "{\n}");
    DoTest("{foo\nbar\nbaz}", "jdiB", "{}");
    DoTest("{foo\nbar\n  \t\t }", "jdiB", "{\n  \t\t }");
    DoTest("{foo\nbar\n  \t\ta}", "jdiB", "{}");
    DoTest("\t{\n\t}", "ldiB", "\t{\n\t}");
    // Quick test to see whether inner curly bracket works in visual mode.
    DoTest("{\nfoo}", "jviBd", "{\n}");
    DoTest("{\nfoo}", "jvaBd", "");
    // Regression test for viB not working if there is a blank line before the closing }.
    DoTest("{\nfoo\n\n}", "viBd", "{\n}");
    // The inner block text object does not include the line containing the opening brace if
    // the opening brace is the last character on its line and there is only whitespace before the closing brace.
    // (In particular: >iB should not indent the line containing the opening brace under these conditions).
    DoTest("{\nfoo\n}", "j>iB", "{\n  foo\n}");
    // Similarly, in such conditions, deleting the inner block should leave the cursor on closing brace line, not the
    // opening.
    DoTest("{\nfoo\n}", "jdiBiX", "{\nX}");
    // Yanking and pasting such a text object should be treated as linewise.
    DoTest("{\nfoo\nbar\n}", "jyiBjp", "{\nfoo\nbar\nfoo\nbar\n}");
    // Changing such a text object should delete everything but one line, which we will begin insertion at.
    DoTest("{\nfoo\nbar\n}", "jciBbaz\\esc", "{\nbaz\n}");
    // Make sure we remove the "last motion was a *linewise* curly text object" flag when we next parse a motion!
    DoTest("{\nfoo\n}", "jciBbaz xyz\\escdiw", "{\nbaz \n}");
    DoTest("{\nfoo\nbar\n}", "jviBbd", "{\nar\n}");

    DoTest("int main() {\n  printf( \"HelloWorld!\\n\" );\n  return 0;\n} ", "jda}xr;", "int main();");

    DoTest("QList<QString>", "wwldi>", "QList<>");
    DoTest("QList<QString>", "wwlda<", "QList");
    DoTest("<>\n<title>Title</title>\n</head>", "di<jci>\\ctrl-c", "<>\n<>Title</title>\n</head>");

    DoTest("foo bar baz", "wldiw", "foo  baz");

    DoTest("foo bar baz", "wldawx", "foo az");

    DoTest("foo ( \n bar\n)baz", "jdi(", "foo ()baz");
    DoTest("foo ( \n bar\n)baz", "jda(", "foo baz");
    DoTest("(foo(bar)baz)", "ldi)", "()");
    DoTest("(foo(bar)baz)", "lca(\\ctrl-c", "");
    DoTest("( foo ( bar ) )baz", "di(", "()baz");
    DoTest("( foo ( bar ) )baz", "da(", "baz");
    DoTest("[foo [ bar] [(a)b [c]d ]]", "$hda]", "[foo [ bar] ]");
    DoTest("(a)", "di(", "()");
    DoTest("(ab)", "di(", "()");
    DoTest("(abc)", "di(", "()");

    DoTest("hi!))))}}]]", "di]di}da)di)da]", "hi!))))}}]]");

    DoTest("foo \"bar\" baz", "4ldi\"", "foo \"\" baz");
    DoTest("foo \"bar\" baz", "8lca\"\\ctrl-c", "foo  baz");

    DoTest("foo 'bar' baz", "4lca'\\ctrl-c", "foo  baz");
    DoTest("foo 'bar' baz", "8ldi'", "foo '' baz");

    DoTest("foo `bar` baz", "4lca`\\ctrl-c", "foo  baz");
    DoTest("foo `bar` baz", "8ldi`", "foo `` baz");

    DoTest("()", "di(", "()");
    DoTest("\"\"", "di\"", "\"\"");

    // Comma text object
    DoTest("func(aaaa);", "llllldi,", "func();");
    DoTest("func(aaaa);", "lllllda,", "func;");
    DoTest("//Hello, world!\nfunc(a[0] > 2);", "jf>di,", "//Hello, world!\nfunc();");
    DoTest("//Hello, world!\nfunc(a[0] > 2);", "jf>da,", "//Hello, world!\nfunc;");
    DoTest("//Hello, world!\na[] = {135};", "jf3di,", "//Hello, world!\na[] = {};");

    // Some corner case tests for t/ T, mainly dealing with how a ; after e.g. a ta will
    // start searching for the next a *after* the character after the cursor.
    // Hard to explain; I'll let the test-cases do the talking :)
    DoTest("bar baz", "ta;x", "bar az");
    // Ensure we reset the flag that says we must search starting from the character after the cursor!
    DoTest("bar baz", "ta;^tax", "ar baz");
    // Corresponding tests for T
    DoTest("bar baz", "$Ta;x", "ba baz");
    // Ensure we reset the flag that says we must search starting from the character before the cursor!
    DoTest("bar baz", "$Ta;$Tax", "bar ba");
    // Ensure that command backwards works, too - only one test, as any additional ones would
    // just overlap with our previous ones.
    DoTest("aba bar", "lTa,x", "aba ar");
    // Some tests with counting.
    DoTest("aba bar", "2tax", "aba ar");
    // If we can't find 3 further a's, don't move at all...
    DoTest("aba bar", "3tax", "ba bar");
    // ... except if we are repeating the last search, in which case stop at the last
    // one that we do find.
    DoTest("aba bar", "ta2;x", "aba ar");

    // Don't move if we can't find any matches at all, or fewer than we require.
    DoTest("nocapitalc", "lltCx", "noapitalc");
    DoTest("nocapitalc", "llTCx", "noapitalc");

    DoTest("123c456", "2tcx", "23c456");
    DoTest("123c456", "$2Tcx", "123c45");
    // Commands with searches that do not find anything, or find less than required, should do nothing.
    DoTest("foo", "dtk", "foo");
    DoTest("foomxyz", "d2tm", "foomxyz");
    DoTest("foo", "dfk", "foo");
    DoTest("foomxyz", "d2fm", "foomxyz");
    DoTest("foo", "$dTk", "foo");
    DoTest("foomxyz", "$d2Fm", "foomxyz");
    // They should also return a range marked as invalid.
    DoTest("foo bar", "gUF(", "foo bar");
    DoTest("foo bar", "gUf(", "foo bar");
    DoTest("foo bar", "gUt(", "foo bar");
    DoTest("foo bar", "gUT(", "foo bar");

    // Changes using backward motions don't consume cursor character
    DoTest("foo bar", "$dTf", "fr");
    DoTest("foo bar", "$c2Fo", "fr");

    // Regression test for special-handling of "/" and "?" keys: these shouldn't interfere
    // with character searches.
    DoTest("foo /", "f/rX", "foo X");
    // d{f,F}{/,?}
    DoTest("foo/bar?baz", "df/", "bar?baz");
    DoTest("foo/bar?baz", "f/df?", "foobaz");
    DoTest("foo/bar?baz", "df?", "baz");
    DoTest("foo/bar?baz", "f?dF/", "foo?baz");
    // d{t,T}{/,?}
    DoTest("foo/bar?baz", "dt/", "/bar?baz");
    DoTest("foo/bar?baz", "t/dt?", "fo?baz");
    DoTest("foo/bar?baz", "dt?", "?baz");
    DoTest("foo/bar?baz", "t?dT/", "foo/r?baz");
    // c{f,F}{/,?}
    DoTest("foo/bar?baz", "cf/qux\\esc", "quxbar?baz");
    DoTest("foo/bar?baz", "f/cf?qux\\esc", "fooquxbaz");
    DoTest("foo/bar?baz", "cf?qux\\esc", "quxbaz");
    DoTest("foo/bar?baz", "f?cF/qux\\esc", "fooqux?baz");
    // c{t,T}{/,?}
    DoTest("foo/bar?baz", "ct/qux\\esc", "qux/bar?baz");
    DoTest("foo/bar?baz", "t/ct?qux\\esc", "foqux?baz");
    DoTest("foo/bar?baz", "ct?qux\\esc", "qux?baz");
    DoTest("foo/bar?baz", "t?cT/qux\\esc", "foo/quxr?baz");
    // y{f,F}{/,?}
    DoTest("foo/bar?baz", "yf/p", "ffoo/oo/bar?baz");
    DoTest("foo/bar?baz", "f/yf?p", "foo//bar?bar?baz");
    DoTest("foo/bar?baz", "yf?p", "ffoo/bar?oo/bar?baz");
    DoTest("foo/bar?baz", "f?yF/p", "foo/bar?/barbaz");
    // y{t,T}{/,?}
    DoTest("foo/bar?baz", "yt/p", "ffoooo/bar?baz");
    DoTest("foo/bar?baz", "t/yt?p", "fooo/bar/bar?baz");
    DoTest("foo/bar?baz", "yt?p", "ffoo/baroo/bar?baz");
    DoTest("foo/bar?baz", "t?yT/p", "foo/barba?baz");

    // gU, gu, g~.
    DoTest("foo/bar?baz", "gUf/", "FOO/bar?baz");
    DoTest("FOO/bar?baz", "g~f?", "foo/BAR?baz");
    DoTest("foo/BAR?baz", "guf?", "foo/bar?baz");

    // Not adding tests for =f/, >t?, <F?, gqT/ :
    //  Not likely to be used with those motions.
    // gw and g@ are currently not supported by ktexteditor's vimode

    // Using registers
    DoTest("foo/bar?baz", "\"2df?", "baz");
    DoTest("foo/bar?baz", "\"_ct/qux", "qux/bar?baz");

    // counted find on change/deletion != find digit
    DoTest("foo2barbaz", "df2ax", "bxarbaz");
    DoTest("foo2barbaz", "d2fax", "");

    // Motion to lines starting with { or }
    DoTest("{\nfoo\n}", "][x", "{\nfoo\n");
    DoTest("{\nfoo\n}", "j[[x", "\nfoo\n}");
    DoTest("bar\n{\nfoo\n}", "]]x", "bar\n\nfoo\n}");
    DoTest("{\nfoo\n}\nbar", "jjj[]x", "{\nfoo\n\nbar");
    DoTest("bar\nfoo\n}", "d][", "}");
    DoTest("bar\n{\nfoo\n}", "d]]", "{\nfoo\n}");
    DoTest("bar\nfoo\n}", "ld][", "b\n}");
    DoTest("{\nfoo\n}", "jld[[", "oo\n}");
    DoTest("bar\n{\nfoo\n}", "ld]]", "b\n{\nfoo\n}");
    DoTest("{\nfoo\n}\nbar", "jjjld[]", "{\nfoo\nar");

    // Testing the "(" motion
    DoTest("", "(", "");
    DoTest("\nhello.", "fh(iX", "X\nhello.");
    DoTest("\n   hello.", "jfe(iX", "X\n   hello.");
    DoTest("hello. world.", "fr(iX", "Xhello. world.");
    DoTest("hello. world.\n", "j(iX", "hello. Xworld.\n");
    DoTest("hello. world\nyay. lol.\n", "jfl(iX", "hello. Xworld\nyay. lol.\n");
    DoTest("Hello.\n\n", "jj(iX", "XHello.\n\n");
    DoTest("\nHello.", "j(iX", "X\nHello.");
    DoTest("\n\n\nHello.", "jj(iX", "X\n\n\nHello.");
    DoTest("Hello! Bye!", "fB(iX", "XHello! Bye!");
    DoTest("Hello! Bye! Hye!", "fH(iX", "Hello! XBye! Hye!");
    DoTest("\nHello. Bye.. Asd.\n\n\n\nAnother.", "jjjj(iX", "\nHello. Bye.. XAsd.\n\n\n\nAnother.");

    // Testing the ")" motion
    DoTest("", ")", "");
    DoTest("\nhello.", ")iX", "\nXhello.");
    DoTest("hello. world.", ")iX", "hello. Xworld.");
    DoTest("hello. world\n\nasd.", "))iX", "hello. world\nX\nasd.");
    DoTest("hello. wor\nld.?? Asd", "))iX", "hello. wor\nld.?? XAsd");
    DoTest("hello. wor\nld.?? Asd", "jfA(iX", "hello. Xwor\nld.?? Asd");
    DoTest("Hello.\n\n\nWorld.", ")iX", "Hello.\nX\n\nWorld.");
    DoTest("Hello.\n\n\nWorld.", "))iX", "Hello.\n\n\nXWorld.");
    DoTest("Hello.\n\n", ")iX", "Hello.\nX\n");
    DoTest("Hello.\n\n", "))iX", "Hello.\n\nX");
    DoTest("Hello. ", ")aX", "Hello. X");
    DoTest("Hello?? Bye!", ")iX", "Hello?? XBye!");

    // Testing "{" and "}" motions
    DoTest("", "{}", "");
    DoTest("foo", "{}dd", "");
    DoTest("foo\n\nbar", "}dd", "foo\nbar");
    DoTest("foo\n\nbar\n\nbaz", "3}x", "foo\n\nbar\n\nba");
    DoTest("foo\n\nbar\n\nbaz", "3}{dd{dd", "foo\nbar\nbaz");
    DoTest("foo\nfoo\n\nbar\n\nbaz", "5}{dd{dd", "foo\nfoo\nbar\nbaz");
    DoTest("foo\nfoo\n\nbar\n\nbaz", "5}3{x", "oo\nfoo\n\nbar\n\nbaz");
    DoTest("foo\n\n\nbar", "10}{{x", "oo\n\n\nbar");
    DoTest("foo\n\n\nbar", "}}x", "foo\n\n\nba");
    DoTest("foo\n\n\nbar\n", "}}dd", "foo\n\n\nbar");

    // Testing the position of the cursor in some cases of the "c" command.
    DoTest("(a, b, c)", "cibX", "(X)");
    DoTest("(a, b, c)", "f)cibX", "(X)");
    DoTest("(a, b, c)", "ci(X", "(X)");
    DoTest("(a, b, c)", "ci)X", "(X)");
    DoTest("[a, b, c]", "ci[X", "[X]");
    DoTest("[a, b, c]", "ci]X", "[X]");
    DoTest("{a, b, c}", "ciBX", "{X}");
    DoTest("{a, b, c}", "ci{X", "{X}");
    DoTest("{a, b, c}", "ci}X", "{X}");
    DoTest("<a, b, c>", "ci<X", "<X>");
    DoTest("<a, b, c>", "ci>X", "<X>");

    // Things like "cn" and "cN" don't crash.
    DoTest("Hello", "cn", "Hello");
    DoTest("Hello", "cN", "Hello");
}

void ModesTest::NormalCommandsTests()
{
    // Testing "J"
    DoTest("foo\nbar", "J", "foo bar");
    DoTest("foo\nbar", "JrX", "fooXbar");
    DoTest("foo\nbar\nxyz\n123", "3J", "foo bar xyz\n123");
    DoTest("foo\nbar\nxyz\n123", "3JrX", "foo barXxyz\n123");
    DoTest("foo\nbar\nxyz\n12345\n789", "4JrX", "foo bar xyzX12345\n789");
    DoTest("foo\nbar\nxyz\n12345\n789", "6JrX", "Xoo\nbar\nxyz\n12345\n789");
    DoTest("foo\nbar\nxyz\n12345\n789", "j5JrX", "foo\nXar\nxyz\n12345\n789");
    DoTest("foo\nbar\nxyz\n12345\n789", "7JrX", "Xoo\nbar\nxyz\n12345\n789");
    DoTest("\n\n", "J", "\n");
    DoTest("foo\n\t   \t\t  bar", "JrX", "fooXbar");
    DoTest("foo\n\t   \t\t", "J", "foo ");
    DoTest("foo\n\t   \t\t", "JrX", "fooX");

    // Testing "dd"
    DoTest("foo\nbar", "dd", "bar");
    DoTest("foo\nbar", "2dd", "");
    DoTest("foo\nbar\n", "Gdd", "foo\nbar");
    // Testing numbered registers
    DoTest("hello\nworld", "dddd\"2p\"1p", "\nhello\nworld");
    DoTest("foo\nbar", "ddD\"1p", "\nfoo");
    DoTest("abc\nlmn\nxyz", "dddd\"2dd\"1p\"2p", "\nxyz\nlmn");
    DoTest("123\n456", "dd\"add\"1p\"2p", "\n456\n123");
    DoTest("9\n8\n7\n6\n5\n4\n3\n2\n1\n0", "\"1dddddddddddddddd2dd\"1p\"2p\"3p\"4p\"5p\"6p\"7p\"8p\"9p", "\n1\n2\n3\n4\n5\n6\n7\n8\n9\n0");

    // Testing "D"
    DoTest("foo bar", "lllD", "foo");
    DoTest("foo\nfoo2\nfoo3", "l2D", "f\nfoo3");
    DoTest("qwerty", "d frDai", "wei");
    // Testing small delete (-) register
    DoTest("12345\n67890", "ddD\"-p", "67890");
    DoTest("foo\nbar\nhello world", "ld$dj$\"-p", "hello worldoo");
    DoTest("123\n456", "Dj\"aD\"-p", "\n123");
    DoTest("abc\nlmn\nxyz", "yyjlYjxk\"-p", "abc\nlmyn\nxz");

    // Testing "d"
    DoTest("foobar", "ld2l", "fbar");
    DoTest("1 2 3\n4 5 6", "ld100l", "1\n4 5 6");

    DoTest("123\n", "d10l", "\n");
    DoTest("123\n", "10lx", "12\n");

    // Testing "Y"
    DoTest("qwerty", "ld Yep", "qertyerty");
    // Testing ""<x>y" where x is in a-z (overwrite/insert to register)
    DoTest("x", "\"ayh\"Ayl\"ap", "xx");
    DoTest("xy", "\"byll\"byll\"bp", "xyy");
    DoTest("xyz", "\"cy3l$\"cp", "xyzxyz");
    DoTest("abz", "\"xyll\"yyll\"zyll\"zp\"xp\"yp", "abzzab");
    DoTest("abcdwxyz", "\"byh\"ayll\"Byll\"cyll\"dyll\"dyl\"zye$\"zp\"ap\"Cp\"bp\"dp", "abcdwxyzwxyzacbw");
    // Testing "<X>y" where X is in A-Z (append-copy)
    DoTest("foo bar ", "\"ayew\"Aye$\"ap", "foo bar foobar");
    DoTest("foo bar 2", "\"ayew\"Ayew\"aye\"ap", "foo bar 22");
    DoTest("123 foo ", "\"ayew\"zye\"Aye0\"Zy4l$\"Zp\"ap", "123 foo foo123 123foo");

    // Testing "X"
    DoTest("ABCD", "$XX", "AD");
    DoTest("foo", "XP", "foo");

    // Testing Del key
    DoTest("foo", "\\home\\delete", "oo");
    DoTest("foo", "$\\delete", "fo");

    // Delete. Note that when sent properly via Qt, the key event text() will inexplicably be "127",
    // which can trip up the key parser. Duplicate this oddity here.
    BeginTest(QStringLiteral("xyz"));
    TestPressKey(QStringLiteral("l"));
    QKeyEvent *deleteKeyDown = new QKeyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier, QStringLiteral("127"));
    QApplication::postEvent(kate_view->focusProxy(), deleteKeyDown);
    QApplication::sendPostedEvents();
    QKeyEvent *deleteKeyUp = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Delete, Qt::NoModifier, QStringLiteral("127"));
    QApplication::postEvent(kate_view->focusProxy(), deleteKeyUp);
    QApplication::sendPostedEvents();
    FinishTest("xz");

    // Testing "gu"
    DoTest("FOO\nBAR BAZ", "guj", "foo\nbar baz");
    DoTest("AbCDF", "gu3l", "abcDF");

    // Testing "guu"
    DoTest("FOO", "guu", "foo");
    DoTest("FOO\nBAR\nBAZ", "2guu", "foo\nbar\nBAZ");
    DoTest("", "guu", "");

    // Testing "gU"
    DoTest("aBcdf", "gU2l", "ABcdf");
    DoTest("foo\nbar baz", "gUj", "FOO\nBAR BAZ");

    // Testing "gUU"
    DoTest("foo", "gUU", "FOO");
    DoTest("foo\nbar\nbaz", "2gUU", "FOO\nBAR\nbaz");
    DoTest("", "gUU", "");

    // Testing "g~"
    DoTest("fOo BAr", "lg~fA", "foO bar");
    DoTest("fOo BAr", "$hg~FO", "foO bAr");
    DoTest("fOo BAr", "lf~fZ", "fOo BAr");
    DoTest("{\nfOo BAr\n}", "jg~iB", "{\nFoO baR\n}");

    // Testing "g~~"
    DoTest("", "g~~", "");
    DoTest("\nfOo\nbAr", "g~~", "\nfOo\nbAr");
    DoTest("fOo\nbAr\nBaz", "g~~", "FoO\nbAr\nBaz");
    DoTest("fOo\nbAr\nBaz\nfAR", "j2g~~", "fOo\nBaR\nbAZ\nfAR");
    DoTest("fOo\nbAr\nBaz", "jlg~~rX", "fOo\nXaR\nBaz");
    DoTest("fOo\nbAr\nBaz\nfAR", "jl2g~~rX", "fOo\nBXR\nbAZ\nfAR");

    // Testing "s"
    DoTest("substitute char repeat", "w4scheck\\esc", "substitute check repeat");

    // Testing "r".
    DoTest("foobar", "l2r.", "f..bar");
    DoTest("foobar", "l5r.", "f.....");
    // Do nothing if the count is too high.
    DoTest("foobar", "l6r.", "foobar");

    // Testing "Ctrl-o" and "Ctrl-i"
    DoTest("abc\ndef\nghi", "Gx\\ctrl-ox", "bc\ndef\nhi");
    DoTest("{\n}", "%\\ctrl-ox", "\n}");
    DoTest("Foo foo.\nBar bar.\nBaz baz.", "lmajlmb`a`b\\ctrl-ox", "Fo foo.\nBar bar.\nBaz baz.");
    DoTest("Foo foo.\nBar bar.\nBaz baz.", "lmajlmb`a`bj\\ctrl-o\\ctrl-ix", "Foo foo.\nBar bar.\nBa baz.");

    // Testing "gq" (reformat) text
    DoTest("foo\nbar", "gqq", "foo\nbar");
    DoTest("foo\nbar", "2gqq", "foo bar");
    DoTest("foo\nbar\nbaz", "jgqj", "foo\nbar baz");
    DoTest("foo\nbar\n\n\n", "gqap", "foo bar\n\n\n");

    // when setting the text to wrap at column 10, this should be re-formatted to
    // span several lines ...
    kate_document->setWordWrapAt(10);
    DoTest("foo bar foo bar foo bar", "gqq", "foo bar \nfoo bar \nfoo bar");

    // ... and when re-setting it to column 80 again, they should be joined again
    kate_document->setWordWrapAt(80);
    DoTest("foo bar\nfoo bar\nfoo bar", "gqG", "foo bar foo bar foo bar");

    // test >> and << (indent and de-indent)
    kate_document->config()->setReplaceTabsDyn(true);

    DoTest("foo\nbar", ">>", "  foo\nbar");
    DoTest("foo\nbar", "2>>", "  foo\n  bar");
    DoTest("foo\nbar", "100>>", "  foo\n  bar");

    DoTest("fop\nbar", "yiwjlgpx", "fop\nbafop");
    DoTest("fop\nbar", "yiwjlgPx", "fop\nbfopr");

    DoTest("repeat\nindent", "2>>2>>", "    repeat\n    indent");

    // make sure we record correct history when indenting
    DoTest("repeat\nindent and undo", "2>>2>>2>>uu", "  repeat\n  indent and undo");
    DoTest("repeat\nunindent and undo", "2>>2>>2<<u", "    repeat\n    unindent and undo");

    // Yank and paste op\ngid into bar i.e. text spanning lines, but not linewise.
    DoTest("fop\ngid\nbar", "lvjyjjgpx", "fop\ngid\nbaop\ngi");
    DoTest("fop\ngid\nbar", "lvjyjjgPx", "fop\ngid\nbop\ngir");
    DoTest("fop\ngid\nbar", "lvjyjjpx", "fop\ngid\nbap\ngir");
    DoTest("fop\ngid\nbar", "lvjyjjPx", "fop\ngid\nbp\ngiar");
    // Linewise
    DoTest("fop\ngid\nbar\nhuv", "yjjjgpx", "fop\ngid\nbar\nfop\ngid\nuv");
    DoTest("fop\ngid\nbar\nhuv", "yjjjgPx", "fop\ngid\nfop\ngid\nar\nhuv");
    DoTest("fop\ngid", "yjjgpx", "fop\ngid\nfop\nid");
    DoTest("fop\ngid\nbar\nhuv", "yjjjPx", "fop\ngid\nop\ngid\nbar\nhuv");

    DoTest("fop\nbar", "yiwjlpx", "fop\nbafor");
    DoTest("fop\nbar", "yiwjlPx", "fop\nbfoar");

    // Indented paste.
    // ]p behaves as ordinary paste if not linewise, and on unindented line.
    DoTest("foo bar", "wyiwgg]p", "fbaroo bar");
    // ]p behaves as ordinary paste if not linewise, even on indented line.
    DoTest("  foo bar", "wwyiwggw]p", "  fbaroo bar");
    // [p behaves as ordinary Paste (P) if not linewise, and on unindented line.
    DoTest("foo bar", "wyiwgg[p", "barfoo bar");
    // [p behaves as ordinary Paste (P) if not linewise, even on indented line.
    DoTest("  foo bar", "wwyiw0w[p", "  barfoo bar");
    // Prepend the spaces from the current line to the beginning of a single, pasted line.
    DoTest("  foo bar\nxyz", "jVygg]p", "  foo bar\n  xyz\nxyz");
    // Prepend the spaces from the current line to the beginning of each pasted line.
    DoTest("  foo bar\nxyz\nnose", "jVjygg]p", "  foo bar\n  xyz\n  nose\nxyz\nnose");
    const bool oldReplaceTabsDyn = kate_document->config()->replaceTabsDyn();
    kate_document->config()->setReplaceTabsDyn(false);
    // Tabs as well as spaces!
    DoTest("  \tfoo bar\nxyz\nnose", "jVjygg]p", "  \tfoo bar\n  \txyz\n  \tnose\nxyz\nnose");
    // Same for [p.
    DoTest("  \tfoo bar\nxyz\nnose", "jVjygg[p", "  \txyz\n  \tnose\n  \tfoo bar\nxyz\nnose");
    // Test if everything works if the current line has no non-whitespace.
    DoTest("\t \nbar", "jVygg]p", "\t \n\t bar\nbar");
    // Test if everything works if the current line is empty.
    DoTest("\nbar", "jVygg]p", "\nbar\nbar");
    // Unindent a pasted indented line if the current line has no indent.
    DoTest("foo\n  \tbar", "jVygg]p", "foo\nbar\n  \tbar");
    // Unindent subsequent lines, too - TODO - this assumes that each subsequent line has
    // *identical* trailing whitespace to the first pasted line: Vim seems to be able to
    // deal with cases where this does not hold.
    DoTest("foo\n  \tbar\n  \txyz", "jVjygg]p", "foo\nbar\nxyz\n  \tbar\n  \txyz");
    DoTest("foo\n  \tbar\n  \t  xyz", "jVjygg]p", "foo\nbar\n  xyz\n  \tbar\n  \t  xyz");
    kate_document->config()->setReplaceTabsDyn(oldReplaceTabsDyn);

    // Some special cases of cw/ cW.
    DoTest("foo bar", "cwxyz\\esc", "xyz bar");
    DoTest("foo+baz bar", "cWxyz\\esc", "xyz bar");
    DoTest("foo+baz bar", "cwxyz\\esc", "xyz+baz bar");
    DoTest(" foo bar", "cwxyz\\esc", "xyzfoo bar");
    DoTest(" foo+baz bar", "cWxyz\\esc", "xyzfoo+baz bar");
    DoTest(" foo+baz bar", "cwxyz\\esc", "xyzfoo+baz bar");
    DoTest("\\foo bar", "cWxyz\\esc", "xyz bar");
    DoTest("foo   ", "lllcwxyz\\esc", "fooxyz");

    DoTest("foo", "yr", "foo");
    QCOMPARE(kate_view->renderer()->caretStyle(), KTextEditor::caretStyles::Block);

    // BUG #332523
    const bool oldDynWordWrap = KateViewConfig::global()->dynWordWrap();
    BeginTest(QStringLiteral("asdasdasd\nasdasdasdasdasdasdasd"));
    kate_document->setWordWrap(true);
    kate_document->setWordWrapAt(10);
    TestPressKey(QStringLiteral("Jii"));
    FinishTest("iasdasdasd\n \nasdasdasda \nsdasdasdas \nd");
    kate_document->setWordWrap(oldDynWordWrap);
}

void ModesTest::NormalControlTests()
{
    // Testing "Ctrl-x"
    DoTest("150", "101\\ctrl-x", "49");
    DoTest("1", "\\ctrl-x\\ctrl-x\\ctrl-x\\ctrl-x", "-3");
    DoTest("0xabcdef", "1000000\\ctrl-x", "0x9c8baf");
    DoTest("0x0000f", "\\ctrl-x", "0x0000e");
    // Octal numbers should retain leading 0's.
    DoTest("00010", "\\ctrl-x", "00007");

    // Testing "Ctrl-a"
    DoTest("150", "101\\ctrl-a", "251");
    DoTest("1000", "\\ctrl-ax", "100");
    DoTest("-1", "1\\ctrl-a", "0");
    DoTest("-1", "l1\\ctrl-a", "0");
    DoTest("0x0000f", "\\ctrl-a", "0x00010");
    // Decimal with leading 0's - increment, and strip leading 0's, like Vim.
    DoTest("0000193", "\\ctrl-a", "194");
    // If a number begins with 0, parse it as octal if we can. The resulting number should retain the
    // leadingi 0.
    DoTest("07", "\\ctrl-a", "010");
    DoTest("5", "5\\ctrl-a.", "15");
    DoTest("5", "5\\ctrl-a2.", "12");
    DoTest("5", "5\\ctrl-a2.10\\ctrl-a", "22");
    DoTest(" 5 ", "l\\ctrl-ax", "  ");
    // If there's no parseable number under the cursor, look to the right to see if we can find one.
    DoTest("aaaa0xbcX", "\\ctrl-a", "aaaa0xbdX");
    DoTest("1 1", "l\\ctrl-a", "1 2");
    // We can skip across word boundaries in our search if need be.
    DoTest("aaaa 0xbcX", "\\ctrl-a", "aaaa 0xbdX");
    // If we can't find a parseable number anywhere, don't change anything.
    DoTest("foo", "\\ctrl-a", "foo");
    // Don't hang if the cursor is at the end of the line and the only number is to the immediate left of the cursor.
    DoTest("1 ", "l\\ctrl-a", "1 ");
    // ctrl-a/x algorithm involves stepping back to the previous word: don't crash if this is on the previous line
    // and at a column greater than the length of the current line.
    DoTest(" a a\n1", "j\\ctrl-a", " a a\n2");
    DoTest(" a a    a\n  1", "jll\\ctrl-a", " a a    a\n  2");
    // Regression test.
    DoTest("1w3", "l\\ctrl-a", "1w4");

    // Test "Ctrl-a/x" on a blank document/ blank line.
    DoTest("", "\\ctrl-a", "");
    DoTest("", "\\ctrl-x", "");
    DoTest("foo\n", "j\\ctrl-x", "foo\n");
    DoTest("foo\n", "j\\ctrl-a", "foo\n");

    // Testing "Ctrl-r"
    DoTest("foobar", "d3lu\\ctrl-r", "bar");
    DoTest("line 1\nline 2\n", "ddu\\ctrl-r", "line 2\n");
}

void ModesTest::NormalNotYetImplementedFeaturesTests()
{
    QSKIP("This tests never worked :(", SkipAll);

    // Testing "))"
    DoTest("Foo foo. Bar bar.", "))\\ctrl-ox", "Foo foo. ar bar.");
    DoTest("Foo foo.\nBar bar.\nBaz baz.", ")))\\ctrl-ox\\ctrl-ox", "Foo foo.\nar bar.\nBaz baz.");
    DoTest("Foo foo.\nBar bar.\nBaz baz.", "))\\ctrl-ox\\ctrl-ix", "Foo foo.\nBar bar.\naz baz.");
    DoTest("Foo foo.\nBar bar.\nBaz baz.", "))\\ctrl-ox\\ctrl-ix", "Foo foo.\nBar bar.\naz baz.");
}

// END: Normal mode.

// BEGIN: Insert mode.

void ModesTest::InsertTests()
{
    // Basic stuff.
    DoTest("bar", "s\\ctrl-c", "ar");
    DoTest("bar", "ls\\ctrl-cx", "r");
    DoTest("foo\nbar", "S\\ctrl-c", "\nbar");
    DoTest("baz bar", "lA\\ctrl-cx", "baz ba");
    DoTest("baz bar", "la\\ctrl-cx", "bz bar");
    DoTest("foo\nbar\nbaz", "C\\ctrl-c", "\nbar\nbaz");
    DoTest("foo bar baz", "c2w\\ctrl-c", " baz");
    DoTest("foo\nbar\nbaz", "jo\\ctrl-c", "foo\nbar\n\nbaz");
    DoTest("foo\nbar\nbaz", "jO\\ctrl-c", "foo\n\nbar\nbaz");
    DoTest("foo\nbar", "O\\ctrl-c", "\nfoo\nbar");
    DoTest("foo\nbar", "o\\ctrl-c", "foo\n\nbar");
    DoTest("foo bar", "wlI\\ctrl-cx", "oo bar");
    DoTest("foo bar", "wli\\ctrl-cx", "foo ar");
    DoTest("foo bar", "wlihello\\ctrl-c", "foo bhelloar");

    // With count.
    DoTest("", "5ihello\\esc", "hellohellohellohellohello");
    DoTest("bar", "5ahello\\esc", "bhellohellohellohellohelloar");
    DoTest("   bar", "5Ihello\\esc", "   hellohellohellohellohellobar");
    DoTest("bar", "5Ahello\\esc", "barhellohellohellohellohello");
    DoTest("", "5ihello\\ctrl-c", "hello");
    DoTest("bar", "5ohello\\esc", "bar\nhello\nhello\nhello\nhello\nhello");
    DoTest("bar", "5Ohello\\esc", "hello\nhello\nhello\nhello\nhello\nbar");
    DoTest("bar", "Ohello\\escu", "bar");
    DoTest("bar", "5Ohello\\escu", "bar");
    DoTest("bar", "ohello\\escu", "bar");
    DoTest("bar", "5ohello\\escu", "bar");
    DoTest("foo\nbar", "j5Ohello\\esc", "foo\nhello\nhello\nhello\nhello\nhello\nbar");
    DoTest("bar", "5ohello\\esc2ixyz\\esc", "bar\nhello\nhello\nhello\nhello\nhellxyzxyzo");
    DoTest("", "ihello\\esc5.", "hellhellohellohellohellohelloo");

    // Ensure that the flag that says that counted repeats should begin on a new line is reset.
    DoTest("foo", "obar\\ctrl-c5ixyz\\esc", "foo\nbaxyzxyzxyzxyzxyzr");
    DoTest("foo", "obar\\ctrl-cgg\\ctrl-vlljAxyz\\esc5i123\\esc", "fooxy123123123123123z\nbarxyz");
    DoTest("foo foo foo", "c3wbar\\esc", "bar");
    DoTest("abc", "lOxyz", "xyz\nabc");

    // Test that our test driver can handle newlines during insert mode :)
    DoTest("", "ia\\returnb", "a\nb");
}

void ModesTest::InsertKeysTests()
{
    // Ctrl-w
    DoTest("foobar", "$i\\ctrl-w", "r");
    DoTest("foobar\n", "A\\ctrl-w", "\n");
    DoTest("   foo", "i\\ctrl-wX\\esc", "X   foo");
    DoTest("   foo", "lli\\ctrl-wX\\esc", "X foo");

    // Ctrl-u
    DoTest("", "i\\ctrl-u", "");
    DoTest("foobar", "i\\ctrl-u", "foobar");
    DoTest("foobar", "fbi\\ctrl-u", "bar");
    DoTest("foobar\nsecond", "ji\\ctrl-u", "foobarsecond");
    DoTest("foobar\n  second", "jwi\\ctrl-u", "foobar\nsecond");
    DoTest("foobar\n  second", "jfci\\ctrl-u", "foobar\n  cond");
    DoTest("foobar\n  second", "j$a\\ctrl-u", "foobar\n  ");

    // Ctrl-e
    DoTest("foo\nbar", "i\\ctrl-e", "bfoo\nbar");
    DoTest("foo\nbar", "i\\ctrl-e\\ctrl-e\\ctrl-e", "barfoo\nbar");
    DoTest("foo\nb", "i\\ctrl-e\\ctrl-e", "bfoo\nb");

    // Ctrl-y
    DoTest("foo\nbar", "ji\\ctrl-y", "foo\nfbar");
    DoTest("foo\nbar", "ji\\ctrl-y\\ctrl-y\\ctrl-y", "foo\nfoobar");
    DoTest("f\nbar", "ji\\ctrl-y\\ctrl-y", "f\nfbar");

    // Ctrl-R
    DoTest("barbaz", "\"ay3li\\ctrl-ra", "barbarbaz");
    DoTest("barbaz", "\"ay3li\\ctrl-raX", "barXbarbaz");
    DoTest("bar\nbaz", "\"byylli\\ctrl-rb", "bar\nbar\nbaz");
    DoTest("Hello", "0yei\\ctrl-r\"", "HelloHello");

    // Ctrl-O
    DoTest("foo bar baz", "3li\\ctrl-od2w", "foobaz");
    DoTest("foo bar baz", "3li\\ctrl-od2w\\ctrl-w", "baz");
    DoTest("foo bar baz", "i\\ctrl-o3l\\ctrl-w", " bar baz");
    DoTest("foo\nbar\nbaz", "li\\ctrl-oj\\ctrl-w\\ctrl-oj\\ctrl-w", "foo\nar\naz");

    // Test that the text written after the Ctrl-O command completes is treated as
    // an insertion of text (rather than a sequence of commands) when repeated via "."
    DoTest("", "isausage\\ctrl-obugo\\esc.", "ugugoosausage");

    // 'Step back' on Ctrl-O if at the end of the line
    DoTest("foo bar baz", "A\\ctrl-ox", "foo bar ba");

    // Paste acts as gp when executing in a Ctrl-O
    DoTest("foo bar baz", "yiwea\\ctrl-opd", "foo foodbar baz");
    DoTest("bar", "A\\ctrl-o\\ctrl-chx", "br");
    DoTest("bar", "A\\ctrl-o\\eschx", "br");

    // Ctrl-D & Ctrl-T
    DoTest("foo", "i\\ctrl-t", "  foo");
    DoTest(" foo", "i\\ctrl-d", "foo");
    DoTest("foo\nbar", "i\\ctrl-t\\ctrl-d", "foo\nbar");

    // Ctrl-H
    DoTest("foo", "i\\ctrl-h", "foo");
    DoTest(" foo", "li\\ctrl-h", "foo");
    DoTest("foo\nbar", "ji\\ctrl-h", "foobar");
    DoTest("1234567890", "A\\ctrl-h\\ctrl-h\\ctrl-h\\ctrl-h\\ctrl-h", "12345");
    DoTest("1\n2\n3", "GA\\ctrl-h\\ctrl-h\\ctrl-h\\ctrl-h", "1");

    // Ctrl-J
    DoTest("foo", "i\\ctrl-j", "\nfoo");
    DoTest("foo", "lli\\ctrl-j", "fo\no");
    DoTest("foo\nbar", "ji\\ctrl-j", "foo\n\nbar");
    DoTest("foobar", "A\\ctrl-j", "foobar\n");
    DoTest("foobar", "li\\ctrl-j\\ctrl-cli\\ctrl-j\\ctrl-cli\\ctrl-j\\ctrl-cli\\ctrl-j\\ctrl-cli\\ctrl-j\\ctrl-c", "f\no\no\nb\na\nr");

    // Ctrl-left & Ctrl-right.
    DoTest("foo bar", "i\\ctrl-\\rightX\\esc", "foo Xbar");
    DoTest("foo bar", "i\\ctrl-\\right\\ctrl-\\rightX\\esc", "foo barX");
    DoTest("foo", "\\endi\\ctrl-\\left\\ctrl-\\leftX", "Xfoo"); // we crashed here before

    // Special keys: enter, return, insert, etc.
    DoTest("", "ifoo\\enterbar", "foo\nbar");
    DoTest("", "ifoo\\returnbar", "foo\nbar");
    DoTest("", "\\insertfoo", "foo");
    DoTest("foo bar", "i\\home\\delete", "oo bar");

    // BUG #451076, don't want an extra newline when using the "Find" action from the menu or Ctrl-F
    BaseTest::ensureKateViewVisible(); // required for the bug to trigger
    BeginTest(QStringLiteral("foobar"));
    TestPressKey(QStringLiteral("i"));
    kate_view->find();
    TestPressKey(QStringLiteral("bar\\return"));
    // ensure the cursor is at 'foo_b_ar'
    QCOMPARE(kate_view->cursorPosition().line(), 0);
    QCOMPARE(kate_view->cursorPosition().column(), 3);
    FinishTest("foobar");
    // hide the view again
    kate_view->hide();
    mainWindow->hide();
}

// END: Insert mode.

// BEGIN: Visual mode.

void ModesTest::VisualMotionsTests()
{
    // Basic motions.
    DoTest("\n", "vjcX", "X");
    DoTest("foobar", "vlllx", "ar");
    DoTest("foo\nbar", "Vd", "bar");
    DoTest("Hello.\nWorld", "2lvjcX", "HeXld");
    DoTest("Three. Different. Sentences.\n\n", "vapcX", "X");
    DoTest("1234\n1234\n1234", "l\\ctrl-vljjd", "14\n14\n14");
    QCOMPARE(kate_view->blockSelection(), false);
    DoTest("Three. Different. Sentences.", "v)cX", "Xifferent. Sentences.");
    DoTest("Three. Different. Sentences.", "v)cX", "Xifferent. Sentences.");
    DoTest("Three. Different. Sentences.", "v)cX", "Xifferent. Sentences.");
    DoTest("Three. Different. Sentences.", "viWcX", "X Different. Sentences.");
    DoTest("Three. Different. Sentences.", "viwcX", "X. Different. Sentences.");
    DoTest("Three. Different. Sentences.", "vaWcX", "XDifferent. Sentences.");
    DoTest("Three. Different. Sentences.", "vawcX", "X. Different. Sentences.");
    DoTest("Three. Different. Sentences.", "vascX", "XDifferent. Sentences.");
    DoTest("Three. Different. Sentences.", "viscX", "X Different. Sentences.");
    DoTest("Three. Different. Sentences.", "vapcX", "X");
    DoTest("Three. Different. Sentences.", "vipcX", "X");
    DoTest("Hello.\n", "vap\\esciX", "Hello.\nX");
    // Make sure cursor is properly moved to the start of paragraph and sentence text objects
    DoTest("Hello.\nWorld.", "jvapX", "");
    DoTest("Hello.\nWorld.", "jvasX", "Hello.\n");

    // With count.
    DoTest("12345678", "lv3lyx", "1345678");
    DoTest("12345678", "$hv3hyx", "1235678");
    DoTest("aaa\nbbb", "lvj~x", "aA\nBBb");
    DoTest("123\n456", "jlvkyx", "13\n456");
    DoTest("12\n34", "lVjyx", "2\n34");
    DoTest("ab\ncd", "jVlkgux", "a\ncd");
    DoTest("ABCD\nABCD\nABCD\nABCD", "lj\\ctrl-vjlgux", "ABCD\nAcD\nAbcD\nABCD");
    DoTest("abcd\nabcd\nabcd\nabcd", "jjjlll\\ctrl-vkkhgUx", "abcd\nabD\nabCD\nabCD");

    // Cancelling visual mode should not reset the cursor.
    DoTest("12345678", "lv3l\\escx", "1234678");
    DoTest("12345678", "lv3l\\ctrl-cx", "1234678");

    // Don't forget to clear the flag that says we shouldn't reset the cursor, though!
    DoTest("12345678", "lv3l\\ctrl-cxv3lyx", "123478");
    DoTest("12345678", "y\\escv3lyx", "2345678");

    // Regression test for ][ in Visual Mode.
    DoTest("foo {\n\n}", "lV][d", "");

    // Misc tests for motions starting in front of the Visual Mode start point.
    DoTest("{foo}", "lvb%x", "{");
    DoTest("foo bar", "wvbfax", "foo r");
    DoTest("(foo bar)", "wwv^%x", "(foo ");

    // * and #
    DoTest("foo foo", "v*x", "oo");
    DoTest("foo foo", "wv#x", "oo");

    // Quick test that "{" and "}" motions work in visual mode
    DoTest("foo\n\n\nbar\n", "v}}d", "");
    DoTest("\n\nfoo\nbar\n", "jjjv{d", "\nar\n");

    // ctrl-left and ctrl-right
    DoTest("foo bar xyz", "v\\ctrl-\\rightd", "ar xyz");
    DoTest("foo bar xyz", "$v\\ctrl-\\leftd", "foo bar ");
}

void ModesTest::VisualCommandsTests()
{
    // Testing "d"
    DoTest("foobarbaz", "lvlkkjl2ld", "fbaz");
    DoTest("foobar", "v$d", "");
    DoTest("foo\nbar\nbaz", "jVlld", "foo\nbaz");
    DoTest("01\n02\n03\n04\n05", "Vjdj.", "03");

    // Testing Del key
    DoTest("foobarbaz", "lvlkkjl2l\\delete", "fbaz");

    // Testing "D"
    DoTest("foo\nbar\nbaz", "lvjlD", "baz");
    DoTest("foo\nbar", "l\\ctrl-vjD", "f\nb");
    DoTest("foo\nbar", "VjkD", "bar");
    DoTest("Test:\n  - One\n  - Two", "jfnVDia", "Test:\n  a- Two");
    DoTest("Test:\n  - One\n  - Two", "jjfwVDia", "Test:\n  a- One");

    // Testing "gU", "U"
    DoTest("foo bar", "vwgU", "FOO Bar");
    DoTest("foo\nbar\nbaz", "VjjU", "FOO\nBAR\nBAZ");
    DoTest("foo\nbar\nbaz", "\\ctrl-vljjU", "FOo\nBAr\nBAz");
    DoTest("aaaa\nbbbb\ncccc", "\\ctrl-vljgUjll.", "AAaa\nBBBB\nccCC");

    // Testing "gu", "u"
    DoTest("TEST", "Vgu", "test");
    DoTest("TeSt", "vlgu", "teSt");
    DoTest("FOO\nBAR\nBAZ", "\\ctrl-vljju", "foO\nbaR\nbaZ");
    DoTest("AAAA\nBBBB\nCCCC\nDDDD", "vjlujjl.", "aaaa\nbbBB\nCccc\ndddD");

    // Testing "gv"
    DoTest("foo\nbar\nxyz", "l\\ctrl-vjj\\ctrl-cgvr.", "f.o\nb.r\nx.z");

    // Testing "g~"
    DoTest("fOo bAr", "Vg~", "FoO BaR");
    DoTest("foo\nbAr\nxyz", "l\\ctrl-vjjg~", "fOo\nbar\nxYz");

    // Testing "y"
    DoTest("foobar", "Vypp", "foobar\nfoobar\nfoobar");
    DoTest("foo\nbar", "lvjlyp", "fooo\nbaro\nbar");
    DoTest("foo\nbar", "Vjlllypddxxxdd", "foo\nbar");
    DoTest("12\n12", "\\ctrl-vjyp", "112\n112");
    DoTest("1234\n1234\n1234\n1234", "lj\\ctrl-vljyp", "1234\n122334\n122334\n1234");

    // Testing "Y"
    DoTest("foo\nbar", "llvjypx", "foo\nbar\nbar");
    DoTest("foo\nbar", "VYp", "foo\nfoo\nbar");

    // Testing "m."
    DoTest("foo\nbar", "vljmavgg`ax", "foo\nbr");
    DoTest("1\n2\n3\n4", "Vjmajjmb\\:'a,'bd\\", "1");

    // Testing ">"
    DoTest("foo\nbar", "vj>", "  foo\n  bar");
    DoTest("foo\nbar\nbaz", "jVj>", "foo\n  bar\n  baz");
    DoTest("foo", "vl3>", "      foo");
    DoTest("indent\nrepeat", "V>.", "    indent\nrepeat");
    DoTest("indent\nrepeat", "Vj>.", "    indent\n    repeat");
    DoTest("indent\nrepeat\non\nothers", "Vj>jj.", "  indent\n  repeat\n  on\n  others");
    DoTest("foo\nbar\nbaz", "jjVk>.", "foo\n    bar\n    baz");

    // Testing "<"
    DoTest(" foo", "vl<", "foo");
    DoTest("foo\n    bar\n    baz", "jjVk<.", "foo\nbar\nbaz");

    // Testing "o"
    DoTest("foobar", "lv2lo2ld", "fooar");
    DoTest("foo\nbar", "jvllokld", "f");
    DoTest("12\n12", "\\ctrl-vjlold", "1\n1");

    // Testing "~"
    DoTest("foobar", "lv2l~", "fOOBar");
    DoTest("FooBar", "V~", "fOObAR");
    DoTest("foo\nbar", "\\ctrl-vjl~", "FOo\nBAr");

    // Testing "r"
    DoTest("foobar", "Vra", "aaaaaa");
    DoTest("foo\nbar", "jlvklrx", "fox\nxxr");
    DoTest("123\n123", "l\\ctrl-vljrx", "1xx\n1xx");
    DoTest("a", "r\\ctrl-c", "a");
    DoTest("a", "r\\ctrl-[", "a");
    DoTest("a", "r\\keypad-0", "0");
    DoTest("a", "r\\keypad-9", "9");
    DoTest("foo\nbar", "l\\ctrl-vjr\\keypad-9", "f9o\nb9r");

    // Testing "gq"
    DoTest("foo\nbar\nbaz", "Vgq", "foo\nbar\nbaz");
    DoTest("foo\nbar\nbaz", "Vjgq", "foo bar\nbaz");

    // Testing "<<"/">>"
    kate_document->config()->setReplaceTabsDyn(true);
    DoTest("foo\nbar\nbaz", "V>>", "  foo\nbar\nbaz");
    DoTest("foo\nbar\nbaz", "Vj>>", "  foo\n  bar\nbaz");
    DoTest("foo\nbar\nbaz", "V2j>>", "  foo\n  bar\n  baz");
    DoTest("foo\nbar\nbaz", "V10>>", "                    foo\nbar\nbaz");
    DoTest("foo\nbar\nbaz", "V2j3>>", "      foo\n      bar\n      baz");

    DoTest("  foo\nbar\nbaz", "V<<", "foo\nbar\nbaz");
    DoTest("foo\nbar\nbaz", "V>>V<<", "foo\nbar\nbaz");
    DoTest("    foo\n    bar\n    baz", "V2j<<", "  foo\n  bar\n  baz");

    // Testing block append
    DoTest("averyverylongline\nshortline\nshorter\n", "jjV$kkAb\\esc", "averyverylonglineb\nshortlineb\nshorterb\n");
    DoTest("averyverylongline\nshortline\n", "V$jAb\\esc", "averyverylonglineb\nshortlineb\n");

    // Testing "J"
    DoTest("foo\nbar\nxyz\nbaz\n123\n456", "jVjjjJ", "foo\nbar xyz baz 123\n456");
    DoTest("foo\nbar\nxyz\nbaz\n123\n456", "jjjjVkkkJ", "foo\nbar xyz baz 123\n456");
    DoTest("foo\nbar\nxyz\nbaz\n123456\n789", "jjjjVkkkJrX", "foo\nbar xyz bazX123456\n789");
    DoTest("foo\nbar\nxyz\n", "VGJ", "foo bar xyz ");

    // Testing undo behaviour with c and cc
    DoTest("foo", "ciwbar\\escu", "foo");
    DoTest("foo", "ccbar\\escu", "foo");

    // Pasting should replace the current selection.
    DoTest("foo bar xyz", "yiwwviwp", "foo foo xyz");

    // Undo should undo both paste and removal of selection.
    DoTest("foo bar xyz", "yiwwviwpu", "foo bar xyz");
    DoTest("foo\nbar\n123\nxyz", "yiwjVjp", "foo\nfoo\nxyz");

    // Set the *whole* selection to the given text object, even if the cursor is no
    // longer at the position where Visual Mode was started.
    // This seems to work (in Vim) only when the start of the given text object occurs before them
    // start position of Visual Mode.
    DoTest("{\nfoo\nbar\nxyz\n}", "jjvliBd", "{\n}");
    DoTest("foo[hello]", "fhlvli[d", "foo[]");
    DoTest("foo(hello)", "fhlvli(d", "foo()");
    DoTest("foo<hello>", "fhlvli<d", "foo<>");
    DoTest("foo\"hello\"", "fhlvli\"d", "foo\"\"");
    DoTest("foo'hello'", "fhlvli'd", "foo''");

    // A couple of spot tests, where the beginning of the text object occurs after the start position of Visual Mode;
    // the selection should  remain unchanged if we the text object motion is triggered, here.
    DoTest("foobarxyz\n(12345)", "llvjibd", "fo345)");
    DoTest("foobarxyz\n{12345}", "llvjiBd", "fo345}");
    // Cursor should end up at the end of the text object.
    DoTest("foo[hello]", "fhlvli[\\escrX", "foo[hellX]");
    // Ensure we reset the flag that says that the current motion is a text object!
    DoTest("foo[hello]", "jfhlvli[^d", "ello]");

    // proper yanking in block mode
    {
        BeginTest(QStringLiteral("aaaa\nbbbb\ncccc\ndddd"));
        TestPressKey(QStringLiteral("lj\\ctrl-vljy"));
        KateBuffer &buffer = kate_document->buffer();
        QList<Kate::TextRange *> ranges = buffer.rangesForLine(1, kate_view, true);
        QCOMPARE(ranges.size(), 1);
        KTextEditor::Range range = ranges[0]->toRange();
        QCOMPARE(range.start().column(), 1);
        QCOMPARE(range.end().column(), 3);
    }

    // proper selection in block mode after switching to cmdline
    {
        BeginTest(QStringLiteral("aaaa\nbbbb\ncccc\ndddd"));
        TestPressKey(QStringLiteral("lj\\ctrl-vlj:"));
        QCOMPARE(kate_view->selectionText(), QStringLiteral("bb\ncc"));
    }

    // BUG #328277 - make sure kate doesn't crash
    BeginTest(QStringLiteral("aaa\nbbb"));
    TestPressKey(QStringLiteral("Vj>u>."));
    QCOMPARE(kate_view->renderer()->caretStyle(), KTextEditor::caretStyles::Block);
    FinishTest("aaa\nbbb");
}

void ModesTest::VisualExternalTests()
{
    // Test that selecting a range "externally" to Vim (i.e. via the mouse, or
    // one of the ktexteditor api's) switches us into Visual Mode.
    BeginTest(QStringLiteral("foo bar"));

    // Actually selects "oo " (i.e. without the "b").
    kate_view->setSelection(Range(0, 1, 0, 4));
    TestPressKey(QStringLiteral("d"));
    FinishTest("fbar");

    // Always return to normal mode when undoing/redoing.
    BeginTest(QLatin1String(""));
    TestPressKey(QStringLiteral("iHello World!\\esc"));
    TestPressKey(QStringLiteral("0wvlldu"));
    QCOMPARE(vi_input_mode_manager->getCurrentViMode(), KateVi::NormalMode);
    QCOMPARE(kate_view->selectionText(), QLatin1String(""));
    QCOMPARE(kate_document->text(), QStringLiteral("Hello World!"));
    TestPressKey(QStringLiteral("u"));
    QCOMPARE(vi_input_mode_manager->getCurrentViMode(), KateVi::NormalMode);
    QCOMPARE(kate_document->text(), QLatin1String(""));
    TestPressKey(QStringLiteral("\\ctrl-r"));
    QCOMPARE(vi_input_mode_manager->getCurrentViMode(), KateVi::NormalMode);
    FinishTest("Hello World!");

    // Make sure that we don't screw up selection after an undo.
    BeginTest(QStringLiteral("Hola\nHola\nHello\nHallo\n"));
    TestPressKey(QStringLiteral("jVjduVk"));
    QCOMPARE(vi_input_mode_manager->getCurrentViMode(), KateVi::VisualLineMode);
    QCOMPARE(kate_view->selectionText(), QStringLiteral("Hola\nHello"));
    FinishTest("Hola\nHola\nHello\nHallo\n");

    // Test that, if kate_view has a selection before the Vi mode stuff is loaded, then we
    // end up in Visual Mode: this mimics what happens if we click on a Find result in
    // KDevelop's "grepview" plugin.
    delete kate_view;
    kate_view = new KTextEditor::ViewPrivate(kate_document, mainWindow);
    kate_view->setInputMode(View::NormalInputMode);
    mainWindowLayout->addWidget(kate_view);
    kate_document->setText(QStringLiteral("foo bar"));
    kate_view->setSelection(Range(Cursor(0, 1), Cursor(0, 4)));
    QCOMPARE(kate_document->text(kate_view->selectionRange()), QStringLiteral("oo "));
    kate_view->setInputMode(View::ViInputMode);
    qDebug() << "selected: " << kate_document->text(kate_view->selectionRange());
    QVERIFY(kate_view->currentInputMode()->viewInputMode() == View::ViInputMode);
    vi_input_mode = static_cast<KateViInputMode *>(kate_view->currentInputMode());
    vi_input_mode_manager = vi_input_mode->viInputModeManager();
    QVERIFY(vi_input_mode_manager->getCurrentViMode() == KateVi::VisualMode);
    TestPressKey(QStringLiteral("l"));
    QCOMPARE(kate_document->text(kate_view->selectionRange()), QStringLiteral("oo b"));
    TestPressKey(QStringLiteral("d"));
    QCOMPARE(kate_document->text(), QStringLiteral("far"));

    // Test returning to correct mode when selecting ranges with mouse
    BeginTest(QStringLiteral("foo bar\nbar baz"));
    TestPressKey(QStringLiteral("i")); // get me into insert mode
    kate_view->setSelection(Range(0, 1, 1, 4));
    QCOMPARE((int)vi_input_mode_manager->getCurrentViMode(), (int)KateVi::VisualMode);
    kate_view->setSelection(Range::invalid());
    QCOMPARE((int)vi_input_mode_manager->getCurrentViMode(), (int)KateVi::InsertMode);
    TestPressKey(QStringLiteral("\\esc")); // get me into normal mode
    kate_view->setSelection(Range(0, 1, 1, 4));
    QCOMPARE((int)vi_input_mode_manager->getCurrentViMode(), (int)KateVi::VisualMode);
    kate_view->setSelection(Range::invalid());
    QCOMPARE((int)vi_input_mode_manager->getCurrentViMode(), (int)KateVi::NormalMode);
}

// END: Visual mode.

// BEGIN: Command mode.

void ModesTest::CommandTests()
{
    // Testing ":<num>"
    DoTest("foo\nbar\nbaz", "\\:2\\x", "foo\nar\nbaz");
    DoTest("foo\nbar\nbaz", "jmak\\:'a\\x", "foo\nar\nbaz");
    DoTest("foo\nbar\nbaz", "\\:$\\x", "foo\nbar\naz");

    // Testing ":y", ":yank"
    DoTest("foo\nbar\nbaz", "\\:3y\\p", "foo\nbaz\nbar\nbaz");
    DoTest("foo\nbar\nbaz", "\\:2y a 2\\\"ap", "foo\nbar\nbaz\nbar\nbaz");
    DoTest("foo\nbar\nbaz", "\\:y\\p", "foo\nfoo\nbar\nbaz");
    DoTest("foo\nbar\nbaz", "\\:3,1y\\p", "foo\nfoo\nbar\nbaz\nbar\nbaz");

    // Testing ">"
    DoTest("foo", "\\:>\\", "  foo");
    DoTest("   foo", "\\:<\\", "  foo");

    DoTest("foo\nbar", "\\:2>\\", "foo\n  bar");
    DoTest("   foo\nbaz", "\\:1<\\", "  foo\nbaz");

    DoTest("foo\nundo", "\\:2>\\u", "foo\nundo");
    DoTest("  foo\nundo", "\\:1<\\u", "  foo\nundo");

    DoTest("indent\nmultiline\ntext", "\\:1,2>\\", "  indent\n  multiline\ntext");
    DoTest("indent\nmultiline\n+undo", "\\:1,2>\\:1,2>\\:1,2>\\u", "    indent\n    multiline\n+undo");
    // doesn't test correctly, why?
    // DoTest("indent\nmultiline\n+undo", "\\:1,2>\\:1,2<\\u", "  indent\n  multiline\n+undo");

    // Testing ":c", ":change"
    DoTest("foo\nbar\nbaz", "\\:2change\\", "foo\n\nbaz");
    DoTest("foo\nbar\nbaz", "\\:%c\\", "");
    BeginTest(QStringLiteral("foo\nbar\nbaz"));
    TestPressKey(QStringLiteral("\\:$c\\")); // Work around ambiguity in the code that parses commands to execute.
    TestPressKey(QStringLiteral("\\:$change\\"));
    FinishTest("foo\nbar\n");
    DoTest("foo\nbar\nbaz", "ma\\:2,'achange\\", "\nbaz");
    DoTest("foo\nbar\nbaz", "\\:2,3c\\", "foo\n");

    // Testing ":j"
    DoTest("1\n2\n3\n4\n5", "\\:2,4j\\", "1\n2 3 4\n5");

    DoTest("1\n2\n3\n4", "jvj\\ctrl-c\\:'<,'>d\\enter", "1\n4");
    DoTest("1\n2\n3\n4", "\\:1+1+1+1d\\", "1\n2\n3");
    DoTest("1\n2\n3\n4", "2j\\:.,.-1d\\", "1\n4");
    DoTest("1\n2\n3\n4", "\\:.+200-100-100+20-5-5-5-5+.-.,$-1+1-2+2-3+3-4+4-5+5-6+6-7+7-1000+1000+0-0-$+$-.+.-1d\\", "4");
    DoTest("1\n2\n3\n4", "majmbjmcjmdgg\\:'a+'b+'d-'c,.d\\", "");
}

void ModesTest::CommandSedTests()
{
    DoTest("foo", "\\:s/foo/bar\\", "bar");
    DoTest("foobarbaz", "\\:s/bar/xxx\\", "fooxxxbaz");
    DoTest("foo", "\\:s/bar/baz\\", "foo");
    DoTest("foo\nfoo\nfoo", "j\\:s/foo/bar\\", "foo\nbar\nfoo");
    DoTest("foo\nfoo\nfoo", "2jma2k\\:'a,'as/foo/bar\\", "foo\nfoo\nbar");
    DoTest("foo\nfoo\nfoo", "\\:%s/foo/bar\\", "bar\nbar\nbar");
    DoTest("foo\nfoo\nfoo", "\\:2,3s/foo/bar\\", "foo\nbar\nbar");
    DoTest("foo\nfoo\nfoo\nfoo", "j2lmajhmbgg\\:'a,'bs/foo/bar\\", "foo\nbar\nbar\nfoo");
    DoTest("foo\nfoo\nfoo\nfoo", "jlma2jmbgg\\:'b,'as/foo/bar\\", "foo\nbar\nbar\nbar");
    DoTest("foo", "\\:s/$/x/g\\", "foox");
    DoTest("foo", "\\:s/.*/x/g\\", "x");
    DoTest("abc", "\\:s/\\\\s*/x/g\\", "xaxbxc");
    // DoTest("abc\n123", "\\:s/\\\\s*/x/g\\", "xaxbxc\nx1x2x3"); // currently not working properly

    DoTest("foo/bar", "\\:s-/--\\", "foobar");
    DoTest("foo/bar", "\\:s_/__\\", "foobar");

    DoTest("foo\nfoo\nfoo", "\\:2s/foo/bar\\", "foo\nbar\nfoo");
    DoTest("foo\nfoo\nfoo", "2jmagg\\:'as/foo/bar\\", "foo\nfoo\nbar");
    DoTest("foo\nfoo\nfoo", "\\:$s/foo/bar\\", "foo\nfoo\nbar");

    // https://bugs.kde.org/show_bug.cgi?id=235862
    DoTest("try\n\nalso\nfoo", "\\:/r/,/o/s/^/ha/\\", "hatry\nha\nhaalso\nfoo");
    DoTest("much\nmuch\nmuch\nmuch", "\\:.,.+2s/much/try/\\", "try\ntry\ntry\nmuch");
}

void ModesTest::CommandDeleteTests()
{
    DoTest("foo\nbar\nbaz", "\\:2d\\", "foo\nbaz");
    DoTest("foo\nbar\nbaz", "\\:%d\\", "");
    BeginTest(QStringLiteral("foo\nbar\nbaz"));
    TestPressKey(QStringLiteral("\\:$d\\")); // Work around ambiguity in the code that parses commands to execute.
    TestPressKey(QStringLiteral("\\:$d\\"));
    FinishTest("foo");
    DoTest("foo\nbar\nbaz", "ma\\:2,'ad\\", "baz");
    DoTest("foo\nbar\nbaz", "\\:/foo/,/bar/d\\", "baz");
    DoTest("foo\nbar\nbaz", "\\:2,3delete\\", "foo");

    DoTest("foo\nbar\nbaz", "\\:d\\", "bar\nbaz");
    DoTest("foo\nbar\nbaz", "\\:d 33\\", "");
    DoTest("foo\nbar\nbaz", "\\:3d a\\k\"ap", "foo\nbaz\nbar");
}

// END: Command mode.

// BEGIN: Replace mode.

void ModesTest::ReplaceCharacter()
{
    DoTest("", "rr", "");
    DoTest("a", "rb", "b");
    DoTest("abc", "lr\\enter", "a\nc");
    DoTest("abc", "l\\backspace", "abc");
    DoTest("abc", "l\\left", "abc");
}

void ModesTest::ReplaceBasicTests()
{
    // Basic stuff.
    DoTest("", "Rqwerty", "qwerty");
    DoTest("qwerty", "R\\rightXX", "qXXrty");

    // Enter replace and go to the next/previous word.
    DoTest("foo bar", "R\\ctrl-\\rightX", "foo Xar");
    DoTest("foo bar", "R\\ctrl-\\right\\ctrl-\\rightX", "foo barX");
    DoTest("foo bar", "R\\ctrl-\\leftX", "Xoo bar");
    DoTest("foo bar", "R\\ctrl-\\left\\delete", "oo bar");

    // Enter replace mode and go up/down.
    DoTest("foo\nbar\nbaz", "R\\downX", "foo\nXar\nbaz");
    DoTest("foo\nbar\nbaz", "jjR\\upX", "foo\nXar\nbaz");

    // Repeat replacements
    DoTest("foobaz", "Rbar\\esc.", "babarz");
    DoTest("foobarbaz", "Rbar\\esc2.", "babarbarz");
    DoTest("foobarbaz", "Rbar\\esc4.", "babarbarbarbar");
    DoTest("foobarbaz", "Rbar\\esc2.R\\esc2.", "babarbarz");

    // BUG #451076, don't want an extra newline when using the "Find" action from the menu or Ctrl-F
    BaseTest::ensureKateViewVisible(); // required for the bug to trigger
    BeginTest(QStringLiteral("foobar"));
    TestPressKey(QStringLiteral("R"));
    kate_view->find();
    TestPressKey(QStringLiteral("bar\\return"));
    // ensure the cursor is at 'foo_b_ar'
    QCOMPARE(kate_view->cursorPosition().line(), 0);
    QCOMPARE(kate_view->cursorPosition().column(), 3);
    FinishTest("foobar");
    // hide the view again
    kate_view->hide();
    mainWindow->hide();
}

void ModesTest::ReplaceUndoTests()
{
    // Backspace.
    DoTest("", "R\\backspace", "");
    DoTest("qwerty", "lR\\backspaceX", "Xwerty");
    DoTest("qwerty", "lRX\\backspace\\backspaceX", "Xwerty");

    // Ctrl-W
    DoTest("", "R\\ctrl-w", "");
    DoTest("Hello", "lRXX\\ctrl-w", "Hello");
    DoTest("Hello", "lR\t\\ctrl-w", "Hello");
    DoTest("Hello", "lRXX\\left\\ctrl-w", "HXXlo");

    // Ctrl-U
    DoTest("", "R\\ctrl-u", "");
    DoTest("Hello", "lRXX\\ctrl-u", "Hello");
    DoTest("Hello", "lR\t\\ctrl-u", "Hello");
    DoTest("Hello", "lRXX\\left\\ctrl-u", "HXXlo");
    DoTest("Hello World", "3lRXX XX\\ctrl-u", "Hello World");
}

void ModesTest::ReplaceInsertFromLineTests()
{
    // Ctrl-E: replace the current column with the column of the next line.
    DoTest("", "R\\ctrl-e", "");
    DoTest("\n", "jR\\ctrl-e", "\n");
    DoTest("\nqwerty", "R\\ctrl-e\\ctrl-e", "qw\nqwerty");
    DoTest("a\nbb", "R\\ctrl-e\\ctrl-e", "bb\nbb");
    DoTest("aa\n b", "R\\ctrl-e\\ctrl-e", " b\n b");
    DoTest("\n\tb", "R\\ctrl-e\\ctrl-e", "\tb\n\tb");

    // Ctrl-Y: replace the current column with the column of the previous line.
    DoTest("", "R\\ctrl-y", "");
    DoTest("qwerty\n", "jR\\ctrl-y\\ctrl-y", "qwerty\nqw");
    DoTest("aa\nb", "jR\\ctrl-y\\ctrl-y", "aa\naa");
    DoTest(" a\nbb", "jR\\ctrl-y\\ctrl-y", " a\n a");
    DoTest("\tb\n", "jR\\ctrl-y\\ctrl-y", "\tb\n\tb");
}

// END: Replace mode.
#include "moc_modes.cpp"
