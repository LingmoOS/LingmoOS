// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialogs/iotaskdialog.h"
#include "core/lister.h"
#include "core/standardfolders.h"

#include "base/i18n.h"
#include "base/locale.h"
#include "ui/check_box.h"
#include "ui/dialog_window.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/progress_bar.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <sys/stat.h>

using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;

namespace fm {

static ui::Label* addLabel(ui::View* parent, double pixelSize, int weight, ui::ColorId color, bool wrap)
{
    auto* label = parent->addChild<ui::Label>();
    label->setFontSize(pixelSize);
    label->setWeight(weight);
    label->setColor(color);
    label->setWrap(wrap);
    return label;
}

ConflictDialog::Details ConflictDialog::fileDetails(const std::string& path)
{
    Details details;
    details.name = fileName(path);
    struct stat st;
    details.exists = ::lstat(path.c_str(), &st) == 0;
    if (details.exists) {
        details.modified = int64_t(st.st_mtim.tv_sec) * 1000 + st.st_mtim.tv_nsec / 1000000;
        details.iconName = DirectoryLister::makeItem(path).iconName;
    }
    return details;
}

ConflictDialog::ConflictDialog()
    : FramelessDialog(tr("Replace Item"), true)
{
    setContentFactory([this] { return createContent(); }, 480, 20);
    onCloseRequested = [this] {
        // Dismissing is Stop: the task must not stay blocked on a question nobody sees.
        if (!m_answered)
            answer(-1);
    };
}

void ConflictDialog::open(const std::string& operation, int sourceCount, const std::string& sourcePath,
                          const std::string& destinationPath)
{
    m_operation = operation;
    m_sourceCount = sourceCount;
    m_destinationPath = destinationPath;
    m_existing = fileDetails(destinationPath);
    m_incoming = fileDetails(sourcePath);
    m_answered = false;
    hide();
    show();
}

void ConflictDialog::dismiss()
{
    m_answered = true;
    hide();
}

void ConflictDialog::retranslate()
{
    setTitle(tr("Replace Item"));
    if (!isShown())
        return;
    const bool all = m_applyToAll && m_applyToAll->isChecked();
    rebuildContent();
    m_applyToAll->setChecked(all);
}

void ConflictDialog::answer(int policy)
{
    m_answered = true;
    const bool all = m_applyToAll && m_applyToAll->isChecked();
    hide();
    if (onResolved)
        onResolved(policy, all);
}

std::unique_ptr<ui::View> ConflictDialog::createContent()
{
    const std::string itemName = !m_existing.name.empty() ? m_existing.name : fileName(m_destinationPath);
    int age = 0;
    if (m_existing.exists && m_incoming.exists && std::abs(m_existing.modified - m_incoming.modified) >= 1000)
        age = m_existing.modified < m_incoming.modified ? -1 : 1;

    std::string heading;
    switch (age) {
    case -1:
        heading = format(tr("An older item named “{name}” already exists in this location."), {{"name", itemName}});
        break;
    case 1:
        heading = format(tr("A newer item named “{name}” already exists in this location."), {{"name", itemName}});
        break;
    default:
        heading = format(tr("An item named “{name}” already exists in this location."), {{"name", itemName}});
        break;
    }
    std::string question;
    if (m_operation == "link")
        question = tr("Do you want to replace it with the link you’re creating?");
    else if (m_operation == "move")
        question = age < 0   ? tr("Do you want to replace it with the newer one you’re moving?")
                   : age > 0 ? tr("Do you want to replace it with the older one you’re moving?")
                             : tr("Do you want to replace it with the one you’re moving?");
    else if (m_operation == "restore")
        question = age < 0   ? tr("Do you want to replace it with the newer one you’re restoring?")
                   : age > 0 ? tr("Do you want to replace it with the older one you’re restoring?")
                             : tr("Do you want to replace it with the one you’re restoring?");
    else
        question = age < 0   ? tr("Do you want to replace it with the newer one you’re copying?")
                   : age > 0 ? tr("Do you want to replace it with the older one you’re copying?")
                             : tr("Do you want to replace it with the one you’re copying?");

    auto layout = std::make_unique<ui::ColumnLayout>();
    layout->setSpacing(20);
    auto* row = layout->addChild<ui::RowLayout>();
    row->setSpacing(16);
    row->setLayoutHints({.fillWidth = true});
    auto* icon = row->addChild<ui::ThemeIcon>(m_incoming.iconName.empty() ? "unknown" : m_incoming.iconName, 64);
    icon->setLayoutHints({.preferredWidth = 64, .preferredHeight = 64, .verticalAlignment = ui::Alignment::Start});
    auto* texts = row->addChild<ui::Column>();
    texts->setSpacing(4);
    ui::LayoutHints textHints {.fillWidth = true};
    textHints.margins.right = 32;
    texts->setLayoutHints(textHints);
    // The close button sits over the content: the text keeps 32 px clear of it.
    const double textWidth = 480 - 20 * 2 - 64 - 16 - 32;
    ui::Label* title = addLabel(texts, 13, 600, ui::ColorId::Text, true);
    title->setName("conflictHeading");
    title->setText(heading);
    title->setLayoutHints({.preferredWidth = textWidth});
    ui::Label* body = addLabel(texts, 12, 400, ui::ColorId::DisabledText, true);
    body->setText(question);
    body->setLayoutHints({.preferredWidth = textWidth});

    auto* actions = layout->addChild<ui::RowLayout>();
    actions->setSpacing(8);
    actions->setLayoutHints({.fillWidth = true});
    m_applyToAll = actions->addChild<ui::CheckBox>(tr("Apply to All"));
    m_applyToAll->setPadding({});
    m_applyToAll->setVisible(m_sourceCount > 1);
    actions->addChild<ui::View>()->setLayoutHints({.fillWidth = true});
    const auto button = [&](const std::string& text, int policy, bool primary) {
        auto* item = actions->addChild<ui::DialogButton>(text, primary);
        if (primary)
            item->setDestructive(true);
        item->onClicked = [this, policy] { answer(policy); };
        return item;
    };
    button(tr("Stop"), -1, false);
    button(tr("Skip"), 1, false)->setVisible(m_sourceCount > 1);
    button(tr("Keep Both"), 2, false);
    button(tr("Replace"), 3, true);
    return layout;
}

IoTaskDialog::IoTaskDialog(OperationTaskPtr task)
    : FramelessDialog(std::string(), false)
    , m_task(std::move(task))
{
    m_operation = m_task->operationName();
    m_sourceName = m_task->sourceName();
    m_sourceCount = m_task->sourceCount();
    m_destinationName = m_task->destinationName();

    setContentFactory([this] { return createContent(); }, 432, 16);
    // Closing does not cancel: the dialog only goes out of sight, and comes
    // back for a later conflict or error.
    onCloseRequested = [this] { close(); };

    m_conflictDialog.onResolved = [this](int policy, bool all) {
        if (m_conflict)
            resolve(policy, all);
    };

    m_delay.setSingleShot(true);
    // An archive job is asked for on its own, so its window answers at once
    // instead of waiting to see whether the job is quick.
    const bool archive = m_operation == "compress" || m_operation == "extract";
    m_delay.setInterval(std::chrono::milliseconds(archive ? 0 : m_operation == "emptyTrash" ? 300 : 750));
    m_delay.onTimeout = [this] {
        if (!m_finished && !m_conflict && !m_password && !m_dismissed)
            show();
    };

    m_subscriptions.push_back(m_task->conflictRequested.connect(
            [this](uint32_t conflict, const std::string& source, const std::string& destination) {
                askConflict(conflict, source, destination);
            }));
    m_subscriptions.push_back(m_task->changed.connect([this] { taskChanged(); }));
    m_subscriptions.push_back(m_task->finished.connect([this] { finished(); }));

    if (m_task->isFinished()) {
        m_finished = true;
        if (m_task->error() && !m_task->wasCanceled()) {
            m_errorMessage = m_task->errorString();
            show();
        }
    } else if (m_task->waitingForConflict()) {
        askConflict(m_task->conflictId(), m_task->conflictSource(), m_task->conflictPath());
    } else if (m_task->waitingForPassword()) {
        m_password = true;
    } else {
        m_delay.start();
    }
}

IoTaskDialog::~IoTaskDialog() = default;

void IoTaskDialog::retranslate()
{
    update();
    m_conflictDialog.retranslate();
}

void IoTaskDialog::taskChanged()
{
    // A remote task may announce its conflict through its snapshot before,
    // or instead of, the signal.
    if (m_task->waitingForConflict() && !m_conflict && !m_finished)
        askConflict(m_task->conflictId(), m_task->conflictSource(), m_task->conflictPath());
    // The archive's password dialog stands in for the progress window, as
    // the conflict dialog does.
    const bool password = m_task->waitingForPassword() && !m_finished;
    if (password != m_password) {
        m_password = password;
        if (password) {
            m_delay.stop();
            hide();
        } else if (!m_conflict) {
            m_delay.start();
        }
    }
    m_operation = m_task->operationName().empty() ? m_operation : m_task->operationName();
    update();
}

void IoTaskDialog::finished()
{
    m_delay.stop();
    m_finished = true;
    m_conflict = false;
    m_conflictDialog.dismiss();
    if (m_task->error() && !m_task->wasCanceled()) {
        m_errorMessage = m_task->errorString();
        show();
        update();
    } else {
        hide();
        if (onDone)
            onDone();
    }
}

void IoTaskDialog::close()
{
    if (!m_finished) {
        m_dismissed = true;
        hide();
        return;
    }
    hide();
    if (onDone)
        onDone();
}

void IoTaskDialog::cancelTask()
{
    if (m_finished || m_canceling)
        return;
    m_canceling = true;
    m_conflict = false;
    m_conflictDialog.dismiss();
    m_task->cancel();
    update();
}

// The conflict dialog stands in for the progress window, which comes back
// through the usual delay once the task runs again.
void IoTaskDialog::askConflict(uint32_t conflict, const std::string& source, const std::string& destination)
{
    m_conflictId = conflict;
    m_conflictPath = destination;
    m_conflict = true;
    m_delay.stop();
    hide();
    m_conflictDialog.open(m_operation, m_sourceCount, source, destination);
}

void IoTaskDialog::resolve(int policy, bool applyToAll)
{
    m_conflict = false;
    if (policy < 0)
        cancelTask();
    else
        m_task->resolveConflict(m_conflictId, ConflictPolicy(policy), applyToAll);
    if (!m_finished)
        m_delay.start();
}

bool IoTaskDialog::usesBytes() const
{
    const TaskProgress progress = m_task->progress();
    static const std::vector<std::string> itemOperations = {"delete", "emptyTrash", "rename", "link"};
    return progress.totalBytes > 0
            && std::find(itemOperations.begin(), itemOperations.end(), m_operation) == itemOperations.end();
}

double IoTaskDialog::fraction() const
{
    const TaskProgress progress = m_task->progress();
    const double completed = usesBytes() ? double(progress.processedBytes) : double(progress.processedItems);
    const double total = usesBytes() ? double(progress.totalBytes) : double(progress.totalItems);
    return total > 0 ? std::min(1.0, completed / total) : 0;
}

std::string IoTaskDialog::heading() const
{
    const uint64_t count = uint64_t(std::max(0, m_sourceCount));
    const bool one = m_sourceCount == 1;
    if (m_operation == "copy")
        return one ? format(tr("Copying “{name}” to “{destination}”"),
                            {{"name", m_sourceName}, {"destination", m_destinationName}})
                   : format(trn("Copying {n} item to “{destination}”", "Copying {n} items to “{destination}”", count),
                            {{"destination", m_destinationName}});
    if (m_operation == "move")
        return one ? format(tr("Moving “{name}” to “{destination}”"),
                            {{"name", m_sourceName}, {"destination", m_destinationName}})
                   : format(trn("Moving {n} item to “{destination}”", "Moving {n} items to “{destination}”", count),
                            {{"destination", m_destinationName}});
    if (m_operation == "trash")
        return one ? format(tr("Moving “{name}” to Trash"), {{"name", m_sourceName}})
                   : trn("Moving {n} item to Trash", "Moving {n} items to Trash", count);
    if (m_operation == "restore")
        return one ? format(tr("Restoring “{name}”"), {{"name", m_sourceName}})
                   : trn("Restoring {n} item", "Restoring {n} items", count);
    if (m_operation == "delete")
        return one ? format(tr("Deleting “{name}”"), {{"name", m_sourceName}})
                   : trn("Deleting {n} item", "Deleting {n} items", count);
    if (m_operation == "emptyTrash")
        return tr("Emptying Trash");
    if (m_operation == "rename")
        return format(tr("Renaming “{name}” to “{destination}”"),
                      {{"name", m_sourceName}, {"destination", m_destinationName}});
    if (m_operation == "mkdir")
        return format(tr("Creating “{name}”"), {{"name", m_destinationName}});
    if (m_operation == "link")
        return format(tr("Creating links in “{destination}”"), {{"destination", m_destinationName}});
    if (m_operation == "compress")
        return format(tr("Compressing to “{destination}”"), {{"destination", m_destinationName}});
    if (m_operation == "extract")
        return format(tr("Extracting to “{destination}”"), {{"destination", m_destinationName}});
    return tr("Preparing…");
}

std::string IoTaskDialog::timeRemaining() const
{
    const TaskProgress progress = m_task->progress();
    const double done = fraction();
    if (progress.elapsedMs < 1000 || done <= 0 || done >= 1)
        return {};
    const int64_t seconds = std::max<int64_t>(1, std::llround(progress.elapsedMs / 1000.0 * (1 - done) / done));
    if (seconds < 60)
        return seconds == 1 ? std::string(tr("About a second remaining"))
                            : trn("About {n} second remaining", "About {n} seconds remaining", uint64_t(seconds));
    const int64_t minutes = (seconds + 59) / 60;
    return minutes == 1 ? std::string(tr("About a minute remaining"))
                        : trn("About {n} minute remaining", "About {n} minutes remaining", uint64_t(minutes));
}

// As OperationTask::formattedSize: one decimal, SI units.
static std::string formattedSize(uint64_t bytes)
{
    return base::Locale::system().formattedDataSize(int64_t(bytes), 1, base::Locale::DataSizeFormat::Si);
}

std::string IoTaskDialog::summary() const
{
    const TaskProgress progress = m_task->progress();
    if (m_canceling)
        return tr("Stopping…");
    if (progress.preparing)
        return tr("Preparing…");
    if (m_conflict)
        return tr("Waiting for your choice");
    const double total = usesBytes() ? double(progress.totalBytes) : double(progress.totalItems);
    if (total > 0 && fraction() >= 1 && !m_finished)
        return tr("Finishing…");
    const std::string current = !progress.currentPath.empty() ? fileName(progress.currentPath) : m_sourceName;
    if (!progress.totalsKnown)
        return current.empty() ? std::string(tr("Working…")) : format(tr("Working on “{name}”…"), {{"name", current}});
    const std::string amount = usesBytes()
            ? format(tr("{processed} of {total}"),
                     {{"processed", formattedSize(std::min(progress.processedBytes, progress.totalBytes))},
                      {"total", formattedSize(progress.totalBytes)}})
            : format(trn("{processed} of {n} item", "{processed} of {n} items", progress.totalItems),
                     {{"processed", std::to_string(std::min(progress.processedItems, progress.totalItems))}});
    const std::string remaining = timeRemaining();
    if (remaining.empty())
        return amount;
    // TRANSLATORS: progress and the time left, such as "3 of 10 items — About a minute remaining".
    return format(tr("{amount} — {remaining}"), {{"amount", amount}, {"remaining", remaining}});
}

std::unique_ptr<ui::View> IoTaskDialog::createContent()
{
    auto layout = std::make_unique<ui::ColumnLayout>();
    layout->setSpacing(16);
    auto* row = layout->addChild<ui::RowLayout>();
    row->setSpacing(14);
    row->setLayoutHints({.fillWidth = true});
    m_icon = row->addChild<ui::ThemeIcon>("folder", 48);
    m_icon->setLayoutHints({.preferredWidth = 48, .preferredHeight = 48, .verticalAlignment = ui::Alignment::Start});
    auto* column = row->addChild<ui::ColumnLayout>();
    column->setSpacing(4);
    column->setLayoutHints({.fillWidth = true});
    m_heading = addLabel(column, 14, 600, ui::ColorId::Text, true);
    m_heading->setName("ioOperationHeading");
    m_heading->setMaximumLineCount(2);
    m_heading->setLayoutHints({.fillWidth = true});
    m_detail = addLabel(column, 12, 400, ui::ColorId::DisabledText, false);
    m_detail->setLayoutHints({.fillWidth = true});
    m_progress = column->addChild<ui::ProgressBar>();
    m_progress->setName("ioProgressBar");
    ui::LayoutHints progressHints {.fillWidth = true};
    progressHints.margins.top = 4;
    m_progress->setLayoutHints(progressHints);
    m_summary = addLabel(column, 11, 400, ui::ColorId::DisabledText, true);
    m_summary->setName("ioProgressSummary");
    m_summary->setLayoutHints({.fillWidth = true});

    auto* actions = layout->addChild<ui::RowLayout>();
    actions->setSpacing(8);
    actions->setLayoutHints({.horizontalAlignment = ui::Alignment::End});
    m_button = actions->addChild<ui::DialogButton>(tr("Cancel"));
    m_button->setName("ioCancelButton");
    m_button->onClicked = [this] {
        if (m_finished)
            close();
        else
            cancelTask();
    };
    update();
    return layout;
}

void IoTaskDialog::update()
{
    if (!isShown() || !m_heading)
        return;
    const TaskProgress progress = m_task->progress();
    window()->setTitle(heading());
    m_icon->setIconName(m_operation == "trash" || m_operation == "emptyTrash" ? "user-trash" : "folder");
    m_heading->setText(heading());

    std::string detail;
    if (!m_errorMessage.empty())
        detail = m_errorMessage;
    else if (m_conflict)
        detail = format(tr("“{name}” already exists in this location."), {{"name", fileName(m_conflictPath)}});
    else
        detail = !progress.currentPath.empty() ? fileName(progress.currentPath) : m_sourceName;
    m_detail->setText(detail);
    m_detail->setVisible(!detail.empty());
    m_detail->setWrap(m_finished || m_conflict);
    m_detail->setMaximumLineCount(4);
    m_detail->setElide(ui::Label::Elide::Middle);

    m_progress->setVisible(!m_finished);
    m_progress->setValue(std::min(0.99, fraction()));
    m_progress->setIndeterminate(!m_canceling && !m_conflict && (progress.preparing || !progress.totalsKnown));
    m_summary->setVisible(!m_finished);
    m_summary->setText(summary());
    m_button->setText(m_finished ? tr("Close") : tr("Cancel"));
    m_button->setEnabled(m_finished || !m_canceling);
    fit();
}

} // namespace fm
