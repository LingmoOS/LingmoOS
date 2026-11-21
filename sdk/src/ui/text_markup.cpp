// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/text_markup.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <utility>

namespace Lingmo::ui {

namespace {

void appendUtf8(std::string &out, uint32_t code)
{
    if (code < 0x80) {
        out += char(code);
    } else if (code < 0x800) {
        out += char(0xc0 | (code >> 6));
        out += char(0x80 | (code & 0x3f));
    } else if (code < 0x10000) {
        out += char(0xe0 | (code >> 12));
        out += char(0x80 | ((code >> 6) & 0x3f));
        out += char(0x80 | (code & 0x3f));
    } else if (code < 0x110000) {
        out += char(0xf0 | (code >> 18));
        out += char(0x80 | ((code >> 12) & 0x3f));
        out += char(0x80 | ((code >> 6) & 0x3f));
        out += char(0x80 | (code & 0x3f));
    }
}

// The value of `name="..."` (or '...') in a tag's text; empty when absent.
std::string attribute(const std::string &tag, const std::string &name)
{
    std::string lower = tag;
    for (char &ch : lower)
        ch = char(std::tolower(static_cast<unsigned char>(ch)));
    size_t at = lower.find(name);
    while (at != std::string::npos) {
        size_t i = at + name.size();
        while (i < tag.size() && tag[i] == ' ')
            ++i;
        if (i < tag.size() && tag[i] == '=' && (at == 0 || !std::isalnum(static_cast<unsigned char>(lower[at - 1])))) {
            ++i;
            while (i < tag.size() && tag[i] == ' ')
                ++i;
            if (i < tag.size() && (tag[i] == '"' || tag[i] == '\'')) {
                const char quote = tag[i];
                const size_t end = tag.find(quote, i + 1);
                return tag.substr(i + 1, end == std::string::npos ? std::string::npos : end - i - 1);
            }
            const size_t end = tag.find_first_of(" \t", i);
            return tag.substr(i, end == std::string::npos ? std::string::npos : end - i);
        }
        at = lower.find(name, at + 1);
    }
    return {};
}

bool isHeading(const std::string &name)
{
    return name.size() == 2 && name[0] == 'h' && name[1] >= '1' && name[1] <= '6';
}

} // namespace

std::vector<Label::Span> styledTextSpans(const std::string &markup, Color linkColor)
{
    std::vector<Label::Span> spans;
    int bold = 0;
    int italic = 0;
    int underline = 0;
    // The open links' targets, innermost last.
    std::vector<std::string> links;
    std::string text;

    const auto flush = [&] {
        if (text.empty())
            return;
        Label::Span span;
        span.text = std::move(text);
        span.weight = bold > 0 ? 700 : 0;
        span.italic = italic > 0;
        span.underline = underline > 0;
        if (!links.empty()) {
            span.color = linkColor;
            span.link = links.back();
        }
        spans.push_back(std::move(span));
        text.clear();
    };

    // A block tag's line break, without leading with one.
    const auto lineBreak = [&] {
        if (!text.empty()) {
            if (text.back() != '\n')
                text += '\n';
        } else if (!spans.empty() && !spans.back().text.empty() && spans.back().text.back() != '\n') {
            spans.back().text += '\n';
        }
    };

    size_t i = 0;
    while (i < markup.size()) {
        const char c = markup[i];
        if (c == '<') {
            const size_t end = markup.find('>', i);
            if (end == std::string::npos) {
                text += markup.substr(i);
                break;
            }
            std::string tag = markup.substr(i + 1, end - i - 1);
            i = end + 1;
            const bool closing = !tag.empty() && tag[0] == '/';
            if (closing)
                tag.erase(0, 1);
            const size_t nameEnd = tag.find_first_of(" \t\n/");
            std::string name = tag.substr(0, nameEnd);
            for (char &ch : name)
                ch = char(std::tolower(static_cast<unsigned char>(ch)));

            if (name == "b" || name == "strong") {
                flush();
                bold += closing ? (bold > 0 ? -1 : 0) : 1;
            } else if (name == "i" || name == "em") {
                flush();
                italic += closing ? (italic > 0 ? -1 : 0) : 1;
            } else if (name == "u") {
                flush();
                underline += closing ? (underline > 0 ? -1 : 0) : 1;
            } else if (name == "a") {
                flush();
                if (closing) {
                    if (!links.empty())
                        links.pop_back();
                } else {
                    links.push_back(attribute(tag, "href"));
                }
            } else if (name == "br") {
                text += '\n';
            } else if (name == "p" || name == "div") {
                flush();
                lineBreak();
            } else if (isHeading(name)) {
                flush();
                if (closing) {
                    bold = std::max(0, bold - 1);
                    lineBreak();
                } else {
                    lineBreak();
                    ++bold;
                }
            } else if (name == "ul" || name == "ol") {
                flush();
                lineBreak();
            } else if (name == "li") {
                flush();
                lineBreak();
                if (!closing)
                    text += "• ";
            }
            continue;
        }

        if (c == '&') {
            const size_t end = markup.find(';', i);
            if (end != std::string::npos && end - i <= 10) {
                const std::string entity = markup.substr(i + 1, end - i - 1);
                bool known = true;
                if (entity == "lt")
                    text += '<';
                else if (entity == "gt")
                    text += '>';
                else if (entity == "amp")
                    text += '&';
                else if (entity == "quot")
                    text += '"';
                else if (entity == "apos")
                    text += '\'';
                else if (entity == "nbsp")
                    appendUtf8(text, 0xa0);
                else if (entity.size() > 1 && entity[0] == '#') {
                    const bool hex = entity[1] == 'x' || entity[1] == 'X';
                    char *last = nullptr;
                    const unsigned long code = std::strtoul(entity.c_str() + (hex ? 2 : 1), &last, hex ? 16 : 10);
                    if (last && *last == '\0' && code > 0)
                        appendUtf8(text, uint32_t(code));
                    else
                        known = false;
                } else {
                    known = false;
                }
                if (known) {
                    i = end + 1;
                    continue;
                }
            }
        }

        text += c;
        ++i;
    }

    flush();
    return spans;
}

} // namespace Lingmo::ui
