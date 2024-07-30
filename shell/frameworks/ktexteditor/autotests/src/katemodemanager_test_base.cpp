/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katemodemanager_test_base.h"

#include <kateglobal.h>

#include <QString>
#include <QTest>

namespace
{
struct FileTypeDataRow {
    const char *dataTag;
    const char *m_inputString;
    const char *m_fileTypeName;

    QString inputString() const
    {
        return QString::fromUtf8(m_inputString);
    }
    QString fileTypeName() const
    {
        return QString::fromUtf8(m_fileTypeName);
    }
};
// The two tables below have been copied from syntax-highlighting:autotests/repository_test_base.cpp and adjusted:
// removed all except first definition/fileType names because only the first name is used in KTextEditor.
// The two versions of the tables should be kept in sync.

// Additional adjustments to the syntax-highlighting version of fileTypesForFileNames table:
//  1) used the second, not first, definition/fileType name in the QRPG.ninja row because
//     "ILERPG" < "Ninja" in KSyntaxHighlighting but "Sources/ILERPG" > "Other/Ninja" in KTextEditor.
//  2) used the second, not first, definition/fileType name in the qrpg*.tt row because
//     "ILERPG" < "TT2" in KSyntaxHighlighting but "Sources/ILERPG" > "Markup/TT2" in KTextEditor.
constexpr FileTypeDataRow fileTypesForFileNames[] = {
    {.dataTag="empty", .m_inputString="", .m_fileTypeName=""},

    {.dataTag="*.tar.gz", .m_inputString="noMatch.tar.gz", .m_fileTypeName=""},
    {.dataTag="No match", .m_inputString="a_random#filename", .m_fileTypeName=""},
    {.dataTag="Long path, no match", .m_inputString="/this/is/a/moderately/long/path/to/no-match", .m_fileTypeName=""},
    {.dataTag="Prefix in dir name", .m_inputString="Kconfig/no_match", .m_fileTypeName=""},

    {.dataTag="*.qml", .m_inputString="/bla/foo.qml", .m_fileTypeName="QML"},
    {.dataTag="*.frag", .m_inputString="flat.frag", .m_fileTypeName="GLSL"},
    {.dataTag="*.md", .m_inputString="highPriority.md", .m_fileTypeName="Markdown"},
    {.dataTag="*.octave", .m_inputString="lowPriority.octave", .m_fileTypeName="Octave"},
    {.dataTag="*.hats", .m_inputString="sameLastLetterPattern.hats", .m_fileTypeName="ATS"},

    {.dataTag="*.c", .m_inputString="test.c", .m_fileTypeName="C"},
    {.dataTag="*.fs", .m_inputString="test.fs", .m_fileTypeName="FSharp"},
    {.dataTag="*.m", .m_inputString="/bla/foo.m", .m_fileTypeName="Objective-C"},

    {.dataTag="Makefile", .m_inputString="Makefile", .m_fileTypeName="Makefile"},
    {.dataTag="Path to Makefile", .m_inputString="/some/path/to/Makefile", .m_fileTypeName="Makefile"},
    {.dataTag="Makefile.*", .m_inputString="Makefile.am", .m_fileTypeName="Makefile"},

    {.dataTag="not-Makefile.dic", .m_inputString="not-Makefile.dic", .m_fileTypeName="Hunspell Dictionary File"},
    {.dataTag="*qmakefile.cpp", .m_inputString="test_qmakefile.cpp", .m_fileTypeName="C++"},
    {.dataTag="*_makefile.mm", .m_inputString="bench_makefile.mm", .m_fileTypeName="Objective-C++"},

    {.dataTag="xorg.conf", .m_inputString="/etc/literal-pattern/xorg.conf", .m_fileTypeName="x.org Configuration"},
    {.dataTag=".profile", .m_inputString="2-literal-patterns/.profile", .m_fileTypeName="Bash"},

    {.dataTag="Config.*", .m_inputString="Config.beginning", .m_fileTypeName="Kconfig"},
    {.dataTag="usr.libexec.*", .m_inputString="usr.libexec.", .m_fileTypeName="AppArmor Security Profile"},
    {.dataTag="Jam*", .m_inputString="Jam-beginning-no-dot", .m_fileTypeName="Jam"},
    {.dataTag="usr.li-*.ch", .m_inputString="usr.li-many-partial-prefix-matches.ch", .m_fileTypeName="xHarbour"},
    {.dataTag="QRPG*.*", .m_inputString="QRPG1u4[+.unusual", .m_fileTypeName="ILERPG"},

    {.dataTag="*patch", .m_inputString="no-dot-before-ending~patch", .m_fileTypeName="Diff"},
    {.dataTag="*.cmake.in", .m_inputString="two-dots-after-asterisk.cmake.in", .m_fileTypeName="CMake"},
    {.dataTag="*.html.mst", .m_inputString="two-dots-priority!=0.html.mst", .m_fileTypeName="Mustache/Handlebars (HTML)"},

    {.dataTag="*.desktop.cmake", .m_inputString="2_suffixes.desktop.cmake", .m_fileTypeName=".desktop"},
    {.dataTag="*.per.err", .m_inputString="2_suffixes-but-one-a-better-match.per.err", .m_fileTypeName="4GL"},
    {.dataTag="*.xml.eex", .m_inputString="2_suffixes-one-lang.xml.eex", .m_fileTypeName="Elixir"},
    {.dataTag="fishd.*.fish", .m_inputString="fishd.prefix,suffix=one-lang.fish", .m_fileTypeName="Fish"},

    {.dataTag="usr.bin.*.ftl", .m_inputString="usr.bin.heterogenousPatternMatch.ftl", .m_fileTypeName="AppArmor Security Profile"},
    {.dataTag="Doxyfile.*.pro", .m_inputString="Doxyfile.heterogenous.Pattern-Match.pro", .m_fileTypeName="QMake"},
    {.dataTag="Kconfig*.ml", .m_inputString="KconfigHeterogenous_pattern_match.ml", .m_fileTypeName="Objective Caml"},
    {.dataTag="snap-confine.*.html.rac", .m_inputString="snap-confine.2.-higher-priority.html.rac", .m_fileTypeName="Mustache/Handlebars (HTML)"},
    {.dataTag="file_contexts_*.fq.gz", .m_inputString="file_contexts_prefix-higher-priority.fq.gz", .m_fileTypeName="SELinux File Contexts"},
    {.dataTag="QRPG*.ninja", .m_inputString="QRPG.ninja", .m_fileTypeName="Ninja"},
    {.dataTag="qrpg*.tt", .m_inputString="qrpgTwoUnusualPatterns.tt", .m_fileTypeName="TT2"},
    {.dataTag="qrpg*.cl", .m_inputString="qrpg$heterogenous~pattern&match.cl", .m_fileTypeName="OpenCL"},
    {.dataTag=".gitignore*.tt*.textile", .m_inputString=".gitignoreHeterogenous3.tt.textile", .m_fileTypeName="Textile"},
};

constexpr FileTypeDataRow fileTypesForMimeTypeNames[] = {
    {.dataTag="empty", .m_inputString="", .m_fileTypeName=""},

    {.dataTag="Nonexistent MIME type", .m_inputString="text/nonexistent-mt", .m_fileTypeName=""},
    {.dataTag="No match", .m_inputString="application/x-bzip-compressed-tar", .m_fileTypeName=""},

    {.dataTag="High priority", .m_inputString="text/rust", .m_fileTypeName="Rust"},
    {.dataTag="Negative priority", .m_inputString="text/octave", .m_fileTypeName="Octave"},

    {.dataTag="Multiple types match", .m_inputString="text/x-chdr", .m_fileTypeName="C++"},
};

template<std::size_t size>
void addFileTypeDataRows(const FileTypeDataRow (&array)[size])
{
    for (const auto &row : array) {
        QTest::newRow(row.dataTag) << row.inputString() << row.fileTypeName();
    }
}
} // unnamed namespace

KateModeManagerTestBase::KateModeManagerTestBase()
    : m_modeManager{KTextEditor::EditorPrivate::self()->modeManager()}
{
}

void KateModeManagerTestBase::wildcardsFindTestData()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("fileTypeName");
    addFileTypeDataRows(fileTypesForFileNames);
}

void KateModeManagerTestBase::mimeTypesFindTestData()
{
    QTest::addColumn<QString>("mimeTypeName");
    QTest::addColumn<QString>("fileTypeName");
    addFileTypeDataRows(fileTypesForMimeTypeNames);
}

#include "moc_katemodemanager_test_base.cpp"
