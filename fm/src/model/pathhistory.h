// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cmath>
#include <string>
#include <vector>

namespace fm {

// A tab's back and forward history, with each folder's scroll offset and
// selection as it was left.
class PathHistory
{
public:
    void append(const std::string& url);
    void replaceCurrent(const std::string& url);
    std::string last() const { return m_previous.empty() ? std::string() : m_previous.back().url; }
    bool isEmpty() const { return m_previous.empty(); }
    bool canGoBack() const { return m_previous.size() >= 2; }
    bool canGoForward() const { return !m_post.empty(); }

    // Steps through the history and returns the folder now current, or an
    // empty string when there is nothing that way.
    std::string posteriorPath();
    std::string previousPath();

    // Where the view was scrolled in the current folder; NaN until it is left.
    double currentPosition() const { return m_previous.empty() ? NAN : m_previous.back().position; }
    void setCurrentPosition(double position);
    std::vector<std::string> currentSelection() const;
    void setCurrentSelection(std::vector<std::string> selection);

    // The folders before and after the current one, oldest first.
    std::vector<std::string> backward() const;
    std::vector<std::string> forward() const;
    // Puts them around the current folder, as a tab carried to another window had them.
    void restore(const std::vector<std::string>& backward, const std::vector<std::string>& forward);

private:
    struct Entry
    {
        std::string url;
        double position = NAN;
        std::vector<std::string> selection;
    };

    static constexpr size_t kMaxEntries = 200;
    std::vector<Entry> m_previous;
    // The newest step back is last, as it is taken first.
    std::vector<Entry> m_post;
};

} // namespace fm
