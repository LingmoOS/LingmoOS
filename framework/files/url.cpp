// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/url.h"

#include <functional>

#include "base/strings.h"
#include "base/url.h"

namespace Lingmo::Files {

using namespace Lingmo::Base;

// RFC 3986's path characters and the brackets, written without percent-encoding.
static constexpr std::string_view kPathKeep = "!$&'()*+,;=:@/[]";
static constexpr std::string_view kQueryKeep = "!$&'()*+,;=:@/?[]";

static bool isSchemeCharacter(char c, bool first)
{
    if (strings::isAsciiAlpha(c))
        return true;

    return !first && (strings::isAsciiDigit(c) || c == '+' || c == '-' || c == '.');
}

Url::Url(std::string_view text)
{
    if (text.empty())
        return;

    // The scheme, if the text starts with one.
    size_t colon = 0;
    while (colon < text.size() && isSchemeCharacter(text[colon], colon == 0))
        ++colon;

    if (colon > 0 && colon < text.size() && text[colon] == ':') {
        m_scheme = strings::toAsciiLower(text.substr(0, colon));
        text.remove_prefix(colon + 1);
    }

    if (const size_t hash = text.find('#'); hash != std::string_view::npos) {
        m_hasFragment = true;
        m_fragment = url::percentDecoded(text.substr(hash + 1));
        text = text.substr(0, hash);
    }

    if (const size_t question = text.find('?'); question != std::string_view::npos) {
        m_hasQuery = true;
        m_query = url::percentDecoded(text.substr(question + 1));
        text = text.substr(0, question);
    }

    if (text.starts_with("//")) {
        text.remove_prefix(2);
        m_hasAuthority = true;

        const size_t slash = text.find('/');
        std::string_view authority = text.substr(0, slash);
        text = slash == std::string_view::npos ? std::string_view() : text.substr(slash);

        if (const size_t at = authority.rfind('@'); at != std::string_view::npos) {
            m_userInfo = url::percentDecoded(authority.substr(0, at));
            authority.remove_prefix(at + 1);
        }

        // A port follows the last colon, unless that is inside an IPv6 literal.
        const size_t portColon = authority.rfind(':');
        if (portColon != std::string_view::npos && authority.find(']', portColon) == std::string_view::npos) {
            const std::string_view digits = authority.substr(portColon + 1);
            if (!digits.empty()) {
                const auto port = strings::toNumber<int>(digits);
                if (!port || *port < 0 || *port > 65535)
                    return;
                m_port = *port;
            }
            authority = authority.substr(0, portColon);
        }

        m_host = strings::toAsciiLower(url::percentDecoded(authority));
    }

    m_path = url::percentDecoded(text);
    m_valid = true;
}

Url Url::fromLocalFile(std::string_view path)
{
    Url result;
    result.m_scheme = "file";
    result.m_hasAuthority = true;
    result.m_path = std::string(path);
    result.m_valid = true;
    return result;
}

bool Url::isEmpty() const
{
    return m_scheme.empty() && !m_hasAuthority && m_path.empty() && !m_hasQuery && !m_hasFragment;
}

void Url::setScheme(std::string scheme)
{
    m_scheme = strings::toAsciiLower(scheme);
    m_valid = true;
}

void Url::setPath(std::string path)
{
    m_path = std::move(path);
    m_valid = true;
}

std::string Url::toLocalFile() const
{
    if (!isLocalFile())
        return {};

    if (!m_host.empty() && m_host != "localhost")
        return "//" + m_host + m_path;

    return m_path;
}

std::string Url::fileName() const
{
    const size_t slash = m_path.rfind('/');
    return slash == std::string::npos ? m_path : m_path.substr(slash + 1);
}

Url Url::strippedTrailingSlash() const
{
    Url result = *this;
    while (result.m_path.size() > 1 && result.m_path.back() == '/')
        result.m_path.pop_back();
    return result;
}

Url Url::folder() const
{
    Url result = *this;
    const size_t slash = result.m_path.rfind('/');
    if (slash != std::string::npos)
        result.m_path.erase(slash + 1);
    else
        result.m_path.clear();
    return result.strippedTrailingSlash();
}

std::string Url::toString() const
{
    if (!m_valid)
        return {};

    std::string result;
    if (!m_scheme.empty())
        result += m_scheme + ':';

    if (m_hasAuthority) {
        result += "//";
        if (!m_userInfo.empty())
            result += url::percentEncoded(m_userInfo, "!$&'()*+,;=:") + '@';
        result += url::percentEncoded(m_host, "!$&'()*+,;=:[]");
        if (m_port >= 0)
            result += ':' + strings::number(m_port);
    }

    result += url::percentEncoded(m_path, kPathKeep);

    if (m_hasQuery)
        result += '?' + url::percentEncoded(m_query, kQueryKeep);
    if (m_hasFragment)
        result += '#' + url::percentEncoded(m_fragment, kQueryKeep);

    return result;
}

bool Url::operator==(const Url& other) const
{
    return m_valid == other.m_valid && m_scheme == other.m_scheme && m_userInfo == other.m_userInfo
        && m_host == other.m_host && m_port == other.m_port && m_path == other.m_path
        && m_hasQuery == other.m_hasQuery && m_query == other.m_query && m_hasFragment == other.m_hasFragment
        && m_fragment == other.m_fragment;
}

Url upUrl(const Url& url)
{
    Url result = url.strippedTrailingSlash();
    const std::string& path = result.path();
    const size_t slash = path.rfind('/');

    if (slash == std::string::npos)
        result.setPath(".");
    else if (slash == 0)
        result.setPath("/");
    else
        result.setPath(path.substr(0, slash));

    return result;
}

size_t UrlHash::operator()(const Url& url) const
{
    const std::hash<std::string> hash;
    size_t seed = hash(url.path());
    seed ^= hash(url.scheme()) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
    seed ^= hash(url.host()) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
    return seed;
}

} // namespace Lingmo::Files
