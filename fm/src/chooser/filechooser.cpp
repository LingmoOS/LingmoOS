// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chooser/filechooser.h"
#include "core/preferences.h"
#include "core/standardfolders.h"

#include "base/i18n.h"
#include "base/mime_database.h"
#include "base/url.h"

#include <fnmatch.h>
#include <set>
#include <sys/stat.h>
#include <unistd.h>

using Lingmo::i18n::tr;
using Lingmo::i18n::trn;

namespace fm {

static bool isDirectory(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

static bool exists(const std::string& path)
{
    struct stat st;
    return ::lstat(path.c_str(), &st) == 0;
}

static std::string joinPath(const std::string& folder, const std::string& name)
{
    return folder == "/" ? "/" + name : folder + "/" + name;
}

bool matchesFilter(const base::json::Value& filter, const std::string& name)
{
    const base::json::Value::Array& rules = filter["rules"].toArray();
    for (const base::json::Value& rule : rules) {
        const std::string value = rule["value"].toString();
        const int64_t type = rule["type"].toInt();
        if (type == 0) {
            // Apps list "*.jpg" and expect camera files named "IMG_0001.JPG" to show up.
            if (::fnmatch(value.c_str(), name.c_str(), FNM_CASEFOLD) == 0)
                return true;
        } else if (type == 1) {
            const std::string mime = base::mime::typeForFileName(name);
            if (base::mime::inherits(mime, value)
                || (value.ends_with("/*") && mime.starts_with(value.substr(0, value.size() - 1))))
                return true;
        }
    }
    return rules.empty();
}

FileChooser::FileChooser(base::json::Value request)
    : m_request(std::move(request))
    , m_folder(std::make_unique<FolderModel>(nullptr))
{
    FolderModel& folder = *m_folder;
    const Preferences& preferences = Preferences::instance();
    folder.setSortMode(preferences.sortMode());
    folder.setSortDesc(preferences.sortDesc());
    m_subscriptions.push_back(Preferences::instance().changed.connect([this] {
        m_folder->setSortMode(Preferences::instance().sortMode());
        m_folder->setSortDesc(Preferences::instance().sortDesc());
    }));
    m_subscriptions.push_back(folder.selectionChanged.connect([this] {
        // Picking a file names the save after it.
        const std::vector<int> rows = m_folder->selectedRows();
        if (!rows.empty()) {
            if (saveMode() && rows.size() == 1 && !m_folder->item(rows.front())->isDir)
                setFileName(m_folder->item(rows.front())->name);
            if (!m_error.empty())
                setError({});
        }
        selectionChanged.emit();
    }));
    m_subscriptions.push_back(folder.urlChanged.connect([this] {
        m_folder->clearSelection();
        m_overwritePath.clear();
        locationChanged.emit();
    }));
    m_subscriptions.push_back(folder.historyChanged.connect([this] { locationChanged.emit(); }));
    m_subscriptions.push_back(folder.notification.connect([this](const std::string& message) { setError(message); }));

    const int filterCount = int(filters().size());
    m_filterIndex = std::clamp(int(m_request["filter_index"].toInt()), 0, std::max(0, filterCount - 1));
    for (const base::json::Value& choice : choices()) {
        std::string selected = choice["selected"].toString();
        if (selected.empty()) {
            const base::json::Value::Array& options = choice["options"].toArray();
            selected = options.empty() ? std::string("false") : options.front()["id"].toString();
        }
        m_choices.insert(choice["id"].toString(), selected);
    }
    applyFilter();

    std::string location = m_request["current_folder"].toString();
    m_fileName = m_request["current_name"].toString();
    const std::string currentFile = m_request["current_file"].toString();
    if (saveMode() && currentFile.starts_with('/')) {
        location = parentPath(currentFile);
        m_fileName = fm::fileName(currentFile);
    }
    if (!location.starts_with('/') || !isDirectory(location))
        location = homePath();
    navigate(location);
}

FileChooser::~FileChooser() = default;

bool FileChooser::isValidRequest(const base::json::Value& request)
{
    const std::string mode = request["mode"].toString();
    return mode == "OpenFile" || mode == "SaveFile" || mode == "SaveFiles";
}

std::string FileChooser::title() const
{
    return m_request["title"].toString(saveMode() ? std::string(tr("Save File")) : std::string(tr("Open")));
}

std::string FileChooser::acceptLabel() const
{
    std::string label = m_request["accept_label"].toString();
    if (label.empty()) {
        if (m_request["mode"].toString().starts_with("Save"))
            return tr("Save");
        return directoryMode() ? tr("Select Folder") : tr("Open");
    }
    std::erase(label, '_');
    return label;
}

bool FileChooser::saveMode() const
{
    return m_request["mode"].toString() == "SaveFile";
}

bool FileChooser::directoryMode() const
{
    return m_request["mode"].toString() == "SaveFiles" || (!saveMode() && m_request["directory"].toBool());
}

bool FileChooser::multiple() const
{
    return m_request["mode"].toString() == "OpenFile" && m_request["multiple"].toBool();
}

std::string FileChooser::directory() const
{
    return base::url::toLocalFile(m_folder->url()).value_or(m_folder->url());
}

void FileChooser::setFileName(const std::string& name)
{
    if (m_fileName == name)
        return;
    m_fileName = name;
    m_overwritePath.clear();
    fileNameChanged.emit();
}

std::string FileChooser::selectionSummary() const
{
    const std::vector<std::string> urls = m_folder->selectedUrls();
    if (urls.size() == 1)
        return fm::fileName(base::url::toLocalFile(urls.front()).value_or(urls.front()));
    if (urls.empty())
        return directoryMode() ? directory() : std::string(tr("Select a file"));
    return trn("{n} item selected", "{n} items selected", urls.size());
}

const base::json::Value::Array& FileChooser::filters() const
{
    return m_request["filters"].toArray();
}

const base::json::Value::Array& FileChooser::choices() const
{
    return m_request["choices"].toArray();
}

void FileChooser::applyFilter()
{
    const bool directories = directoryMode();
    const base::json::Value::Array& all = filters();
    std::optional<base::json::Value> filter;
    if (!all.empty())
        filter = all[size_t(m_filterIndex)];
    m_folder->contents().setFilter([directories, filter](const FileItem& item) {
        if (item.isDir)
            return true;
        if (directories)
            return false;
        return !filter || matchesFilter(*filter, item.name);
    });
}

void FileChooser::setFilterIndex(int index)
{
    if (index == m_filterIndex || index < 0 || index >= int(filters().size()))
        return;
    m_filterIndex = index;
    m_folder->clearSelection();
    applyFilter();
}

void FileChooser::setShowHidden(bool show)
{
    m_folder->setShowHiddenFiles(show);
}

void FileChooser::navigate(const std::string& input)
{
    std::string path = base::url::toLocalFile(input).value_or(input);
    if (path == "~" || path.starts_with("~/"))
        path = homePath() + path.substr(1);
    if (!path.starts_with('/'))
        path = joinPath(directory(), path);
    path = cleanPath(path);
    if (!isDirectory(path) || ::access(path.c_str(), R_OK | X_OK) != 0) {
        setError(tr("This folder is unavailable or cannot be read."));
        return;
    }
    setError({});
    m_folder->clearSelection();
    m_folder->setUrl(base::url::fromLocalFile(path));
}

void FileChooser::up()
{
    navigate(parentPath(directory()));
}

void FileChooser::back()
{
    m_folder->goBack();
}

void FileChooser::forward()
{
    m_folder->goForward();
}

void FileChooser::activate(int row)
{
    const FileItemPtr item = m_folder->item(row);
    if (!item)
        return;
    if (item->isDir) {
        navigate(item->path);
    } else if (!directoryMode()) {
        m_folder->clearSelection();
        m_folder->setSelected(row);
        accept();
    }
}

void FileChooser::selectAll()
{
    if (!multiple())
        return;
    m_folder->clearSelection();
    std::vector<int> rows;
    for (int row = 0; row < m_folder->count(); ++row) {
        if (directoryMode() || !m_folder->item(row)->isDir)
            rows.push_back(row);
    }
    m_folder->updateSelection(rows, false, false);
}

void FileChooser::setChoice(const std::string& id, const std::string& value)
{
    m_choices.insert(id, value);
}

bool FileChooser::validName(const std::string& name) const
{
    return !name.empty() && name != "." && name != ".." && name.find('/') == std::string::npos
            && name.find('\0') == std::string::npos;
}

void FileChooser::newFolder(const std::string& name)
{
    const std::string path = joinPath(directory(), name);
    if (!validName(name) || ::mkdir(path.c_str(), 0777) != 0) {
        setError(tr("The folder could not be created. Choose another name."));
        return;
    }
    navigate(path);
}

void FileChooser::accept(bool overwrite)
{
    if (m_finished)
        return;
    setError({});
    std::vector<std::string> paths;
    if (saveMode()) {
        if (!validName(m_fileName)) {
            setError(tr("Enter a file name without slashes."));
            return;
        }
        const std::string target = joinPath(directory(), m_fileName);
        if (isDirectory(target)) {
            navigate(target);
            return;
        }
        const bool targetExists = exists(target);
        if (::access(directory().c_str(), W_OK) != 0 || (targetExists && ::access(target.c_str(), W_OK) != 0)) {
            setError(tr("You do not have permission to save here."));
            return;
        }
        if (targetExists && (!overwrite || m_overwritePath != target)) {
            m_overwritePath = target;
            confirmOverwrite.emit(m_fileName);
            return;
        }
        paths.push_back(target);
    } else {
        const std::vector<std::string> selected = m_folder->selectedUrls();
        for (const std::string& url : selected) {
            const auto local = base::url::toLocalFile(url);
            if (!local) {
                setError(tr("Choose a local file or folder."));
                return;
            }
            if (!exists(*local) || ::access(local->c_str(), R_OK) != 0) {
                setError(tr("The selected item is no longer available."));
                return;
            }
            const bool folder = isDirectory(*local);
            if (folder != directoryMode()) {
                if (!directoryMode() && selected.size() == 1)
                    navigate(*local);
                else
                    setError(directoryMode() ? tr("Select folders only.") : tr("Select files only."));
                return;
            }
            paths.push_back(cleanPath(*local));
        }
        if (paths.empty() && directoryMode())
            paths.push_back(directory());
        if (paths.empty()) {
            setError(tr("Select a file to open."));
            return;
        }
        if (!multiple() && paths.size() > 1)
            return;
    }
    if (m_request["mode"].toString() == "SaveFiles") {
        const std::string folder = paths.front();
        if (::access(folder.c_str(), W_OK) != 0) {
            setError(tr("You do not have permission to save here."));
            return;
        }
        paths.clear();
        std::set<std::string> used;
        for (const base::json::Value& entry : m_request["files"].toArray()) {
            const std::string name = entry.toString();
            if (!validName(name)) {
                setError(tr("A suggested file name is invalid."));
                return;
            }
            std::string candidate = joinPath(folder, name);
            int suffix = 1;
            // The extension follows the last dot, unless the dot leads the name.
            const size_t dot = name.rfind('.');
            const bool hasExtension = dot != std::string::npos && dot > 0 && dot + 1 < name.size();
            const std::string stem = hasExtension ? name.substr(0, dot) : name;
            const std::string extension = hasExtension ? name.substr(dot) : std::string();
            while (exists(candidate) || used.contains(candidate))
                candidate = joinPath(folder, stem + " (" + std::to_string(suffix++) + ")" + extension);
            used.insert(candidate);
            paths.push_back(candidate);
        }
        if (paths.empty()) {
            setError(tr("No file names were provided."));
            return;
        }
    }
    complete(paths);
}

void FileChooser::complete(const std::vector<std::string>& paths)
{
    base::json::Value::Array uris;
    for (const std::string& path : paths)
        uris.push_back(base::url::fromLocalFile(path));
    m_finished = true;
    base::json::Value result = base::json::Value::object();
    result.insert("response", 0);
    result.insert("uris", base::json::Value(std::move(uris)));
    result.insert("filter_index", m_filterIndex);
    result.insert("choices", m_choices);
    finished.emit(result);
}

void FileChooser::cancel()
{
    if (m_finished)
        return;
    m_finished = true;
    base::json::Value result = base::json::Value::object();
    result.insert("response", 1);
    finished.emit(result);
}

void FileChooser::setError(const std::string& message)
{
    m_error = message;
    errorChanged.emit();
}

} // namespace fm
