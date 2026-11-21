// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSION_H
#define SESSION_H

#include <set>
#include <string>
#include <vector>

struct TabState
{
    std::string path;
    std::string draftId;
    // The text in the draft is the document's, not the file's.
    bool unsaved = false;
    int line = 0;
    // A UTF-8 byte offset into the line.
    int column = 0;
    double scroll = 0;
    std::string encoding;
    std::string language;
};

struct WindowState
{
    std::vector<TabState> tabs;
    int current = 0;
};

// The open windows and tabs, kept in session.json next to the drafts, so
// they come back after a restart or a crash.
namespace Session
{
// The folder session.json and the drafts live in:
// ~/.local/share/lingmo-text-editor, or another one set for tests.
std::string dataDirectory();
void setDataDirectory(const std::string &directory);
std::string defaultPath();

bool save(const std::vector<WindowState> &windows, const std::string &path = defaultPath());
// `ok` is false when the file exists but cannot be read or parsed.
std::vector<WindowState> load(const std::string &path = defaultPath(), bool *ok = nullptr);
void remove(const std::string &path = defaultPath());

// Deletes drafts no tab refers to any more, once they are older than
// `minimumAgeDays`: a draft of a closed tab may still be wanted. Unfinished
// temporary drafts from before this process started go as well.
void removeOrphanDrafts(const std::string &directory, const std::set<std::string> &keep, int minimumAgeDays = 0);
}

#endif
