// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#define PCRE2_CODE_UNIT_WIDTH 16

#include "RegExp.h"

#include <pcre2.h>

namespace Konsole
{

struct RegExp::Compiled
{
    pcre2_code_16* code = nullptr;

    ~Compiled()
    {
        pcre2_code_free_16(code);
    }
};

// As Qt grows the JIT stack once a match runs out of it.
static thread_local pcre2_jit_stack_16* jitStack = nullptr;

static pcre2_jit_stack_16* jitStackCallback(void*)
{
    return jitStack;
}

RegExp::RegExp() = default;

RegExp::RegExp(const std::u16string& pattern, int options)
    : m_pattern(pattern)
    , m_options(options)
{
    uint32_t flags = PCRE2_UTF;
    if (options & CaseInsensitive)
        flags |= PCRE2_CASELESS;

    int error = 0;
    PCRE2_SIZE errorOffset = 0;
    pcre2_code_16* code = pcre2_compile_16(reinterpret_cast<PCRE2_SPTR16>(pattern.data()), pattern.size(), flags,
                                           &error, &errorOffset, nullptr);
    if (!code)
        return;

    pcre2_jit_compile_16(code, PCRE2_JIT_COMPLETE | PCRE2_JIT_PARTIAL_SOFT | PCRE2_JIT_PARTIAL_HARD);
    m_compiled = std::make_shared<Compiled>();
    m_compiled->code = code;
}

RegExp::~RegExp() = default;
RegExp::RegExp(const RegExp& other) = default;
RegExp& RegExp::operator=(const RegExp& other) = default;

bool RegExp::isValid() const
{
    return m_compiled != nullptr;
}

std::u16string RegExp::Match::captured(const std::u16string& subject, int group) const
{
    const long start = capturedStart(group);
    const long end = capturedEnd(group);
    if (start < 0 || end < start)
        return std::u16string();
    return subject.substr(size_t(start), size_t(end - start));
}

RegExp::Match RegExp::doMatch(const std::u16string& subject, long offset, uint32_t flags) const
{
    Match result;
    if (!m_compiled)
        return result;

    const long length = long(subject.size());
    pcre2_match_context_16* context = pcre2_match_context_create_16(nullptr);
    pcre2_jit_stack_assign_16(context, &jitStackCallback, nullptr);
    pcre2_match_data_16* data = pcre2_match_data_create_from_pattern_16(m_compiled->code, nullptr);

    static const char16_t dummy = 0;
    const char16_t* text = subject.empty() ? &dummy : subject.data();

    int status = pcre2_match_16(m_compiled->code, reinterpret_cast<PCRE2_SPTR16>(text), size_t(length),
                                size_t(offset), flags, data, context);
    if (status == PCRE2_ERROR_JIT_STACKLIMIT && !jitStack)
    {
        jitStack = pcre2_jit_stack_create_16(32 * 1024, 512 * 1024, nullptr);
        status = pcre2_match_16(m_compiled->code, reinterpret_cast<PCRE2_SPTR16>(text), size_t(length),
                                size_t(offset), flags, data, context);
    }

    if (status > 0)
    {
        const PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_16(data);
        result.hasMatch = true;
        result.offsets.resize(size_t(status) * 2);
        for (int i = 0; i < status * 2; ++i)
            result.offsets[i] = ovector[i] == PCRE2_UNSET ? -1 : long(ovector[i]);
    }

    pcre2_match_data_free_16(data);
    pcre2_match_context_free_16(context);
    return result;
}

RegExp::Match RegExp::match(const std::u16string& subject, long offset, bool anchored) const
{
    const long length = long(subject.size());
    if (offset < 0)
        offset += length;
    if (offset < 0 || offset > length)
        return Match();

    return doMatch(subject, offset, anchored ? PCRE2_ANCHORED : 0);
}

RegExp::Match RegExp::nextMatch(const std::u16string& subject, const Match& previous) const
{
    if (!previous.hasMatch || !m_compiled)
        return Match();

    long offset = previous.capturedEnd();
    const long length = long(subject.size());
    if (offset < 0 || offset > length)
        return Match();

    if (previous.capturedStart() != previous.capturedEnd())
        return doMatch(subject, offset, PCRE2_NO_UTF_CHECK);

    // After an empty match: a non-empty one at the same place, else one
    // starting a character later.
    Match result = doMatch(subject, offset, PCRE2_NO_UTF_CHECK | PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED);
    if (result.hasMatch)
        return result;

    uint32_t newline = 0;
    pcre2_pattern_info_16(m_compiled->code, PCRE2_INFO_NEWLINE, &newline);
    const bool crlf = newline == PCRE2_NEWLINE_CRLF || newline == PCRE2_NEWLINE_ANY || newline == PCRE2_NEWLINE_ANYCRLF;

    ++offset;
    if (crlf && offset < length && subject[offset - 1] == u'\r' && subject[offset] == u'\n')
        ++offset;
    else if (offset < length && (subject[offset] & 0xfc00) == 0xdc00)
        ++offset;

    return doMatch(subject, offset, PCRE2_NO_UTF_CHECK);
}

long RegExp::lastIndexIn(const std::u16string& subject, long from, Match* match) const
{
    if (!m_compiled)
        return -1;

    const long endpos = from < 0 ? long(subject.size()) + from + 1 : from + 1;
    long lastIndex = -1;
    Match current = this->match(subject);
    while (current.hasMatch)
    {
        const long start = current.capturedStart();
        if (start >= endpos)
            break;
        lastIndex = start;
        if (match)
            *match = current;
        current = nextMatch(subject, current);
    }
    return lastIndex;
}

std::u16string RegExp::escape(const std::u16string& text)
{
    std::u16string result;
    result.reserve(text.size() * 2);
    for (size_t i = 0; i < text.size(); ++i)
    {
        const char16_t current = text[i];
        if (current == 0)
        {
            result += u"\\0";
        }
        else if ((current < u'a' || current > u'z') && (current < u'A' || current > u'Z')
                 && (current < u'0' || current > u'9') && current != u'_')
        {
            result.push_back(u'\\');
            result.push_back(current);
            if (current >= 0xd800 && current <= 0xdbff && i + 1 < text.size())
                result.push_back(text[++i]);
        }
        else
        {
            result.push_back(current);
        }
    }
    return result;
}

}
