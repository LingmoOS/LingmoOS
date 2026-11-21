// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/dialogs.h"
#include "core/launcher.h"
#include "core/standardfolders.h"
#include "dialogs/iotaskdialog.h"
#include "dialogs/openwithdialog.h"
#include "dialogs/propertiesdialog.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/url.h"
#include "base/mime_database.h"
#include "ui/application.h"
#include "ui/dialog_window.h"

#include <algorithm>

using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace fm {

Dialogs::Dialogs()
{
    m_taskAdded = OperationsClient::instance().taskAdded.connect([this](OperationTaskPtr task) {
        // Not inside the announcement, which a folder model may be handling as well.
        base::EventLoop::current()->post([this, task] { ioTask(task); });
    });
    m_languageChanged = ui::Application::instance()->languageChanged.connect([this] { retranslate(); });
}

Dialogs::~Dialogs() = default;

// Every dialog takes its text again; a question stays open, its answer pending.
void Dialogs::retranslate()
{
    for (const auto& [dialog, parent] : m_dialogs)
        dialog->retranslate();
    for (const Confirm& confirm : m_confirms)
        confirm.texts(*confirm.dialog);
}

void Dialogs::ioTask(const OperationTaskPtr& task)
{
    // A quick task announced by another process can be over already: its
    // Finished came right behind TaskAdded. Only a failure has something to show.
    if (!task || (task->isFinished() && (!task->error() || task->wasCanceled())))
        return;
    auto dialog = std::make_unique<IoTaskDialog>(task);
    IoTaskDialog* raw = dialog.get();
    raw->onDone = [this, raw] { remove(raw); };
    add(std::move(dialog), nullptr);
}

void Dialogs::confirmDelete(const std::vector<std::string>& urls, ui::Window* parent)
{
    if (urls.empty())
        return;
    confirm(
            parent,
            [](ui::ConfirmDialog& question) {
                question.setTitle(tr("Delete"));
                question.setText(tr("Do you want to delete it permanently?"));
                question.setConfirmText(tr("Delete"));
            },
            true, [urls] { OperationsClient::instance().remove(urls); });
}

void Dialogs::confirmEmptyTrash(ui::Window* parent)
{
    ui::ConfirmDialog* dialog = confirm(
            parent,
            [](ui::ConfirmDialog& question) {
                question.setTitle(tr("Empty Trash"));
                question.setText(tr("Do you want to permanently delete all files from the Trash?"));
                question.setConfirmText(tr("Empty Trash"));
            },
            true, [] { OperationsClient::instance().emptyTrash(); });
    // Opened from the dock or the desktop it has no window of the file
    // manager's to belong to: it stays off the task list and the menu bar,
    // and above other windows.
    if (ui::Window* window = dialog->window()) {
        window->setSkipTaskbar(true);
        window->setAlwaysOnTop(true);
    }
}

void Dialogs::confirmLaunch(const std::string& path, ui::Window* parent)
{
    if (path.empty())
        return;
    const std::string absolute = cleanPath(path);
    confirm(
            parent,
            [absolute](ui::ConfirmDialog& question) {
                question.setTitle(tr("Run Program"));
                question.setText(format(tr("“{name}” is a program. Only run it if you trust where it came from."),
                                        {{"name", fileName(absolute)}}));
                question.setConfirmText(tr("Run"));
            },
            false, [absolute] { launcher::trustAndLaunch(absolute); });
}

void Dialogs::properties(const FileItemList& items, ui::Window* parent)
{
    if (items.empty())
        return;
    auto dialog = std::make_unique<PropertiesDialog>(items);
    PropertiesDialog* raw = dialog.get();
    raw->onDone = [this, raw] { remove(raw); };
    raw->setParentWindow(parent);
    add(std::move(dialog), parent);
    raw->show();
}

void Dialogs::openWith(const std::string& url, ui::Window* parent)
{
    if (url.empty())
        return;
    const std::string path = base::url::toLocalFile(url).value_or(url);
    auto dialog = std::make_unique<OpenWithDialog>(path, base::mime::typeForFile(path));
    OpenWithDialog* raw = dialog.get();
    raw->onDone = [this, raw] { remove(raw); };
    raw->setParentWindow(parent, true);
    add(std::move(dialog), parent);
    raw->show();
}

void Dialogs::closeOwnedBy(ui::Window* window)
{
    if (!window)
        return;
    std::vector<std::shared_ptr<void>> gone;
    for (auto it = m_dialogs.begin(); it != m_dialogs.end();) {
        if (it->second == window) {
            it->first->hide();
            gone.push_back(std::shared_ptr<FramelessDialog>(std::move(it->first)));
            it = m_dialogs.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = m_confirms.begin(); it != m_confirms.end();) {
        if (it->parent == window) {
            gone.push_back(std::shared_ptr<ui::ConfirmDialog>(std::move(it->dialog)));
            it = m_confirms.erase(it);
        } else {
            ++it;
        }
    }
    if (gone.empty())
        return;
    base::EventLoop::current()->post([gone] {});
    if (onChanged)
        onChanged();
}

void Dialogs::add(std::unique_ptr<FramelessDialog> dialog, ui::Window* parent)
{
    m_dialogs.emplace_back(std::move(dialog), parent);
    if (onChanged)
        onChanged();
}

void Dialogs::remove(FramelessDialog* dialog)
{
    auto it = std::find_if(m_dialogs.begin(), m_dialogs.end(),
                           [dialog](const auto& entry) { return entry.first.get() == dialog; });
    if (it == m_dialogs.end())
        return;
    // Called from the dialog's own handlers: it goes after them.
    std::shared_ptr<FramelessDialog> gone(std::move(it->first));
    m_dialogs.erase(it);
    base::EventLoop::current()->post([gone] {});
    if (onChanged)
        onChanged();
}

ui::ConfirmDialog* Dialogs::confirm(ui::Window* parent, std::function<void(ui::ConfirmDialog&)> texts,
                                    bool destructive, std::function<void()> accepted)
{
    auto dialog = std::make_unique<ui::ConfirmDialog>(parent, std::string());
    ui::ConfirmDialog* raw = dialog.get();
    texts(*raw);
    raw->setDestructive(destructive);
    raw->onAccepted = [this, raw, accepted = std::move(accepted)] {
        accepted();
        removeConfirm(raw);
    };
    raw->onRejected = [this, raw] { removeConfirm(raw); };
    m_confirms.push_back({std::move(dialog), parent, std::move(texts)});
    raw->show();
    if (onChanged)
        onChanged();
    return raw;
}

void Dialogs::removeConfirm(ui::ConfirmDialog* dialog)
{
    auto it = std::find_if(m_confirms.begin(), m_confirms.end(),
                           [dialog](const Confirm& confirm) { return confirm.dialog.get() == dialog; });
    if (it == m_confirms.end())
        return;
    std::shared_ptr<ui::ConfirmDialog> gone(std::move(it->dialog));
    m_confirms.erase(it);
    base::EventLoop::current()->post([gone] {});
    if (onChanged)
        onChanged();
}

} // namespace fm
