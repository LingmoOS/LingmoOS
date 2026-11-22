// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialogs/propertiesdialog.h"
#include "core/standardfolders.h"

#include "base/i18n.h"
#include "base/locale.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/progress_bar.h"
#include "ui/segmented_control.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"

#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>

using Lingmo::i18n::tr;
using Lingmo::i18n::trn;
using Lingmo::i18n::trNoop;
using Lingmo::i18n::format;

namespace fm {

static constexpr double kSmallSpacing = 6;
static constexpr double kLargeSpacing = 12;
// Keep the two-page property form wide enough for labels and paths.  A zero
// width makes FramelessDialog rely on an implicit layout width, which is not
// defined for the nested segmented control and collapses the window.
static constexpr double kContentWidth = 520;
static constexpr double kPageWidth = 440;
static constexpr double kPageHeight = 400;

// "Fri Oct 16 09:30:00 2026", local time.
static std::string textDate(int64_t milliseconds)
{
    if (milliseconds <= 0)
        return {};
    const base::DateTime time = base::DateTime::fromSecondsSinceEpoch(milliseconds / 1000);
    return base::Locale("C").toString(time, "ddd MMM d HH:mm:ss yyyy");
}

// What a copy of everything would take: files' sizes, folders walked.
static uint64_t totalSize(const std::string& path, const std::atomic_bool& cancelled)
{
    struct stat st;
    if (::lstat(path.c_str(), &st) != 0)
        return 0;
    if (!S_ISDIR(st.st_mode))
        return uint64_t(st.st_size);
    uint64_t total = 0;
    if (DIR* directory = ::opendir(path.c_str())) {
        while (dirent* entry = ::readdir(directory)) {
            if (cancelled.load())
                break;
            const std::string name = entry->d_name;
            if (name == "." || name == "..")
                continue;
            total += totalSize(path + '/' + name, cancelled);
        }
        ::closedir(directory);
    }
    return total;
}

PropertiesDialog::PropertiesDialog(FileItemList items)
    : FramelessDialog(tr("Properties"), true)
    , m_items(std::move(items))
{
    const bool multiple = m_items.size() > 1;
    const FileItem& first = *m_items.front();
    if (!multiple) {
        m_fileName = first.name;
        m_iconName = first.isDir ? "folder" : first.iconName;
        m_mimeType = first.mimeType;
        m_bytes = first.size;
        m_size = formatSize(first.size);
        m_location = parentPath(first.path);
        m_created = textDate(first.created);
        m_modified = textDate(first.modified);
        m_accessed = textDate(first.accessed);
    } else {
        m_fileName = trn("{n} file", "{n} files", m_items.size());
        m_location = parentPath(first.path);
        m_iconName = "unknown";
    }

    std::vector<std::string> paths;
    for (const FileItemPtr& item : m_items)
        paths.push_back(item->path);
    m_sizeJob = base::ThreadPool::global().run(
            [paths, cancelled = m_cancelled] {
                uint64_t total = 0;
                for (const std::string& path : paths)
                    total += totalSize(path, *cancelled);
                return total;
            },
            [this](uint64_t total) {
                m_bytes = total;
                m_size = formatSize(total);
                if (m_sizeLabel) {
                    m_sizeLabel->setText(m_size);
                    fit();
                }
            });

    setHeader(40, 4);
    setPadding(kLargeSpacing * 1.5, kSmallSpacing, kLargeSpacing * 2 - kSmallSpacing);
    setContentFactory([this] { return createContent(); }, kContentWidth, 0);
    onCloseRequested = [this] {
        hide();
        if (onDone)
            onDone();
    };
}

PropertiesDialog::~PropertiesDialog()
{
    m_cancelled->store(true);
}

void PropertiesDialog::retranslate()
{
    setTitle(tr("Properties"));
    if (m_items.size() > 1)
        m_fileName = trn("{n} file", "{n} files", m_items.size());
    if (m_bytes)
        m_size = formatSize(*m_bytes);
    rebuildContent();
}

std::unique_ptr<ui::View> PropertiesDialog::createContent()
{
    auto layout = std::make_unique<ui::ColumnLayout>();
    layout->setSpacing(kLargeSpacing);
    m_pages = layout->addChild<ui::SegmentedControl>(
            std::vector<std::string>{tr("General"), tr("Permissions")}, ui::SegmentedControl::Style::Segmented);
    m_pages->setLayoutHints({.fillWidth = true});
    m_pages->setCurrentIndex(m_page);
    m_pages->onCurrentChanged = [this](int index) { setPage(index); };
    // The active page must participate in the dialog's intrinsic height.
    // A plain View reports zero height, which used to crop the property rows.
    m_pageHost = layout->addChild<ui::ColumnLayout>();
    m_pageHost->setLayoutHints({.fillWidth = true, .preferredHeight = kPageHeight});
    setPage(m_page);
    return layout;
}

void PropertiesDialog::setPage(int index)
{
    m_page = std::clamp(index, 0, 1);
    if (!m_pageHost)
        return;
    m_sizeLabel = nullptr;
    m_storageBar = nullptr;
    std::shared_ptr<ui::View> old;
    for (const auto& child : m_pageHost->children()) {
        old = m_pageHost->takeChild(child.get());
        break;
    }
    if (old)
        base::EventLoop::current()->post([old] {});
    std::unique_ptr<ui::View> page = m_page == 0 ? createGeneralContent() : createPermissionsContent();
    page->setLayoutHints({.preferredWidth = kPageWidth, .horizontalAlignment = ui::Alignment::Center});
    m_pageHost->addChild(std::move(page));
    fit();
}

std::unique_ptr<ui::View> PropertiesDialog::createGeneralContent()
{
    auto layout = std::make_unique<ui::ColumnLayout>();
    layout->setSpacing(kLargeSpacing);

    struct Row
    {
        std::string label;
        std::string value;
        bool wrap = false;
        bool size = false;
    };
    std::vector<Row> rows = {
        {trNoop("Type:"), m_mimeType},
        {trNoop("Location:"), m_location, true},
        {trNoop("File system:"), m_items.front()->filesystemType},
        {trNoop("Mounted at:"), m_items.front()->mountPoint, true},
        {trNoop("Size:"), m_size.empty() ? std::string(tr("Calculating...")) : m_size, false, true},
        {trNoop("Created:"), m_created},
        {trNoop("Modified:"), m_modified},
        {trNoop("Accessed:"), m_accessed},
    };

    // Hidden rows leave the grid, so the label column is measured over all.
    double labelWidth = 48;
    for (const Row& row : rows) {
        ui::Label probe(tr(row.label.c_str()));
        if (window())
            probe.setTextLayoutMode(window()->textLayoutMode());
        labelWidth = std::max(labelWidth, probe.sizeHint().width);
    }

    // The icon ends where the labels do and the name starts where the values do.
    auto* header = layout->addChild<ui::RowLayout>();
    header->setSpacing(kLargeSpacing);
    auto* iconBox = header->addChild<ui::View>();
    iconBox->setLayoutHints({.preferredWidth = labelWidth, .preferredHeight = 48});
    auto* icon = iconBox->addChild<ui::ThemeIcon>(m_iconName, 48);
    icon->setGeometry({std::ceil(labelWidth) - 48, 0, 48, 48});
    auto* name = header->addChild<ui::Label>(m_fileName);
    name->setElide(ui::Label::Elide::Middle);
    name->setLayoutHints({.fillWidth = true});

    auto* form = layout->addChild<ui::GridLayout>(2);
    form->setColumnSpacing(kLargeSpacing);
    form->setRowSpacing(kLargeSpacing);
    form->setLayoutHints({.fillWidth = true, .verticalAlignment = ui::Alignment::Start});
    for (const Row& row : rows) {
        // A row without a value is left out; location and size always show.
        const bool shown = !row.value.empty() || row.label == "Location:" || row.size;
        if (!shown)
            continue;
        auto* label = form->addChild<ui::Label>(tr(row.label.c_str()));
        label->setColor(ui::ColorId::DisabledText);
        label->setTextAlignment(ui::Label::TextAlignment::Right);
        label->setLayoutHints({.preferredWidth = labelWidth, .horizontalAlignment = ui::Alignment::End});
        auto* value = form->addChild<ui::Label>(row.value);
        if (row.wrap) {
            value->setWrap(true);
            value->setLayoutHints({.fillWidth = true});
        }
        if (row.size)
            m_sizeLabel = value;
    }
    if (m_items.front()->storageTotal && *m_items.front()->storageTotal > 0) {
        auto* label = form->addChild<ui::Label>(tr("Used:"));
        label->setColor(ui::ColorId::DisabledText);
        label->setTextAlignment(ui::Label::TextAlignment::Right);
        label->setLayoutHints({.preferredWidth = labelWidth, .horizontalAlignment = ui::Alignment::End});
        auto* meter = form->addChild<ui::RowLayout>();
        meter->setSpacing(kSmallSpacing);
        m_storageBar = meter->addChild<ui::ProgressBar>();
        m_storageBar->setValue(std::clamp(double(*m_items.front()->storageUsed) / double(*m_items.front()->storageTotal),
                                           0.0, 1.0));
        m_storageBar->setLayoutHints({.fillWidth = true});
        const auto& locale = base::Locale::system();
        meter->addChild<ui::Label>(format(tr("{used} of {total}"),
                                          {{"used", locale.formattedDataSize(int64_t(*m_items.front()->storageUsed))},
                                           {"total", locale.formattedDataSize(int64_t(*m_items.front()->storageTotal))}}));
    }
    return layout;
}

std::unique_ptr<ui::View> PropertiesDialog::createPermissionsContent()
{
    auto layout = std::make_unique<ui::ColumnLayout>();
    layout->setSpacing(kLargeSpacing);
    const FileItem& first = *m_items.front();
    struct stat info {};
    const bool local = !first.path.empty() && ::stat(first.path.c_str(), &info) == 0;
    auto row = [&layout](const std::string& label, const std::string& value) {
        auto* line = layout->addChild<ui::RowLayout>();
        line->setSpacing(kLargeSpacing);
        auto* title = line->addChild<ui::Label>(label);
        title->setColor(ui::ColorId::DisabledText);
        title->setLayoutHints({.preferredWidth = 110});
        line->addChild<ui::Label>(value)->setLayoutHints({.fillWidth = true});
    };
    if (!local) {
        layout->addChild<ui::Label>(tr("Permissions are unavailable for this location."));
        return layout;
    }
    row(tr("Owner:"), std::to_string(info.st_uid));
    row(tr("Group:"), std::to_string(info.st_gid));
    row(tr("Mode:"), std::to_string(unsigned(info.st_mode & 0777)));
    layout->addChild<ui::Label>(tr("Permissions are shown for the selected item."))->setColor(ui::ColorId::DisabledText);
    return layout;
}

} // namespace fm
