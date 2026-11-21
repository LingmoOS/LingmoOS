// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"
#include "model/foldermodel.h"

#include "base/json.h"
#include "base/signal.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fm {

// One portal file dialog's state over a folder model. The request and the
// result are JSON objects.
class FileChooser
{
public:
    explicit FileChooser(base::json::Value request);
    ~FileChooser();

    // Whether the request names a mode this dialog serves.
    static bool isValidRequest(const base::json::Value& request);

    const base::json::Value& request() const { return m_request; }
    FolderModel& folder() { return *m_folder; }
    std::string title() const;
    std::string acceptLabel() const;
    bool saveMode() const;
    bool directoryMode() const;
    bool multiple() const;
    // The folder shown, a local path.
    std::string directory() const;
    const std::string& fileName() const { return m_fileName; }
    void setFileName(const std::string& name);
    std::string selectionSummary() const;
    const std::string& error() const { return m_error; }
    const base::json::Value::Array& filters() const;
    const base::json::Value::Array& choices() const;
    int filterIndex() const { return m_filterIndex; }
    void setFilterIndex(int index);
    bool showHidden() const { return m_folder->showHiddenFiles(); }
    void setShowHidden(bool show);

    void navigate(const std::string& path);
    void up();
    void back();
    void forward();
    // The view's row was activated: a folder opens, a file is chosen.
    void activate(int row);
    void selectAll();
    void setChoice(const std::string& id, const std::string& value);
    void newFolder(const std::string& name);
    void accept(bool overwrite = false);
    void cancel();

    base::Signal<> locationChanged;
    base::Signal<> fileNameChanged;
    base::Signal<> selectionChanged;
    base::Signal<> errorChanged;
    base::Signal<std::string> confirmOverwrite;
    base::Signal<const base::json::Value&> finished;

private:
    void setError(const std::string& message);
    void complete(const std::vector<std::string>& paths);
    bool validName(const std::string& name) const;
    void applyFilter();

    base::json::Value m_request;
    std::unique_ptr<FolderModel> m_folder;
    std::string m_fileName;
    std::string m_error;
    std::string m_overwritePath;
    base::json::Value m_choices = base::json::Value::object();
    int m_filterIndex = 0;
    bool m_finished = false;
    std::vector<base::Subscription> m_subscriptions;
};

// Whether a file name matches a portal filter's rules: glob (type 0, any
// case, '/' matched like any character) or MIME type (type 1, by name).
bool matchesFilter(const base::json::Value& filter, const std::string& name);

} // namespace fm
