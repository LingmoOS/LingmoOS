// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "syntax.h"
#include "regexp.h"
#include "unicode.h"

#include "base/i18n.h"
#include "base/locale.h"
#include "base/mime_database.h"
#include "base/strings.h"

#include <algorithm>
#include <filesystem>
#include <fnmatch.h>
#include <unordered_set>
#include <utility>

using Lingmo::i18n::tr;
namespace strings = Lingmo::base::strings;

struct WordHash
{
    using is_transparent = void;
    size_t operator()(std::string_view word) const { return std::hash<std::string_view>{}(word); }
};

using WordSet = std::unordered_set<std::string, WordHash, std::equal_to<>>;

struct Language
{
    enum Kind {
        Code,
        Markdown,
        Diff,
        Ini,
        Yaml,
        Markup,
        Json,
        Log,
    };

    std::string id;
    const char *name;
    Kind kind = Code;
    std::vector<std::string> patterns;
    std::vector<std::string> interpreters;
    std::vector<std::string> mimeTypes;
    std::vector<std::string> lineComments;
    std::vector<std::pair<std::string, std::string>> blockComments;
    std::string quotes;
    std::vector<std::string> multilineQuotes;
    WordSet keywords;
    WordSet types;
    WordSet constants;
    bool caseInsensitive = false;
    bool hashPreprocessor = false;
    bool dollarVariables = false;
    bool decorators = false;
    bool hexColors = false;
    bool functions = true;
};

static WordSet words(const char *list)
{
    WordSet result;
    for (std::string &word : strings::split(list, ' ', strings::SplitBehavior::SkipEmptyParts))
        result.insert(std::move(word));
    return result;
}

static void addWords(WordSet &set, const char *list)
{
    WordSet more = words(list);
    set.merge(more);
}

static std::vector<Language> makeLanguages()
{
    std::vector<Language> list;
    const std::pair<std::string, std::string> cComment{"/*", "*/"};
    const std::vector<std::string> slashes{"//"};
    const std::vector<std::string> hash{"#"};
    const WordSet cTypes = words("void bool char short int long float double signed unsigned auto size_t ssize_t "
                                       "int8_t int16_t int32_t int64_t uint8_t uint16_t uint32_t uint64_t wchar_t "
                                       "char8_t char16_t char32_t intptr_t uintptr_t ptrdiff_t FILE");

    {
        Language l;
        l.id = "c";
        l.name = "C";
        l.patterns = {"*.c", "*.h"};
        l.mimeTypes = {"text/x-csrc", "text/x-chdr"};
        l.lineComments = slashes;
        l.blockComments = {cComment};
        l.quotes = "\"'";
        l.keywords = words("break case const continue default do else enum extern for goto if inline register restrict "
                           "return sizeof static struct switch typedef union volatile while _Bool _Atomic _Generic "
                           "_Noreturn _Static_assert _Thread_local alignas alignof static_assert thread_local typeof");
        l.types = cTypes;
        l.constants = words("NULL true false EOF stdin stdout stderr nullptr");
        l.hashPreprocessor = true;
        list.push_back(l);
    }
    {
        Language l = list.back();
        l.id = "cpp";
        l.name = "C++";
        l.patterns = {"*.cpp", "*.cc", "*.cxx", "*.c++",
                      "*.hpp", "*.hh", "*.hxx", "*.h++",
                      "*.ipp", "*.tpp", "*.inl"};
        l.mimeTypes = {"text/x-c++src", "text/x-c++hdr"};
        addWords(l.keywords, "alignas and asm catch class co_await co_return co_yield concept consteval constexpr constinit "
                            "const_cast decltype delete dynamic_cast explicit export final friend mutable namespace new "
                            "noexcept not operator or override private protected public reinterpret_cast requires "
                            "static_cast template this throw try typeid typename using virtual xor emit signals slots "
                            "Q_OBJECT Q_PROPERTY Q_INVOKABLE Q_SIGNALS Q_SLOTS Q_EMIT");
        addWords(l.types, "std string wstring vector map set unordered_map unique_ptr shared_ptr QString QStringList "
                         "QList QByteArray QVariant QObject qint64 quint64 qint32 quint32 qreal");
        list.push_back(l);
    }
    {
        Language l;
        l.id = "python";
        l.name = "Python";
        l.patterns = {"*.py", "*.pyw", "*.pyi",
                      "SConstruct", "SConscript"};
        l.interpreters = {"python", "python2", "python3"};
        l.mimeTypes = {"text/x-python", "text/x-python3"};
        l.lineComments = hash;
        l.quotes = "\"'";
        l.multilineQuotes = {"\"\"\"", "'''"};
        l.keywords = words("and as assert async await break class continue def del elif else except finally for from "
                           "global if import in is lambda nonlocal not or pass raise return try while with yield match case");
        l.types = words("int float str bytes bool list dict set tuple object type complex frozenset bytearray");
        l.constants = words("None True False self cls __name__ __main__ __init__ print len range open super isinstance");
        l.decorators = true;
        list.push_back(l);
    }
    {
        Language l;
        l.id = "javascript";
        l.name = "JavaScript";
        l.patterns = {"*.js", "*.mjs", "*.cjs", "*.jsx"};
        l.interpreters = {"node", "nodejs", "gjs"};
        l.mimeTypes = {"application/javascript", "text/javascript"};
        l.lineComments = slashes;
        l.blockComments = {cComment};
        l.quotes = "\"'";
        l.multilineQuotes = {"`"};
        l.keywords = words("async await break case catch class const continue debugger default delete do else export "
                           "extends finally for from function get if import in instanceof let new of return set static "
                           "super switch this throw try typeof var void while with yield");
        l.constants = words("true false null undefined NaN Infinity console window document globalThis");
        list.push_back(l);
    }
    {
        Language l = list.back();
        l.id = "typescript";
        l.name = "TypeScript";
        l.patterns = {"*.ts", "*.tsx", "*.mts", "*.cts"};
        l.interpreters = {"deno", "ts-node"};
        l.mimeTypes = {"application/x-typescript", "text/x-typescript"};
        addWords(l.keywords, "abstract as declare enum implements interface keyof namespace private protected public "
                            "readonly type satisfies infer is");
        l.types = words("any boolean number string symbol bigint never unknown object void");
        list.push_back(l);
    }
    {
        Language l = list[3];
        l.id = "qml";
        l.name = "QML";
        l.patterns = {"*.qml", "qmldir"};
        l.interpreters = {};
        l.mimeTypes = {"text/x-qml"};
        addWords(l.keywords, "property readonly required signal alias component pragma on default");
        l.types = words("int real double bool string var url color point size rect list date font");
        list.push_back(l);
    }
    {
        Language l;
        l.id = "sh";
        l.name = "Shell";
        l.patterns = {"*.sh", "*.bash", "*.zsh", "*.ksh",
                      ".bashrc", ".bash_profile", ".bash_aliases",
                      ".profile", ".zshrc", ".zprofile",
                      "PKGBUILD", "*.ebuild", "APKBUILD"};
        l.interpreters = {"sh", "bash", "zsh", "dash",
                          "ksh", "ash"};
        l.mimeTypes = {"application/x-shellscript", "text/x-sh"};
        l.lineComments = hash;
        l.quotes = "\"'`";
        l.keywords = words("if then else elif fi case esac for select while until do done in function time coproc "
                           "return exit break continue local export readonly declare typeset unset shift source alias "
                           "set trap eval exec");
        l.constants = words("true false echo printf read cd test");
        l.dollarVariables = true;
        l.functions = false;
        list.push_back(l);
    }
    {
        Language l;
        l.id = "rust";
        l.name = "Rust";
        l.patterns = {"*.rs"};
        l.mimeTypes = {"text/rust", "text/x-rust"};
        l.lineComments = slashes;
        l.blockComments = {cComment};
        l.quotes = "\"";
        l.keywords = words("as async await break const continue crate dyn else enum extern fn for if impl in let loop "
                           "match mod move mut pub ref return static struct super trait type unsafe use where while "
                           "macro_rules union");
        l.types = words("i8 i16 i32 i64 i128 isize u8 u16 u32 u64 u128 usize f32 f64 bool char str String Vec Option "
                        "Result Box Self HashMap");
        l.constants = words("true false None Some Ok Err self");
        list.push_back(l);
    }
    {
        Language l;
        l.id = "go";
        l.name = "Go";
        l.patterns = {"*.go"};
        l.mimeTypes = {"text/x-go"};
        l.lineComments = slashes;
        l.blockComments = {cComment};
        l.quotes = "\"'";
        l.multilineQuotes = {"`"};
        l.keywords = words("break case chan const continue default defer else fallthrough for func go goto if import "
                           "interface map package range return select struct switch type var");
        l.types = words("bool byte complex64 complex128 error float32 float64 int int8 int16 int32 int64 rune string "
                        "uint uint8 uint16 uint32 uint64 uintptr any");
        l.constants = words("true false nil iota append cap close copy delete len make new panic print println recover");
        list.push_back(l);
    }
    {
        Language l;
        l.id = "java";
        l.name = "Java";
        l.patterns = {"*.java", "*.kt", "*.kts"};
        l.mimeTypes = {"text/x-java"};
        l.lineComments = slashes;
        l.blockComments = {cComment};
        l.quotes = "\"'";
        l.multilineQuotes = {"\"\"\""};
        l.keywords = words("abstract assert break case catch class continue default do else enum extends final finally "
                           "for if implements import instanceof interface native new package private protected public "
                           "return static strictfp super switch synchronized this throw throws transient try volatile "
                           "while var record sealed permits yield fun val when object companion");
        l.types = words("boolean byte char double float int long short void String Object Integer Long Boolean List Map");
        l.constants = words("true false null");
        l.decorators = true;
        list.push_back(l);
    }
    {
        Language l;
        l.id = "css";
        l.name = "CSS";
        l.patterns = {"*.css", "*.scss", "*.less", "*.qss"};
        l.mimeTypes = {"text/css", "text/x-scss"};
        l.blockComments = {cComment};
        l.quotes = "\"'";
        l.keywords = words("@media @import @font-face @keyframes @supports @charset @layer !important");
        l.constants = words("none auto inherit initial unset transparent block inline flex grid absolute relative fixed "
                            "solid bold normal center left right top bottom");
        l.hexColors = true;
        list.push_back(l);
    }
    {
        Language l;
        l.id = "sql";
        l.name = "SQL";
        l.patterns = {"*.sql"};
        l.mimeTypes = {"application/sql", "text/x-sql"};
        l.lineComments = {"--"};
        l.blockComments = {cComment};
        l.quotes = "'\"";
        l.caseInsensitive = true;
        l.keywords = words("select from where insert into values update set delete create table drop alter add column "
                           "index view primary key foreign references join inner left right outer full cross on as "
                           "and or not null is in like between order by group having limit offset union all distinct "
                           "case when then else end begin commit rollback transaction if exists default unique check "
                           "constraint returning with recursive asc desc trigger replace pragma");
        l.types = words("int integer bigint smallint real float double decimal numeric char varchar text blob boolean "
                        "date time timestamp serial");
        l.constants = words("true false current_timestamp current_date count sum avg min max coalesce");
        list.push_back(l);
    }
    {
        Language l;
        l.id = "cmake";
        l.name = "CMake";
        l.patterns = {"CMakeLists.txt", "*.cmake", "*.cmake.in"};
        l.mimeTypes = {"text/x-cmake"};
        l.lineComments = hash;
        l.quotes = "\"";
        l.keywords = words("if elseif else endif foreach endforeach while endwhile function endfunction macro endmacro "
                           "return break continue");
        l.constants = words("ON OFF TRUE FALSE YES NO PRIVATE PUBLIC INTERFACE REQUIRED COMPONENTS NOT AND OR STREQUAL "
                            "EQUAL DEFINED");
        l.dollarVariables = true;
        list.push_back(l);
    }
    {
        Language l;
        l.id = "makefile";
        l.name = "Makefile";
        l.patterns = {"Makefile", "makefile", "GNUmakefile",
                      "*.mk", "*.mak", "Makefile.*"};
        l.mimeTypes = {"text/x-makefile"};
        l.lineComments = hash;
        l.quotes = "\"'";
        l.keywords = words("ifeq ifneq ifdef ifndef else endif include define endef export unexport override .PHONY");
        l.dollarVariables = true;
        l.functions = false;
        list.push_back(l);
    }
    {
        Language l;
        l.id = "markdown";
        l.name = "Markdown";
        l.kind = Language::Markdown;
        l.patterns = {"*.md", "*.markdown", "*.mkd"};
        l.mimeTypes = {"text/markdown", "text/x-markdown"};
        list.push_back(l);
    }
    {
        Language l;
        l.id = "html";
        l.name = "HTML";
        l.kind = Language::Markup;
        l.patterns = {"*.html", "*.htm", "*.xhtml"};
        l.mimeTypes = {"text/html", "application/xhtml+xml"};
        list.push_back(l);
    }
    {
        Language l;
        l.id = "xml";
        l.name = "XML";
        l.kind = Language::Markup;
        l.patterns = {"*.xml", "*.svg", "*.ui", "*.qrc",
                      "*.xsl", "*.xslt", "*.plist",
                      "*.xbel", "*.kml", "*.gpx", "*.rss",
                      "*.atom", "*.policy"};
        l.mimeTypes = {"application/xml", "text/xml", "image/svg+xml"};
        list.push_back(l);
    }
    {
        Language l;
        l.id = "json";
        l.name = "JSON";
        l.kind = Language::Json;
        l.patterns = {"*.json", "*.jsonc", "*.json5",
                      "*.geojson", "*.webmanifest", ".babelrc",
                      ".eslintrc"};
        l.mimeTypes = {"application/json"};
        l.lineComments = slashes;
        l.blockComments = {cComment};
        l.quotes = "\"";
        l.constants = words("true false null");
        l.functions = false;
        list.push_back(l);
    }
    {
        Language l;
        l.id = "yaml";
        l.name = "YAML";
        l.kind = Language::Yaml;
        l.patterns = {"*.yaml", "*.yml", ".clang-format",
                      ".clang-tidy"};
        l.mimeTypes = {"application/x-yaml", "application/yaml"};
        l.lineComments = hash;
        l.quotes = "\"'";
        l.constants = words("true false null yes no on off True False Null ~");
        l.functions = false;
        list.push_back(l);
    }
    {
        Language l;
        l.id = "toml";
        l.name = "TOML";
        l.kind = Language::Ini;
        l.patterns = {"*.toml", "Cargo.lock", "Pipfile"};
        l.mimeTypes = {"application/toml"};
        l.lineComments = hash;
        l.quotes = "\"'";
        l.multilineQuotes = {"\"\"\"", "'''"};
        l.constants = words("true false inf nan");
        l.functions = false;
        list.push_back(l);
    }
    {
        Language l;
        l.id = "ini";
        l.name = "INI";
        l.kind = Language::Ini;
        l.patterns = {"*.ini", "*.conf", "*.cfg", "*.desktop",
                      "*.service", "*.socket", "*.timer",
                      "*.target", "*.mount", "*.path",
                      "*.network", "*.netdev", "*.link",
                      "*.properties", "*.editorconfig", ".editorconfig",
                      ".gitconfig", ".gitmodules", "*.theme",
                      "*.directory", "*.rc", "mimeapps.list",
                      "*.nmconnection", "*.inf"};
        l.mimeTypes = {"text/x-ini", "application/x-desktop",
                       "text/x-systemd-unit"};
        l.lineComments = {"#", ";"};
        l.constants = words("true false yes no on off");
        l.functions = false;
        list.push_back(l);
    }
    {
        Language l;
        l.id = "diff";
        l.name = "Diff";
        l.kind = Language::Diff;
        l.patterns = {"*.diff", "*.patch", "*.rej",
                      "*.debdiff"};
        l.mimeTypes = {"text/x-patch", "text/x-diff"};
        list.push_back(l);
    }
    {
        Language l;
        l.id = "log";
        l.name = "Log";
        l.kind = Language::Log;
        l.patterns = {"*.log", "*.log.[0-9]", "Xorg.*.log"};
        l.mimeTypes = {"text/x-log"};
        list.push_back(l);
    }
    return list;
}

static const std::vector<Language> &allLanguages()
{
    static const std::vector<Language> list = makeLanguages();
    return list;
}

const std::vector<const Language *> &Syntax::languages()
{
    static const std::vector<const Language *> list = [] {
        std::vector<const Language *> result;
        for (const Language &language : allLanguages())
            result.push_back(&language);
        const Lingmo::base::Locale &locale = Lingmo::base::Locale::system();
        std::stable_sort(result.begin(), result.end(), [&locale](const Language *a, const Language *b) {
            return locale.compare(a->name, b->name) < 0;
        });
        return result;
    }();
    return list;
}

const Language *Syntax::language(std::string_view id)
{
    if (id.empty())
        return nullptr;
    for (const Language &language : allLanguages()) {
        if (language.id == id)
            return &language;
    }
    return nullptr;
}

std::string Syntax::id(const Language *language)
{
    return language ? language->id : std::string();
}

std::string Syntax::name(std::string_view id)
{
    const Language *l = language(id);
    return l ? l->name : tr("Plain Text");
}

// Without trailing version digits: "python3.11" is "python".
static std::string_view withoutVersion(std::string_view name)
{
    while (!name.empty() && (strings::isAsciiDigit(name.back()) || name.back() == '.'))
        name.remove_suffix(1);
    return name;
}

std::string Syntax::detect(const std::string &fileName, std::string_view firstLine)
{
    const std::string base = std::filesystem::path(fileName).filename().string();
    if (!base.empty()) {
        // Exact names first: "CMakeLists.txt" is CMake, not a text file.
        for (const Language &language : allLanguages()) {
            for (const std::string &pattern : language.patterns) {
                if (pattern.find('*') == std::string::npos && pattern == base)
                    return language.id;
            }
        }
        for (const Language &language : allLanguages()) {
            for (const std::string &pattern : language.patterns) {
                if (pattern.find('*') != std::string::npos
                        && ::fnmatch(pattern.c_str(), base.c_str(), FNM_CASEFOLD | FNM_PERIOD) == 0)
                    return language.id;
            }
        }
    }

    if (strings::startsWith(firstLine, "#!")) {
        // "#!/usr/bin/env -S python3 -u" names python3.
        for (const std::string_view part : strings::splitViews(firstLine.substr(2), ' ', strings::SplitBehavior::SkipEmptyParts)) {
            if (part[0] == '-' || strings::endsWith(part, "/env"))
                continue;
            const size_t slash = part.rfind('/');
            const std::string_view interpreter = withoutVersion(slash == std::string_view::npos ? part : part.substr(slash + 1));
            for (const Language &language : allLanguages()) {
                for (const std::string &name : language.interpreters) {
                    if (withoutVersion(name) == interpreter)
                        return language.id;
                }
            }
            break;
        }
    }
    if (strings::startsWith(firstLine, "<?xml"))
        return "xml";
    if (strings::startsWith(firstLine, "<!DOCTYPE html", strings::CaseSensitivity::Insensitive)
            || strings::startsWith(firstLine, "<html", strings::CaseSensitivity::Insensitive))
        return "html";
    if (strings::startsWith(firstLine, "diff --git ") || strings::startsWith(firstLine, "--- "))
        return "diff";

    if (!base.empty()) {
        const std::string mime = Lingmo::base::mime::typeForFileName(base);
        for (const Language &language : allLanguages()) {
            for (const std::string &type : language.mimeTypes) {
                if (Lingmo::base::mime::inherits(mime, type))
                    return language.id;
            }
        }
    }
    return std::string();
}

static bool isWordStart(char32_t c)
{
    return Unicode::isLetter(c) || c == '_';
}

static bool isWordChar(char32_t c)
{
    return Unicode::isLetterOrNumber(c) || c == '_';
}

static void add(std::vector<SyntaxSpan> *spans, int start, int length, SyntaxStyle style)
{
    if (length <= 0 || style == SyntaxStyle::Normal)
        return;
    if (!spans->empty()) {
        SyntaxSpan &last = spans->back();
        if (last.style == style && last.start + last.length == start) {
            last.length += length;
            return;
        }
    }
    spans->push_back({start, length, style});
}

namespace
{
// A line read a character at a time by byte offset.
struct Line
{
    std::string_view text;
    int size;

    explicit Line(std::string_view line)
        : text(line)
        , size(int(line.size()))
    {
    }

    char32_t at(int i) const { return Unicode::at(text, size_t(i)); }
    char32_t before(int i) const { return Unicode::before(text, size_t(i)); }
    int next(int i) const { return int(Unicode::next(text, size_t(i))); }
    bool startsWith(int i, std::string_view prefix) const { return text.substr(size_t(i)).substr(0, prefix.size()) == prefix; }
    int find(std::string_view what, int from = 0) const
    {
        const size_t found = text.find(what, size_t(from));
        return found == std::string_view::npos ? -1 : int(found);
    }
    int find(char what, int from = 0) const
    {
        const size_t found = text.find(what, size_t(from));
        return found == std::string_view::npos ? -1 : int(found);
    }
    // Past white space from `i`.
    int skipSpace(int i) const
    {
        while (i < size && Unicode::isSpace(at(i)))
            i = next(i);
        return i;
    }
    int skipWord(int i) const
    {
        while (i < size && isWordChar(at(i)))
            i = next(i);
        return i;
    }
};
}

// Scans a quoted string from `i` (just past the opening quote); returns the
// index past the closing one, or -1 when the line ends first.
static int scanString(const Line &line, int i, std::string_view close, std::vector<SyntaxSpan> *spans, int start,
                      bool escapes = true)
{
    while (i < line.size) {
        if (escapes && line.text[size_t(i)] == '\\' && i + 1 < line.size) {
            add(spans, start, i - start, SyntaxStyle::String);
            const int escaped = line.next(i + 1);
            add(spans, i, escaped - i, SyntaxStyle::Escape);
            i = escaped;
            start = i;
            continue;
        }
        if (line.startsWith(i, close)) {
            add(spans, start, i + int(close.size()) - start, SyntaxStyle::String);
            return i + int(close.size());
        }
        i = line.next(i);
    }
    add(spans, start, line.size - start, SyntaxStyle::String);
    return -1;
}

static int scanNumber(const Line &line, int i)
{
    const int begin = i;
    const bool hex = line.size - begin >= 2 && line.text[size_t(begin)] == '0'
            && (line.text[size_t(begin) + 1] == 'x' || line.text[size_t(begin) + 1] == 'X');
    while (i < line.size) {
        const char32_t c = line.at(i);
        if (Unicode::isLetterOrNumber(c) || c == '_' || c == '.') {
            i = line.next(i);
        } else if ((c == '+' || c == '-') && i > begin && (line.before(i) == 'e' || line.before(i) == 'E') && !hex) {
            ++i;
        } else {
            break;
        }
    }
    return i;
}

static constexpr int multilineBase = 16;

static Syntax::State highlightCode(const Language &l, std::string_view text, Syntax::State state, std::vector<SyntaxSpan> *spans)
{
    const Line line(text);
    const int size = line.size;
    int i = 0;

    // Carried over: inside a block comment or a multi-line string.
    if (state > 0 && state < multilineBase) {
        const std::string &close = l.blockComments[size_t(state - 1)].second;
        const int end = line.find(close);
        if (end < 0) {
            add(spans, 0, size, SyntaxStyle::Comment);
            return state;
        }
        i = end + int(close.size());
        add(spans, 0, i, SyntaxStyle::Comment);
        state = 0;
    } else if (state >= multilineBase) {
        const std::string &close = l.multilineQuotes[size_t(state - multilineBase)];
        const int end = scanString(line, 0, close, spans, 0);
        if (end < 0)
            return state;
        i = end;
        state = 0;
    }

    const int firstNonSpace = line.skipSpace(0);

    if (l.hashPreprocessor && firstNonSpace < size && text[size_t(firstNonSpace)] == '#' && i == 0) {
        int end = line.skipWord(line.skipSpace(firstNonSpace + 1));
        add(spans, firstNonSpace, end - firstNonSpace, SyntaxStyle::Preprocessor);
        // #include <file> reads as a string.
        const int open = line.find('<', end);
        if (text.substr(size_t(firstNonSpace)).find("include") != std::string_view::npos && open >= 0) {
            const int close = line.find('>', open);
            if (close > open) {
                add(spans, open, close - open + 1, SyntaxStyle::String);
                end = close + 1;
            }
        }
        i = end;
    }

    // Makefile targets start at the beginning of a line.
    if (l.id == "makefile" && i == 0 && firstNonSpace == 0) {
        const int colon = line.find(':');
        if (colon > 0 && text.substr(0, size_t(colon)).find('=') == std::string_view::npos && text[0] != '\t'
                && (colon + 1 >= size || text[size_t(colon) + 1] != '=')) {
            add(spans, 0, colon, SyntaxStyle::Function);
            i = colon;
        }
    }

    const bool css = l.id == "css";
    while (i < size) {
        const char32_t c = line.at(i);

        bool matched = false;
        for (const std::string &marker : l.lineComments) {
            if (line.startsWith(i, marker)) {
                // "#" inside a word, e.g. "a#b" in shell, is no comment.
                if (marker == "#" && l.dollarVariables && i > 0 && !Unicode::isSpace(line.before(i)))
                    continue;
                add(spans, i, size - i, SyntaxStyle::Comment);
                return 0;
            }
        }
        for (size_t b = 0; b < l.blockComments.size(); ++b) {
            if (line.startsWith(i, l.blockComments[b].first)) {
                const int end = line.find(l.blockComments[b].second, i + int(l.blockComments[b].first.size()));
                if (end < 0) {
                    add(spans, i, size - i, SyntaxStyle::Comment);
                    return int(b) + 1;
                }
                const int stop = end + int(l.blockComments[b].second.size());
                add(spans, i, stop - i, SyntaxStyle::Comment);
                i = stop;
                matched = true;
                break;
            }
        }
        if (matched)
            continue;

        for (size_t q = 0; q < l.multilineQuotes.size(); ++q) {
            const std::string &quote = l.multilineQuotes[q];
            if (line.startsWith(i, quote)) {
                const int end = scanString(line, i + int(quote.size()), quote, spans, i);
                if (end < 0)
                    return multilineBase + int(q);
                i = end;
                matched = true;
                break;
            }
        }
        if (matched)
            continue;

        if (c < 0x80 && l.quotes.find(char(c)) != std::string::npos) {
            // A JSON string followed by a colon is a key.
            const size_t before = spans->size();
            const bool escapes = !(l.id == "sh" && c == '\'');
            int end = scanString(line, i + 1, std::string_view(&text[size_t(i)], 1), spans, i, escapes);
            if (end < 0)
                end = size;
            if (l.kind == Language::Json || l.kind == Language::Yaml) {
                const int after = line.skipSpace(end);
                if (after < size && text[size_t(after)] == ':') {
                    spans->resize(before);
                    add(spans, i, end - i, SyntaxStyle::Key);
                }
            }
            i = end;
            continue;
        }

        if (l.dollarVariables && c == '$' && i + 1 < size) {
            int end = i + 1;
            const char32_t next = line.at(end);
            if (next == '{' || next == '(') {
                const int stop = line.find(next == '{' ? '}' : ')', end);
                end = stop < 0 ? size : stop + 1;
            } else if (isWordChar(next)) {
                end = line.skipWord(end);
            } else {
                end = line.next(end);
            }
            add(spans, i, end - i, SyntaxStyle::Variable);
            i = end;
            continue;
        }

        if (l.decorators && c == '@' && i + 1 < size && isWordStart(line.at(i + 1))) {
            int end = i + 1;
            while (end < size && (isWordChar(line.at(end)) || text[size_t(end)] == '.'))
                end = line.next(end);
            add(spans, i, end - i, SyntaxStyle::Preprocessor);
            i = end;
            continue;
        }

        if (l.hexColors && c == '#' && i + 1 < size && Unicode::isLetterOrNumber(line.at(i + 1))
                && (i == 0 || !isWordChar(line.before(i)))) {
            int end = i + 1;
            int characters = 1;
            while (end < size && (isWordChar(line.at(end)) || text[size_t(end)] == '-')) {
                end = line.next(end);
                ++characters;
            }
            const bool color = characters == 4 || characters == 5 || characters == 7 || characters == 9;
            add(spans, i, end - i, color ? SyntaxStyle::Number : SyntaxStyle::Function);
            i = end;
            continue;
        }

        if (Unicode::isDigit(c) && (i == 0 || !isWordChar(line.before(i)))) {
            const int end = scanNumber(line, i);
            add(spans, i, end - i, SyntaxStyle::Number);
            i = end;
            continue;
        }

        if (isWordStart(c) || (c == '@' && css) || (c == '.' && l.id == "makefile") || (c == '!' && css)) {
            int end = line.next(i);
            while (end < size && (isWordChar(line.at(end)) || (css && text[size_t(end)] == '-')))
                end = line.next(end);
            const std::string_view word = text.substr(size_t(i), size_t(end - i));
            const std::string lowered = l.caseInsensitive ? Unicode::toLower(word) : std::string();
            const std::string_view key = l.caseInsensitive ? std::string_view(lowered) : word;
            SyntaxStyle style = SyntaxStyle::Normal;
            if (l.keywords.find(key) != l.keywords.end())
                style = SyntaxStyle::Keyword;
            else if (l.types.find(key) != l.types.end())
                style = SyntaxStyle::Type;
            else if (l.constants.find(key) != l.constants.end())
                style = SyntaxStyle::Constant;
            else if (l.functions && end < size && text[size_t(end)] == '(')
                style = SyntaxStyle::Function;
            else if (l.kind == Language::Yaml || css) {
                int after = end;
                while (after < size && (isWordChar(line.at(after)) || text[size_t(after)] == '-' || text[size_t(after)] == '.'))
                    after = line.next(after);
                if (after < size && text[size_t(after)] == ':'
                        && (after + 1 >= size || Unicode::isSpace(line.at(after + 1)))) {
                    style = SyntaxStyle::Key;
                    end = after;
                }
            }
            add(spans, i, end - i, style);
            i = end;
            continue;
        }

        i = line.next(i);
    }
    return 0;
}

static Syntax::State highlightMarkdown(std::string_view text, Syntax::State state, std::vector<SyntaxSpan> *spans)
{
    const Line line(text);
    const int size = line.size;
    const std::string_view trimmed = strings::trimmed(text);
    const bool fence = strings::startsWith(trimmed, "```") || strings::startsWith(trimmed, "~~~");
    if (state == 1) {
        add(spans, 0, size, SyntaxStyle::Code);
        return fence ? 0 : 1;
    }
    if (fence) {
        add(spans, 0, size, SyntaxStyle::Code);
        return 1;
    }

    if (strings::startsWith(trimmed, "#")) {
        add(spans, 0, size, SyntaxStyle::Heading);
        return 0;
    }
    if (strings::startsWith(trimmed, ">")) {
        add(spans, 0, size, SyntaxStyle::Comment);
        return 0;
    }
    static const RegExp rule("^\\s*([-*_])(\\s*\\1){2,}\\s*$");
    if (rule.match(text).hasMatch()
            || (strings::startsWith(text, "===") && size_t(std::count(trimmed.begin(), trimmed.end(), '=')) == trimmed.size())) {
        add(spans, 0, size, SyntaxStyle::Heading);
        return 0;
    }

    int i = line.skipSpace(0);
    static const RegExp list("^([-*+]|\\d+[.)])\\s");
    const RegExp::Match marker = list.match(text, size_t(i), true);
    if (marker.hasMatch()) {
        add(spans, i, int(marker.length(1)), SyntaxStyle::Keyword);
        i += int(marker.length());
    }

    while (i < size) {
        const char c = text[size_t(i)];
        if (c == '\\' && i + 1 < size) {
            i = line.next(i + 1);
            continue;
        }
        if (c == '`') {
            const int end = line.find('`', i + 1);
            if (end > i) {
                add(spans, i, end - i + 1, SyntaxStyle::Code);
                i = end + 1;
                continue;
            }
        }
        if ((c == '*' || c == '_') && i + 1 < size) {
            const bool strong = text[size_t(i) + 1] == c;
            const std::string marker = strong ? std::string(2, c) : std::string(1, c);
            const int from = i + int(marker.size());
            // "_" inside a word, as in snake_case, is not emphasis.
            const bool wordInside = c == '_' && i > 0 && isWordChar(line.before(i));
            if (!wordInside && from < size && !Unicode::isSpace(line.at(from))) {
                const int end = line.find(marker, from);
                if (end > from) {
                    const int stop = end + int(marker.size());
                    add(spans, i, stop - i, strong ? SyntaxStyle::Strong : SyntaxStyle::Emphasis);
                    i = stop;
                    continue;
                }
            }
        }
        if (c == '[' || (c == '!' && i + 1 < size && text[size_t(i) + 1] == '[')) {
            static const RegExp link("!?\\[[^\\]]*\\](\\([^)]*\\)|\\[[^\\]]*\\])");
            const RegExp::Match match = link.match(text, size_t(i), true);
            if (match.hasMatch()) {
                add(spans, i, int(match.length()), SyntaxStyle::Link);
                i += int(match.length());
                continue;
            }
        }
        if (c == '<') {
            static const RegExp autolink("<(https?://|mailto:)[^>\\s]+>");
            const RegExp::Match match = autolink.match(text, size_t(i), true);
            if (match.hasMatch()) {
                add(spans, i, int(match.length()), SyntaxStyle::Link);
                i += int(match.length());
                continue;
            }
        }
        i = line.next(i);
    }
    return 0;
}

// States: 1 inside a comment, 2 inside a tag, 3 and 4 inside a "..." or
// '...' attribute value of a tag, 5 inside CDATA.
static Syntax::State highlightMarkup(std::string_view text, Syntax::State state, std::vector<SyntaxSpan> *spans)
{
    const Line line(text);
    const int size = line.size;
    int i = 0;
    while (i < size) {
        if (state == 1 || state == 5) {
            const std::string_view close = state == 1 ? "-->" : "]]>";
            const int end = line.find(close, i);
            const int stop = end < 0 ? size : end + int(close.size());
            add(spans, i, stop - i, state == 1 ? SyntaxStyle::Comment : SyntaxStyle::Code);
            if (end < 0)
                return state;
            i = stop;
            state = 0;
            continue;
        }
        if (state == 3 || state == 4) {
            const int end = line.find(state == 3 ? '"' : '\'', i);
            const int stop = end < 0 ? size : end + 1;
            add(spans, i, stop - i, SyntaxStyle::String);
            if (end < 0)
                return state;
            i = stop;
            state = 2;
            continue;
        }
        const char32_t c = line.at(i);
        if (state == 2) {
            if (c == '>' || ((c == '/' || c == '?') && i + 1 < size && text[size_t(i) + 1] == '>')) {
                const int length = c == '>' ? 1 : 2;
                add(spans, i, length, SyntaxStyle::Tag);
                i += length;
                state = 0;
            } else if (c == '"' || c == '\'') {
                state = c == '"' ? 3 : 4;
                add(spans, i, 1, SyntaxStyle::String);
                ++i;
            } else if (isWordStart(c) || c == ':') {
                int end = line.next(i);
                while (end < size && (isWordChar(line.at(end)) || text[size_t(end)] == '-' || text[size_t(end)] == ':'
                                      || text[size_t(end)] == '.'))
                    end = line.next(end);
                add(spans, i, end - i, SyntaxStyle::Attribute);
                i = end;
            } else {
                i = line.next(i);
            }
            continue;
        }

        if (line.startsWith(i, "<!--")) {
            state = 1;
            add(spans, i, 4, SyntaxStyle::Comment);
            i += 4;
        } else if (line.startsWith(i, "<![CDATA[")) {
            state = 5;
            add(spans, i, 9, SyntaxStyle::Code);
            i += 9;
        } else if (c == '<' && i + 1 < size
                   && (isWordStart(line.at(i + 1)) || text[size_t(i) + 1] == '/' || text[size_t(i) + 1] == '!'
                       || text[size_t(i) + 1] == '?')) {
            int end = i + 1;
            while (end < size) {
                const char32_t e = line.at(end);
                if (!isWordChar(e) && e != '/' && e != '!' && e != '?' && e != '-' && e != ':' && e != '.')
                    break;
                end = line.next(end);
            }
            add(spans, i, end - i, SyntaxStyle::Tag);
            i = end;
            state = 2;
        } else if (c == '&') {
            const int end = line.find(';', i);
            // Within twelve characters, as counted before in UTF-16.
            if (end > i && Unicode::utf16Length(text.substr(size_t(i), size_t(end - i))) < 12) {
                add(spans, i, end - i + 1, SyntaxStyle::Escape);
                i = end + 1;
            } else {
                ++i;
            }
        } else {
            i = line.next(i);
        }
    }
    return state;
}

static Syntax::State highlightIni(const Language &l, std::string_view text, Syntax::State state, std::vector<SyntaxSpan> *spans)
{
    const Line line(text);
    const int size = line.size;
    if (state >= multilineBase)
        return highlightCode(l, text, state, spans);

    const int i = line.skipSpace(0);
    if (i >= size)
        return 0;
    const char first = text[size_t(i)];
    if (first == '#' || first == ';') {
        add(spans, i, size - i, SyntaxStyle::Comment);
        return 0;
    }
    if (first == '[') {
        const size_t end = text.rfind(']');
        add(spans, i, end != std::string_view::npos && int(end) > i ? int(end) - i + 1 : size - i, SyntaxStyle::Section);
        return 0;
    }
    int equals = line.find('=', i);
    if (equals < 0 && l.id == "ini")
        equals = line.find(':', i);
    if (equals > i) {
        add(spans, i, equals - i, SyntaxStyle::Key);
        // The value: TOML values are typed, INI values are text.
        if (l.id == "toml") {
            std::vector<SyntaxSpan> valueSpans;
            const Syntax::State end = highlightCode(l, text.substr(size_t(equals) + 1), 0, &valueSpans);
            for (const SyntaxSpan &span : valueSpans)
                add(spans, span.start + equals + 1, span.length, span.style);
            return end;
        }
        const std::string_view value = strings::trimmed(text.substr(size_t(equals) + 1));
        const int at = int(text.rfind(value));
        if (l.constants.find(strings::toAsciiLower(value)) != l.constants.end()) {
            add(spans, at, int(value.size()), SyntaxStyle::Constant);
        } else if (!value.empty() && (strings::isAsciiDigit(value[0]) || value[0] == '-')) {
            if (strings::toDouble(value))
                add(spans, at, int(value.size()), SyntaxStyle::Number);
        }
    }
    return 0;
}

static Syntax::State highlightYaml(const Language &l, std::string_view text, std::vector<SyntaxSpan> *spans)
{
    if (strings::startsWith(text, "---") || strings::startsWith(text, "...")) {
        add(spans, 0, int(text.size()), SyntaxStyle::Preprocessor);
        return 0;
    }
    const Line line(text);
    const int i = line.skipSpace(0);
    if (i < line.size && text[size_t(i)] == '-' && (i + 1 >= line.size || Unicode::isSpace(line.at(i + 1))))
        add(spans, i, 1, SyntaxStyle::Keyword);
    return highlightCode(l, text, 0, spans);
}

static void highlightDiff(std::string_view text, std::vector<SyntaxSpan> *spans)
{
    const int size = int(text.size());
    if (strings::startsWith(text, "+++") || strings::startsWith(text, "---") || strings::startsWith(text, "diff ")
            || strings::startsWith(text, "index ") || strings::startsWith(text, "Index: ")
            || strings::startsWith(text, "new file mode") || strings::startsWith(text, "deleted file mode")
            || strings::startsWith(text, "similarity index") || strings::startsWith(text, "rename "))
        add(spans, 0, size, SyntaxStyle::DiffHeader);
    else if (strings::startsWith(text, "@@"))
        add(spans, 0, size, SyntaxStyle::Function);
    else if (strings::startsWith(text, "+") || strings::startsWith(text, ">"))
        add(spans, 0, size, SyntaxStyle::Added);
    else if (strings::startsWith(text, "-") || strings::startsWith(text, "<"))
        add(spans, 0, size, SyntaxStyle::Removed);
}

static void highlightLog(std::string_view text, std::vector<SyntaxSpan> *spans)
{
    static const RegExp pattern(
            "(?<time>^\\S*\\d{2}:\\d{2}(:\\d{2})?\\S*|^\\w{3} [ \\d]\\d \\d{2}:\\d{2}:\\d{2}|^\\d{4}-\\d{2}-\\d{2}[T ][\\d:.,]+\\S*)"
            "|(?<error>\\b(ERROR|Error|error|FATAL|Fatal|CRITICAL|Critical|FAIL(ED)?|Failed|failed|EE)\\b)"
            "|(?<warning>\\b(WARN(ING)?|Warning|warning|WW)\\b)"
            "|(?<info>\\b(INFO|Info|NOTICE|DEBUG|Debug|TRACE|II)\\b)");
    static const int error = pattern.groupNumber("error");
    static const int warning = pattern.groupNumber("warning");
    static const int info = pattern.groupNumber("info");
    pattern.forEachMatch(text, 0, [&](const RegExp::Match &match) {
        SyntaxStyle style = SyntaxStyle::Comment;
        if (match.length(error) > 0)
            style = SyntaxStyle::Error;
        else if (match.length(warning) > 0)
            style = SyntaxStyle::Warning;
        else if (match.length(info) > 0)
            style = SyntaxStyle::Type;
        add(spans, int(match.start()), int(match.length()), style);
        return true;
    });
}

Syntax::State Syntax::highlight(const Language *language, std::string_view text, State state, std::vector<SyntaxSpan> *spans)
{
    if (!language)
        return 0;
    switch (language->kind) {
    case Language::Code:
    case Language::Json:
        return highlightCode(*language, text, state, spans);
    case Language::Markdown:
        return highlightMarkdown(text, state, spans);
    case Language::Markup:
        return highlightMarkup(text, state, spans);
    case Language::Ini:
        return highlightIni(*language, text, state, spans);
    case Language::Yaml:
        return highlightYaml(*language, text, spans);
    case Language::Diff:
        highlightDiff(text, spans);
        return 0;
    case Language::Log:
        highlightLog(text, spans);
        return 0;
    }
    return 0;
}
