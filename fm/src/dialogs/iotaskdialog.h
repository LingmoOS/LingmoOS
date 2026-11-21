// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dialogs/framelessdialog.h"
#include "ops/operations.h"

#include "base/signal.h"
#include "base/timer.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
class CheckBox;
class DialogButton;
class Label;
class ProgressBar;
class ThemeIcon;
}

namespace fm {

// An item of the same name where one goes;
// Stop, Skip, Keep Both or Replace, for this one or all.
class ConflictDialog : public FramelessDialog
{
public:
    struct Details
    {
        std::string name;
        std::string iconName;
        bool exists = false;
        int64_t modified = 0;
    };

    ConflictDialog();

    void open(const std::string& operation, int sourceCount, const std::string& sourcePath,
              const std::string& destinationPath);
    void dismiss();
    // A question being asked stays, Apply to All as it was.
    void retranslate() override;

    // policy is a ConflictPolicy, or -1 to stop the whole task.
    std::function<void(int policy, bool applyToAll)> onResolved;

    static Details fileDetails(const std::string& path);

private:
    std::unique_ptr<ui::View> createContent();
    void answer(int policy);

    std::string m_operation;
    int m_sourceCount = 0;
    std::string m_destinationPath;
    Details m_existing;
    Details m_incoming;
    bool m_answered = true;
    ui::CheckBox* m_applyToAll = nullptr;
};

// A file operation's progress, shown after a
// short delay, its conflicts asked through ConflictDialog and its error
// kept until closed.
class IoTaskDialog : public FramelessDialog
{
public:
    explicit IoTaskDialog(OperationTaskPtr task);
    ~IoTaskDialog() override;

    void retranslate() override;

    // Done: closed after success, or the error dismissed.
    std::function<void()> onDone;

private:
    std::unique_ptr<ui::View> createContent();
    void update();
    void taskChanged();
    void finished();
    void askConflict(uint32_t conflict, const std::string& source, const std::string& destination);
    void resolve(int policy, bool applyToAll);
    void cancelTask();
    void close();
    std::string heading() const;
    std::string summary() const;
    std::string timeRemaining() const;
    double fraction() const;
    bool usesBytes() const;

    OperationTaskPtr m_task;
    std::string m_operation;
    std::string m_sourceName;
    std::string m_destinationName;
    int m_sourceCount = 0;
    bool m_finished = false;
    bool m_conflict = false;
    bool m_password = false;
    bool m_dismissed = false;
    bool m_canceling = false;
    std::string m_errorMessage;
    uint32_t m_conflictId = 0;
    std::string m_conflictPath;
    base::Timer m_delay;
    ConflictDialog m_conflictDialog;

    ui::Label* m_heading = nullptr;
    ui::Label* m_detail = nullptr;
    ui::ProgressBar* m_progress = nullptr;
    ui::Label* m_summary = nullptr;
    ui::DialogButton* m_button = nullptr;
    ui::ThemeIcon* m_icon = nullptr;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace fm
