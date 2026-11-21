// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "remoteurl.h"

#include "base/strings.h"
#include "base/url.h"

using namespace Lingmo::base;

RemoteUrl RemoteUrl::parse(std::string_view text)
{
    RemoteUrl url;
    text = strings::trimmed(text);

    const size_t colon = text.find(':');
    if (colon == std::string_view::npos || colon == 0)
        return url;
    for (char c : text.substr(0, colon)) {
        if (!strings::isAsciiAlpha(c) && !strings::isAsciiDigit(c) && c != '+' && c != '-' && c != '.')
            return url;
    }
    url.scheme = strings::toAsciiLower(text.substr(0, colon));

    std::string_view rest = text.substr(colon + 1);
    if (!rest.starts_with("//")) {
        url.path = url::percentDecoded(rest);
        return url;
    }
    rest.remove_prefix(2);

    const size_t authorityEnd = rest.find_first_of("/?#");
    std::string_view authority = rest.substr(0, authorityEnd);
    std::string_view path = authorityEnd == std::string_view::npos ? std::string_view() : rest.substr(authorityEnd);
    if (const size_t query = path.find_first_of("?#"); query != std::string_view::npos)
        path = path.substr(0, query);
    url.path = url::percentDecoded(path);

    if (const size_t at = authority.rfind('@'); at != std::string_view::npos) {
        const std::string_view userInfo = authority.substr(0, at);
        authority.remove_prefix(at + 1);
        const size_t separator = userInfo.find(':');
        url.user = url::percentDecoded(userInfo.substr(0, separator));
        if (separator != std::string_view::npos)
            url.password = url::percentDecoded(userInfo.substr(separator + 1));
    }

    std::string_view host = authority;
    std::string_view port;
    if (authority.starts_with('[')) {
        const size_t close = authority.find(']');
        host = authority.substr(1, close == std::string_view::npos ? std::string_view::npos : close - 1);
        if (close != std::string_view::npos && close + 1 < authority.size() && authority[close + 1] == ':')
            port = authority.substr(close + 2);
    } else if (const size_t separator = authority.rfind(':'); separator != std::string_view::npos) {
        host = authority.substr(0, separator);
        port = authority.substr(separator + 1);
    }

    url.host = strings::toAsciiLower(url::percentDecoded(host));
    if (!port.empty()) {
        const auto number = strings::toNumber<int>(port);
        if (!number || *number < 0 || *number > 65535)
            return RemoteUrl();
        url.port = *number;
    }

    return url;
}

std::string RemoteUrl::hostForUrl() const
{
    return host.find(':') != std::string::npos ? '[' + host + ']' : host;
}

std::string RemoteUrl::toString() const
{
    std::string result = scheme + "://";
    if (!user.empty())
        result += url::percentEncoded(user, "!$&'()*+,;=") + '@';
    result += hostForUrl();
    if (port >= 0)
        result += ':' + std::to_string(port);
    result += url::percentEncoded(path, "/!$&'()*+,;=:@");
    return result;
}
