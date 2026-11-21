// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model/pathhistory.h"

namespace fm {

void PathHistory::append(const std::string& url)
{
    // A new destination ends the forward branch, as in a browser; the back
    // list is bounded so a long-lived tab does not keep every step.
    m_post.clear();
    m_previous.push_back({url});
    while (m_previous.size() > kMaxEntries)
        m_previous.erase(m_previous.begin());
}

void PathHistory::replaceCurrent(const std::string& url)
{
    if (m_previous.empty()) {
        m_previous.push_back({url});
        return;
    }

    m_previous.back().url = url;
    if (m_previous.size() > 1 && m_previous[m_previous.size() - 2].url == url)
        m_previous.pop_back();
}

std::string PathHistory::posteriorPath()
{
    if (m_post.empty())
        return {};

    m_previous.push_back(std::move(m_post.back()));
    m_post.pop_back();
    return m_previous.back().url;
}

std::string PathHistory::previousPath()
{
    if (!canGoBack())
        return {};

    m_post.push_back(std::move(m_previous.back()));
    m_previous.pop_back();
    return m_previous.back().url;
}

void PathHistory::setCurrentPosition(double position)
{
    if (!m_previous.empty())
        m_previous.back().position = position;
}

std::vector<std::string> PathHistory::currentSelection() const
{
    return m_previous.empty() ? std::vector<std::string>() : m_previous.back().selection;
}

void PathHistory::setCurrentSelection(std::vector<std::string> selection)
{
    if (!m_previous.empty())
        m_previous.back().selection = std::move(selection);
}

std::vector<std::string> PathHistory::backward() const
{
    std::vector<std::string> urls;
    for (size_t i = 0; i + 1 < m_previous.size(); ++i)
        urls.push_back(m_previous[i].url);
    return urls;
}

std::vector<std::string> PathHistory::forward() const
{
    std::vector<std::string> urls;
    for (const Entry& entry : m_post)
        urls.push_back(entry.url);
    return urls;
}

void PathHistory::restore(const std::vector<std::string>& backward, const std::vector<std::string>& forward)
{
    const std::string current = last();
    m_previous.clear();
    for (const std::string& url : backward)
        m_previous.push_back({url});
    if (!current.empty())
        m_previous.push_back({current});
    m_post.clear();
    for (const std::string& url : forward)
        m_post.push_back({url});
}

} // namespace fm
