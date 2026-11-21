// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "urls.h"

#include "base/strings.h"
#include "base/url.h"

#include <algorithm>
#include <filesystem>
#include <string_view>

using namespace Lingmo;
namespace strings = base::strings;

struct FullParts : urls::Parts
{
    std::string userInfo;
};

static bool isHexDigit(char c)
{
    return strings::isAsciiDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

// Typed text made a URL tolerantly: bytes a URL cannot hold
// escaped, a stray "%" too, everything else left as it is.
static std::string tolerantEncoded(std::string_view text)
{
    static constexpr std::string_view kEscaped = " \"<>\\^`{|}";
    static constexpr char kDigits[] = "0123456789ABCDEF";
    std::string result;
    result.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        const auto byte = static_cast<unsigned char>(text[i]);
        const bool stray = byte == '%' && !(i + 2 < text.size() && isHexDigit(text[i + 1]) && isHexDigit(text[i + 2]));
        if (byte < 0x21 || byte >= 0x7f || kEscaped.find(char(byte)) != std::string_view::npos || stray) {
            result += '%';
            result += kDigits[byte >> 4];
            result += kDigits[byte & 15];
        } else {
            result += char(byte);
        }
    }
    return result;
}

static std::optional<FullParts> parseFull(std::string_view url)
{
    FullParts parts;
    std::string_view rest = url;
    const size_t colon = url.find(':');
    if (colon != std::string_view::npos && colon > 0 && strings::isAsciiAlpha(url[0])) {
        bool scheme = true;
        for (size_t i = 1; i < colon; ++i) {
            const char c = url[i];
            if (!strings::isAsciiAlpha(c) && !strings::isAsciiDigit(c) && c != '+' && c != '-' && c != '.')
                scheme = false;
        }
        if (scheme) {
            parts.scheme = strings::toAsciiLower(url.substr(0, colon));
            rest = url.substr(colon + 1);
        }
    }

    const size_t hash = rest.find('#');
    if (hash != std::string_view::npos) {
        parts.fragment = rest.substr(hash + 1);
        rest = rest.substr(0, hash);
    }
    const size_t question = rest.find('?');
    if (question != std::string_view::npos) {
        parts.query = rest.substr(question + 1);
        rest = rest.substr(0, question);
    }

    if (rest.starts_with("//")) {
        const size_t end = rest.find('/', 2);
        const std::string_view authority = rest.substr(2, end == std::string_view::npos ? std::string_view::npos : end - 2);
        parts.authority = std::string(authority);
        rest = end == std::string_view::npos ? std::string_view() : rest.substr(end);

        std::string_view hostPort = authority;
        const size_t at = authority.rfind('@');
        if (at != std::string_view::npos) {
            parts.userInfo = authority.substr(0, at);
            hostPort = authority.substr(at + 1);
        }
        std::string_view portText;
        if (hostPort.starts_with('[')) {
            const size_t close = hostPort.find(']');
            if (close == std::string_view::npos)
                return std::nullopt;
            parts.host = strings::toAsciiLower(hostPort.substr(0, close + 1));
            const std::string_view after = hostPort.substr(close + 1);
            if (!after.empty()) {
                if (after[0] != ':')
                    return std::nullopt;
                portText = after.substr(1);
            }
        } else {
            const size_t portColon = hostPort.rfind(':');
            parts.host = strings::toAsciiLower(hostPort.substr(0, portColon));
            if (portColon != std::string_view::npos)
                portText = hostPort.substr(portColon + 1);
        }
        if (!portText.empty()) {
            const auto port = strings::toNumber<int>(portText);
            if (!port || *port < 0 || *port > 65535
                    || !std::all_of(portText.begin(), portText.end(), strings::isAsciiDigit))
                return std::nullopt;
            parts.port = *port;
        }
        for (const char c : parts.host) {
            if (c == '%' || c == '/' || c == '@' || (c == ':' && !parts.host.starts_with('[')))
                return std::nullopt;
        }
    }
    parts.path = rest;
    return parts;
}

static std::string build(const FullParts& parts)
{
    std::string url;
    if (!parts.scheme.empty())
        url = parts.scheme + ':';
    if (parts.authority) {
        url += "//";
        if (!parts.userInfo.empty())
            url += parts.userInfo + '@';
        url += parts.host;
        if (parts.port && *parts.port >= 0)
            url += ':' + std::to_string(*parts.port);
    }
    url += parts.path;
    if (!parts.query.empty())
        url += '?' + parts.query;
    if (!parts.fragment.empty())
        url += '#' + parts.fragment;
    return url;
}

static std::string fromTypedUrl(std::string_view trimmed)
{
    if (trimmed.starts_with('/'))
        return urls::fromLocalFile(trimmed);

    const std::string encoded = tolerantEncoded(trimmed);
    const auto url = parseFull(encoded);
    const auto prepended = parseFull("http://" + encoded);
    // host:port would otherwise read as a scheme.
    const bool prependedPort = prepended && prepended->port && *prepended->port >= 0;
    if (url && !url->scheme.empty() && !prependedPort)
        return build(*url);
    if (prepended && (!prepended->host.empty() || !prepended->path.empty())) {
        FullParts adjusted = *prepended;
        const size_t dot = trimmed.find('.');
        if (strings::toAsciiLower(trimmed.substr(0, dot)) == "ftp")
            adjusted.scheme = "ftp";
        return build(adjusted);
    }
    return {};
}

namespace urls
{

std::optional<Parts> parse(std::string_view url)
{
    auto parts = parseFull(url);
    if (!parts)
        return std::nullopt;
    return Parts(*parts);
}

std::string fromUserInput(std::string_view input, const std::string& workingDirectory, bool assumeLocalFile)
{
    const std::string_view trimmed = strings::trimmed(input);
    if (trimmed.empty())
        return {};
    if (!workingDirectory.empty()) {
        const std::filesystem::path typed{std::string(input)};
        const std::filesystem::path file = (std::filesystem::path(workingDirectory) / typed).lexically_normal();
        std::error_code error;
        if (std::filesystem::exists(file, error))
            return fromLocalFile(file.string());
        const auto url = parseFull(tolerantEncoded(trimmed));
        if (assumeLocalFile && url && url->scheme.empty() && !typed.is_absolute())
            return fromLocalFile(file.string());
    }
    return fromTypedUrl(trimmed);
}

bool isLocalFile(std::string_view url)
{
    return base::url::isLocalFile(url);
}

std::string toLocalFile(std::string_view url)
{
    return base::url::toLocalFile(url).value_or(std::string());
}

std::string fromLocalFile(std::string_view path)
{
    return base::url::fromLocalFile(path);
}

std::string scheme(std::string_view url)
{
    const auto parts = parseFull(url);
    return parts ? parts->scheme : std::string();
}

std::string host(std::string_view url)
{
    const auto parts = parseFull(url);
    return parts ? base::url::percentDecoded(parts->host) : std::string();
}

std::string path(std::string_view url)
{
    const auto parts = parseFull(url);
    return parts ? base::url::percentDecoded(parts->path) : std::string();
}

std::string fileName(std::string_view url)
{
    const std::string decoded = path(url);
    const size_t slash = decoded.rfind('/');
    return slash == std::string::npos ? decoded : decoded.substr(slash + 1);
}

std::string displayString(std::string_view url)
{
    auto parts = parseFull(url);
    if (!parts)
        return std::string(url);
    const size_t colon = parts->userInfo.find(':');
    if (colon != std::string::npos)
        parts->userInfo.resize(colon);
    return base::url::percentDecoded(build(*parts));
}

std::vector<std::string> fromUriList(std::string_view list)
{
    std::vector<std::string> result;
    for (const std::string_view line : strings::splitViews(list, '\n')) {
        const std::string_view url = strings::trimmed(line);
        if (!url.empty() && !url.starts_with('#'))
            result.emplace_back(url);
    }
    return result;
}

} // namespace urls
