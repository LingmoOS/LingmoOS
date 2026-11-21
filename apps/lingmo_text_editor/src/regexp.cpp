// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "regexp.h"
#include "unicode.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

struct RegExp::Code
{
    ~Code()
    {
        if (matchData)
            pcre2_match_data_free(matchData);
        if (code)
            pcre2_code_free(code);
    }

    pcre2_code *code = nullptr;
    pcre2_match_data *matchData = nullptr;
};

std::string_view RegExp::Match::captured(std::string_view subject, int group) const
{
    if (start(group) < 0)
        return {};
    return subject.substr(size_t(start(group)), size_t(length(group)));
}

RegExp::RegExp(std::string_view pattern, unsigned options)
    : m_pattern(pattern)
{
    uint32_t flags = PCRE2_UTF;
    if (options & CaseInsensitive)
        flags |= PCRE2_CASELESS;
    if (options & UnicodeProperties)
        flags |= PCRE2_UCP;
    int errorCode = 0;
    PCRE2_SIZE errorOffset = 0;
    pcre2_code *code = pcre2_compile(reinterpret_cast<PCRE2_SPTR>(pattern.data()), pattern.size(), flags, &errorCode,
                                     &errorOffset, nullptr);
    if (!code) {
        PCRE2_UCHAR buffer[256];
        const int length = pcre2_get_error_message(errorCode, buffer, sizeof(buffer));
        m_error = length > 0 ? std::string(reinterpret_cast<const char *>(buffer), size_t(length)) : std::string("error");
        return;
    }
    pcre2_jit_compile(code, PCRE2_JIT_COMPLETE);
    m_code = std::make_shared<Code>();
    m_code->code = code;
    m_code->matchData = pcre2_match_data_create_from_pattern(code, nullptr);
}

int RegExp::groupNumber(std::string_view name) const
{
    if (!m_code)
        return -1;
    const std::string terminated(name);
    const int number = pcre2_substring_number_from_name(m_code->code, reinterpret_cast<PCRE2_SPTR>(terminated.c_str()));
    return number < 0 ? -1 : number;
}

RegExp::Match RegExp::matchWith(std::string_view subject, size_t offset, unsigned options) const
{
    Match result;
    if (!m_code || offset > subject.size())
        return result;
    // The subject is valid UTF-8 by the editor's rules.
    const int count = pcre2_match(m_code->code, reinterpret_cast<PCRE2_SPTR>(subject.data()), subject.size(), offset,
                                  options | PCRE2_NO_UTF_CHECK, m_code->matchData, nullptr);
    if (count < 0)
        return result;
    const PCRE2_SIZE *vector = pcre2_get_ovector_pointer(m_code->matchData);
    uint32_t groups = 0;
    pcre2_pattern_info(m_code->code, PCRE2_INFO_CAPTURECOUNT, &groups);
    result.starts.resize(groups + 1, -1);
    result.ends.resize(groups + 1, -1);
    for (uint32_t i = 0; i <= groups && int(i) < count; ++i) {
        if (vector[2 * i] == PCRE2_UNSET)
            continue;
        result.starts[i] = std::ptrdiff_t(vector[2 * i]);
        result.ends[i] = std::ptrdiff_t(vector[2 * i + 1]);
    }
    return result;
}

RegExp::Match RegExp::match(std::string_view subject, size_t offset, bool anchored, bool notEmpty) const
{
    return matchWith(subject, offset, (anchored ? PCRE2_ANCHORED : 0) | (notEmpty ? PCRE2_NOTEMPTY_ATSTART : 0));
}

void RegExp::forEachMatch(std::string_view subject, size_t offset, const std::function<bool(const Match &)> &callback) const
{
    if (!m_code)
        return;
    Match match = matchWith(subject, offset, 0);
    while (match.hasMatch()) {
        if (!callback(match))
            return;
        const size_t end = size_t(match.end());
        if (match.start() != match.end()) {
            match = matchWith(subject, end, 0);
            continue;
        }
        if (end >= subject.size())
            return;
        match = matchWith(subject, end, PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED);
        if (!match.hasMatch())
            match = matchWith(subject, Unicode::next(subject, end), 0);
    }
}

std::string RegExp::escape(std::string_view text)
{
    std::string result;
    result.reserve(text.size() * 2);
    for (const char c : text) {
        const auto byte = static_cast<unsigned char>(c);
        if (byte == 0) {
            result += "\\0";
            continue;
        }
        // Outside ASCII nothing is special; a backslash before it would still be literal.
        const bool word = byte >= 0x80 || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
                || c == '_';
        if (!word)
            result += '\\';
        result += c;
    }
    return result;
}
