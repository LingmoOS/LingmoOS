// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "archivewindow.h"
#include "base/i18n.h"
#include "base/process.h"
#include "base/url.h"
#include "ui/about_dialog.h"
#include "ui/application.h"
#include "ui/button.h"
#include "ui/combo_box.h"
#include "ui/file_chooser.h"
#include "ui/file_drop_area.h"
#include "ui/keys.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/list_view.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/popup_glass.h"
#include "ui/progress_bar.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/window_header.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>
using namespace Lingmo::ui;
using Lingmo::i18n::tr;
namespace ArchiveManager {
namespace {
constexpr double sidebarWidth = 224;
std::string bytesText(uint64_t value) {
    double size = static_cast<double>(value);
    const char* units[] = {"B", "KiB", "MiB", "GiB"};
    size_t index = 0;
    while (size >= 1024 && index < 3) { size /= 1024; ++index; }
    std::ostringstream stream; stream << std::fixed << std::setprecision(index ? 1 : 0) << size << ' ' << units[index];
    return stream.str();
}
// Translate application-owned diagnostics at the UI boundary; retain paths and
// libarchive/OS diagnostics verbatim. The core stays usable without the UI SDK.
std::string translatedArchiveDetail(const std::string& detail)
{
    static const char* messages[] = {
        Lingmo::i18n::trNoop("UTF-8 filename locale is unavailable"),
        Lingmo::i18n::trNoop("Cannot set filename locale"),
        Lingmo::i18n::trNoop("Archive operation failed"),
        Lingmo::i18n::trNoop("Cannot allocate archive reader"),
        Lingmo::i18n::trNoop("Not a supported archive"),
        Lingmo::i18n::trNoop("Negative archive entry size"),
        Lingmo::i18n::trNoop("Archive directory conflicts with another entry"),
        Lingmo::i18n::trNoop("Destination directory is unavailable"),
        Lingmo::i18n::trNoop("Invalid selected path"),
        Lingmo::i18n::trNoop("Selected entries are no longer present"),
        Lingmo::i18n::trNoop("No source files selected"),
        Lingmo::i18n::trNoop("Cannot allocate archive writer"),
        Lingmo::i18n::trNoop("Save the archive outside the selected source folder"),
        Lingmo::i18n::trNoop("Invalid source name: "),
        Lingmo::i18n::trNoop("Duplicate source name: "),
        Lingmo::i18n::trNoop("Source file changed: "),
        Lingmo::i18n::trNoop("Cannot read source: "),
        Lingmo::i18n::trNoop("Duplicate file entry: "),
        Lingmo::i18n::trNoop("Conflicting archive entry: "),
    };
    for (const auto* source : messages) {
        const std::string prefix(source);
        if (detail == prefix || (prefix.ends_with(": ") && detail.starts_with(prefix)))
            return std::string(tr(source)) + detail.substr(prefix.size());
    }
    return detail;
}
std::string errorText(const Result& result) {
    switch (result.error) {
    case Error::Canceled: return tr("Operation canceled. No output was committed.");
    case Error::Password: return tr("Password required, incorrect password, or unsupported encryption. Enter a password and retry.");
    case Error::Exists: return tr("The destination already exists. Choose a new name.");
    case Error::Limit: return tr("The operation exceeds the size or entry limit.");
    case Error::UnsupportedEntry: return tr("This archive contains a link, special file or unsafe path that cannot be extracted.");
    default: return std::string(tr("Operation failed")) + ": " + translatedArchiveDetail(result.detail);
    }
}
class Header : public WindowHeader {
public:
    Header() : WindowHeader(44) {
        title = addChild<Label>(); title->setName("window-title");
        title->setTextAlignment(Label::TextAlignment::Center); title->setVerticallyCentered(true);
    }
    Label* title;
protected:
    void layout() override {
        WindowHeader::layout();
        title->setGeometry({104, 0, std::max(0.0, geometry().width - 208), 44});
    }
};
class ActionButton : public Button {
public:
    ActionButton() { m_icon = addChild<Icon>("", 18); }
    void setGlyph(const std::string& glyph) { m_icon->setIconName(glyph); m_hasIcon = !glyph.empty(); requestLayout(); }
    void setEmphasized(bool value) { m_emphasized = value; requestPaint(); }
protected:
    void layout() override {
        Button::layout();
        m_icon->setGeometry({12, (geometry().height - 18) / 2, 18, 18});
        m_icon->setVisible(m_hasIcon);
        for (const auto& child : children()) if (auto* label = dynamic_cast<Label*>(child.get())) {
            label->setGeometry({m_hasIcon ? 38.0 : 12.0, 0, std::max(0.0, geometry().width - (m_hasIcon ? 50 : 24)), geometry().height});
            label->setTextAlignment(m_hasIcon ? Label::TextAlignment::Left : Label::TextAlignment::Center);
        }
        m_icon->setColor(isEnabled() ? std::optional<Color>(Theme::current().text) : std::optional<Color>(Theme::current().disabledText));
    }
    void paint(Painter& painter) override {
        Button::paint(painter);
        if (m_emphasized && isEnabled())
            painter.fillRoundedRect(localRect(), Theme::current().mediumRadius, Theme::current().highlight.withAlpha(0.18));
    }
private:
    Icon* m_icon;
    bool m_hasIcon = false, m_emphasized = false;
};
void registerGlyphs() {
    static const bool registered = [] {
        Icon::registerGlyph("archive-box", R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g fill="none" stroke="currentColor" stroke-width="1.6" stroke-linejoin="round"><rect x="4" y="7" width="16" height="14" rx="2"/><rect x="3" y="3" width="18" height="4" rx="1"/><path d="M9 11h6M12 15v3"/></g></svg>)");
        Icon::registerGlyph("archive-document", R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><g fill="none" stroke="currentColor" stroke-width="1.6" stroke-linejoin="round"><path d="M6 3h8l4 4v14H6zM14 3v5h4M9 12h6M9 16h6"/></g></svg>)");
        return true;
    }();
    (void)registered;
}
class EntryRow : public View {
public:
    EntryRow() {
        m_icon = addChild<Icon>("archive-document", 22);
        m_mark = addChild<Label>(); m_name = addChild<Label>(); m_kind = addChild<Label>(); m_size = addChild<Label>();
        m_mark->setTextAlignment(Label::TextAlignment::Center);
        m_size->setTextAlignment(Label::TextAlignment::Right);
        for (auto* label : {m_mark, m_name, m_kind, m_size}) label->setVerticallyCentered(true);
        m_name->setElide(Label::Elide::Middle); m_name->setFontSize(13);
        m_kind->setFontSize(12); m_size->setFontSize(12);
    }
    void bind(const Entry& entry, const std::string& label, bool selected) {
        m_selected = selected; setName("entry:" + entry.path);
        m_mark->setText(selected ? "✓" : "");
        m_icon->setIconName(entry.directory ? "folder" : "archive-document");
        m_folder = entry.directory;
        m_name->setText(label); m_kind->setText(entry.unsafe ? tr("Restricted") : entry.directory ? tr("Folder") : entry.encrypted ? tr("Encrypted") : tr("File"));
        m_size->setText(entry.directory ? "—" : bytesText(entry.size)); setToolTip(entry.path);
        requestPaint();
    }
    std::function<void()> onSelect, onOpen;
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }
protected:
    void layout() override {
        const auto r = localRect();
        m_mark->setGeometry({0, 0, 22, r.height});
        m_icon->setGeometry({28, (r.height - 22) / 2, 22, 22});
        m_icon->setColor(m_folder ? Theme::current().highlight : Theme::current().disabledText);
        m_name->setGeometry({62, 0, std::max(0.0, r.width - 242), r.height});
        m_kind->setGeometry({r.width - 172, 0, 82, r.height}); m_size->setGeometry({r.width - 84, 0, 76, r.height});
    }
    void paint(Painter& painter) override {
        if (m_selected || m_hovered || hasVisibleFocus())
            painter.fillRoundedRect(localRect(), 7, Theme::current().highlight.withAlpha(m_selected ? 0.16 : 0.07));
    }
    void pointerEnter() override { m_hovered = true; requestPaint(); }
    void pointerLeave() override { m_hovered = false; requestPaint(); }
    void pointerPress(const PointerEvent& event) override { if (event.button == MouseButton::Left) { m_pressed = true; setFocus(); } }
    void pointerRelease(const PointerEvent& event) override {
        if (!m_pressed || event.button != MouseButton::Left) return;
        m_pressed = false;
        if (!localRect().contains(event.position)) return;
        if (event.clickCount >= 2) { if (auto action = onOpen) action(); }
        else if (auto action = onSelect) action();
    }
    bool keyPress(const KeyEvent& event) override {
        if (event.key == key::Return || event.key == key::KeypadEnter) { if (auto action = onOpen) action(); return true; }
        if (event.key == key::Space) { if (auto action = onSelect) action(); return true; }
        return false;
    }
private:
    Icon* m_icon;
    bool m_folder = false;
    Label *m_mark, *m_name, *m_kind, *m_size;
    bool m_selected = false, m_hovered = false, m_pressed = false;
};
}
class ArchiveView : public View {
public:
    explicit ArchiveView(ArchiveWindow& owner) : m_owner(owner) {
        registerGlyphs();
        setName("archive-manager");
        m_header = addChild<Header>();
        m_localMenu = addChild<Button>("⋯"); m_localMenu->setName("local-menu");
        m_localMenu->onClicked = [this] { m_owner.showMenu(m_localMenu); };
        m_open = button("open", [&owner] { owner.openChooser(); });
        m_new = button("new", [&owner] { owner.newArchive(); });
        m_browse = button("browse", [&owner] { owner.showArchive(); });
        m_archiveIcon = addChild<Icon>("archive-box", 34);
        m_archiveIcon->setName("archive-icon");
        m_archiveLabel = addChild<Label>(); m_archiveLabel->setWeight(600); m_archiveLabel->setFontSize(14); m_archiveLabel->setWrap(true); m_archiveLabel->setMaximumLineCount(3);
        m_details = addChild<Label>(); m_details->setWrap(true); m_details->setFontSize(12);
        m_extract = button("extract-all", [&owner] { owner.extractChooser(false); });
        m_extractSelected = button("extract-selected", [&owner] { owner.extractChooser(true); });
        m_verify = button("verify", [&owner] { owner.verifyArchive(); });
        m_passwordToggle = button("password-toggle", [this] { m_passwordExpanded = !m_passwordExpanded; refresh(); });
        m_passwordToggle->setGlyph("lock");
        m_passwordLabel = addChild<Label>(); m_passwordLabel->setFontSize(12);
        m_password = addChild<TextField>(); m_password->setName("password"); m_password->setEchoMode(TextField::EchoMode::Password); m_password->setMaximumLength(1024);
        m_password->onAccepted = [&owner] { if (!owner.m_path.empty()) owner.openArchive(owner.m_path); };
        m_reveal = button("reveal-output", [&owner] { owner.revealOutput(); });
        m_hint = addChild<Label>(); m_hint->setWrap(true); m_hint->setFontSize(12);
        m_subtitle = addChild<Label>(); m_subtitle->setFontSize(12);
        m_heading = addChild<Label>(); m_heading->setName("page-title"); m_heading->setFontSize(24); m_heading->setWeight(600); m_heading->setElide(Label::Elide::Middle); m_heading->setVerticallyCentered(true);
        m_up = button("up", [&owner] { owner.up(); });
        m_location = addChild<Label>(); m_location->setName("location"); m_location->setElide(Label::Elide::Middle); m_location->setVerticallyCentered(true);
        m_search = addChild<TextField>(); m_search->setName("search"); m_search->setMaximumLength(256);
        m_search->onTextChanged = [this] { if (m_owner.m_query != m_search->text()) m_owner.setQuery(m_search->text()); };
        m_addFiles = button("add-files", [&owner] { owner.addChooser(false); });
        m_addFolder = button("add-folder", [&owner] { owner.addChooser(true); });
        m_remove = button("remove", [&owner] { owner.removeSelected(); });
        m_format = addChild<ComboBox>(std::vector<std::string>{"ZIP (.zip)", "TAR (.tar)", "TAR + gzip (.tar.gz)", "TAR + xz (.tar.xz)"});
        m_format->setName("format"); m_format->setCurrentIndex(0);
        m_save = button("create-archive", [&owner] { owner.createChooser(); });
        m_columns = addChild<Label>(); m_columns->setFontSize(12);
        m_typeColumn = addChild<Label>(); m_typeColumn->setFontSize(12);
        m_sizeColumn = addChild<Label>(); m_sizeColumn->setFontSize(12); m_sizeColumn->setTextAlignment(Label::TextAlignment::Right);
        for (auto* label : {m_columns, m_typeColumn, m_sizeColumn}) label->setVerticallyCentered(true);
        m_list = addChild<ListView>(); m_list->setName("entries"); m_list->setRowHeight(48); m_list->setSpacing(2);
        m_list->createItem = [] { return std::make_unique<EntryRow>(); };
        m_list->bindItem = [this](View* item, size_t row) {
            if (row >= m_rows.size()) return;
            auto* entry = static_cast<EntryRow*>(item);
            const auto data = m_rows[row];
            const auto label = m_owner.m_query.empty() ? std::filesystem::path(data.path).filename().string() : data.path;
            entry->bind(data, label, m_owner.selected(data.path));
            entry->onSelect = [this, path = data.path] { m_owner.select(path); };
            entry->onOpen = [this, data] {
                if (m_owner.m_page == ArchiveWindow::Page::Browse && data.directory && !data.unsafe) m_owner.navigate(data.path);
                else m_owner.select(data.path);
            };
        };
        m_empty = addChild<Label>(); m_empty->setName("empty-state"); m_empty->setWrap(true); m_empty->setTextAlignment(Label::TextAlignment::Center); m_empty->setFontSize(17);
        m_status = addChild<Label>(); m_status->setName("status"); m_status->setWrap(true); m_status->setMaximumLineCount(3); m_status->setFontSize(12);
        m_progress = addChild<ProgressBar>();
        m_cancel = button("cancel", [&owner] { owner.cancel(); });
        m_open->setGlyph("folder"); m_new->setGlyph("window-add"); m_browse->setGlyph("list");
        m_extract->setGlyph("archive-box"); m_verify->setGlyph("check"); m_reveal->setGlyph("folder");
        m_addFiles->setGlyph("window-add"); m_addFolder->setGlyph("folder");
        m_extract->setEmphasized(true); m_save->setEmphasized(true);
    }
    std::string password() const { return m_password->text(); }
    void clearPassword() { m_password->clear(); m_passwordExpanded = false; }
    void expandPassword() { m_passwordExpanded = true; }
    Format format() const { return static_cast<Format>(std::max(0, m_format->currentIndex())); }
    void focusSearch() { m_search->setFocus(); }
    void refresh() {
        const auto& owner = m_owner;
        const bool create = owner.m_page == ArchiveWindow::Page::Create;
        const bool loaded = owner.m_listing.ok() && !owner.m_listing.output.empty();
        const bool available = owner.m_globalMenu && owner.m_globalMenu->isAvailable();
        if (available && m_localMenu->hasFocus()) setFocus();
        m_localMenu->setVisible(!available); m_localMenu->setToolTip(tr("Menu"));
        m_header->title->setText(tr("Archive Manager"));
        m_open->setText(tr("Open Archive…")); m_new->setText(tr("New Archive")); m_browse->setText(tr("Archive Contents"));
        m_open->setEnabled(!owner.m_busy); m_new->setEnabled(!owner.m_busy); m_browse->setEnabled(!owner.m_busy && !owner.m_path.empty());
        m_new->setEmphasized(create); m_browse->setEmphasized(!create && loaded);
        m_heading->setText(create ? tr("Create an archive") : loaded ? std::filesystem::path(owner.m_path).filename().string() : tr("Your archives, organized"));
        m_heading->setToolTip(owner.m_path);
        m_subtitle->setText(create ? tr("Choose your files and a compression format.") : loaded ? tr("Browse, select and extract your files.") : tr("Open a compressed file to get started."));
        m_archiveLabel->setText(create ? tr("New archive") : owner.m_path.empty() ? tr("No archive open") : std::filesystem::path(owner.m_path).filename().string());
        m_archiveLabel->setToolTip(owner.m_path);
        m_details->setText(create ? std::to_string(owner.m_sources.size()) + " " + tr("source items")
            : loaded ? owner.m_listing.format + "\n" + std::to_string(owner.m_listing.entries.size()) + " " + tr("entries") + "\n" + bytesText(owner.m_listing.totalBytes) + " " + tr("uncompressed") : tr("ZIP · TAR · 7z · RAR"));
        m_extract->setText(tr("Extract All…")); m_extractSelected->setText(tr("Extract Selected…")); m_verify->setText(tr("Check Integrity"));
        m_extract->setEnabled(!owner.m_busy && loaded && !create);
        m_extractSelected->setEnabled(!owner.m_busy && loaded && !create && !owner.m_selection.empty());
        m_verify->setEnabled(!owner.m_busy && loaded && !create);
        m_passwordLabel->setText(tr("Archive password")); m_password->setPlaceholder(tr("Optional · not saved"));
        m_password->setEnabled(!owner.m_busy && !create);
        m_extract->setVisible(!create && loaded);
        m_extractSelected->setVisible(!create && loaded && !owner.m_selection.empty());
        m_verify->setVisible(!create && loaded);
        const bool encrypted = std::any_of(owner.m_listing.entries.begin(), owner.m_listing.entries.end(), [](const Entry& entry) { return entry.encrypted; });
        const bool passwordVisible = !create && (m_passwordExpanded || encrypted);
        m_passwordToggle->setVisible(!create && !owner.m_path.empty());
        m_passwordToggle->setText(passwordVisible ? tr("Password options") : tr("Enter a password…"));
        m_passwordToggle->setEnabled(!owner.m_busy);
        m_passwordLabel->setVisible(passwordVisible); m_password->setVisible(passwordVisible);
        m_reveal->setText(tr("Show Output Folder")); m_reveal->setEnabled(!owner.m_output.empty() && !owner.m_busy); m_reveal->setVisible(!owner.m_output.empty());
        m_hint->setText(create ? tr("ZIP, TAR, gzip or xz. Source files are kept.") : tr("Extraction creates a new folder. Existing files are kept."));
        m_up->setText(tr("Up")); m_up->setEnabled(!owner.m_busy && !owner.m_directory.empty());
        m_up->setVisible(!create); m_location->setVisible(!create); m_search->setVisible(!create);
        m_location->setText(owner.m_directory.empty() ? "/" : "/" + owner.m_directory); m_location->setToolTip(m_location->text());
        m_search->setPlaceholder(tr("Search this folder")); m_search->setEnabled(!owner.m_busy && loaded);
        if (m_search->text() != owner.m_query) m_search->setText(owner.m_query);
        m_addFiles->setText(tr("Add Files…")); m_addFolder->setText(tr("Add Folder…")); m_remove->setText(tr("Remove")); m_save->setText(tr("Create…"));
        for (View* view : std::vector<View*>{m_addFiles, m_addFolder, m_remove, m_format, m_save}) view->setVisible(create);
        m_addFiles->setEnabled(!owner.m_busy); m_addFolder->setEnabled(!owner.m_busy);
        m_remove->setEnabled(!owner.m_busy && !owner.m_sourceSelection.empty());
        m_format->setEnabled(!owner.m_busy); m_save->setEnabled(!owner.m_busy && !owner.m_sources.empty());
        m_columns->setText(tr("Name")); m_typeColumn->setText(tr("Type")); m_sizeColumn->setText(tr("Size"));
        m_rows.clear();
        if (create) {
            for (const auto& path : owner.m_sources) {
                const auto found = owner.m_sourceInfo.find(path);
                m_rows.push_back(found == owner.m_sourceInfo.end() ? Entry{path} : found->second);
            }
        } else m_rows = ArchiveManager::children(owner.m_listing.entries, owner.m_directory, owner.m_query);
        m_list->setRowCount(m_rows.size()); m_list->reload(); m_list->setEnabled(!owner.m_busy);
        m_empty->setVisible(m_rows.empty());
        m_empty->setText(create ? tr("Add files or folders to create a new archive.")
            : !loaded ? tr("Open an archive or drop it here.\nBrowse its contents before extracting.") : tr("No matching entries"));
        m_status->setText(owner.m_status.empty() ? std::to_string(m_rows.size()) + " " + tr("items shown") : owner.m_status);
        m_status->setToolTip(owner.m_status);
        m_cancel->setText(tr("Cancel")); m_cancel->setVisible(owner.m_busy);
        m_progress->setVisible(owner.m_busy); m_progress->setIndeterminate(owner.m_busy);
        requestLayout(); requestPaint();
    }
    void setProgressText(const std::string& text) { m_status->setText(text); m_status->setToolTip(text); }
    bool acceptsFocus() const override { return true; }
protected:
    void layout() override {
        const auto r = localRect();
        m_header->setGeometry({0, 0, r.width, 44}); m_localMenu->setGeometry({12, 6, 40, 32});
        const double side = sidebarWidth - 32;
        m_open->setGeometry({16, 64, side, 36}); m_new->setGeometry({16, 108, side, 36}); m_browse->setGeometry({16, 152, side, 36});
        m_archiveIcon->setGeometry({28, 232, 34, 34}); m_archiveIcon->setColor(Theme::current().highlight);
        m_archiveLabel->setGeometry({24, 288, side - 16, 54}); m_details->setGeometry({24, 350, side - 16, 72});
        m_passwordToggle->setGeometry({16, 448, side, 34});
        m_passwordLabel->setGeometry({24, 498, side - 16, 20}); m_password->setGeometry({16, 524, side, 34});
        m_reveal->setGeometry({16, r.height - 100, side, 34}); m_hint->setGeometry({24, r.height - 54, side - 16, 42});
        const double x = sidebarWidth + 24, w = r.width - x - 24;
        const bool create = m_owner.m_page == ArchiveWindow::Page::Create;
        m_heading->setGeometry({x, 64, w - (m_extract->isVisible() ? 164 : 0), 36});
        m_subtitle->setGeometry({x, 106, w, 24});
        m_extract->setGeometry({r.width - 172, 64, 148, 36});
        m_up->setGeometry({x, 150, 64, 34}); m_location->setGeometry({x + 76, 150, std::max(0.0, w - 302), 34});
        m_search->setGeometry({r.width - 236, 150, 212, 34});
        m_verify->setGeometry({x, 200, 154, 32}); m_extractSelected->setGeometry({x + 162, 200, 164, 32});
        m_addFiles->setGeometry({x, 150, 130, 34}); m_addFolder->setGeometry({x + 138, 150, 144, 34}); m_remove->setGeometry({x + 290, 150, 94, 34});
        m_format->setGeometry({x, 200, w - 132, 34}); m_save->setGeometry({r.width - 144, 200, 120, 34});
        const double top = 252;
        m_tableTop = top;
        m_columns->setGeometry({x + 62, top, w - 242, 30});
        m_typeColumn->setGeometry({x + w - 172, top, 82, 30}); m_sizeColumn->setGeometry({x + w - 84, top, 76, 30});
        m_list->setGeometry({x, top + 38, w, std::max(80.0, r.height - top - 134)});
        m_empty->setGeometry({x + 32, top + 100, w - 64, 100});
        m_status->setGeometry({x, r.height - 76, w - (m_owner.m_busy ? 108 : 0), 52});
        m_cancel->setGeometry({r.width - 120, r.height - 72, 96, 34}); m_progress->setGeometry({x, r.height - 12, w, 4});
        updateMaterial();
    }
    void paint(Painter& painter) override {
        const auto r = localRect(); const auto& theme = Theme::current();
        if (!m_glass) painter.fillRect({0, 0, sidebarWidth, r.height}, theme.secondBackground);
        else if (window() && !window()->isActive()) painter.fillRect({0, 0, sidebarWidth, r.height}, theme.background.withAlpha(0.1));
        painter.fillRect({sidebarWidth, 0, r.width - sidebarWidth, r.height}, theme.background);
        painter.fillRect({sidebarWidth, 0, 1 / painter.scale(), r.height}, theme.text.withAlpha(0.08));
        painter.fillRoundedRect({20, 224, 50, 50}, 14, theme.highlight.withAlpha(0.12));
        painter.fillRect({24, 208, sidebarWidth - 48, 1 / painter.scale()}, theme.text.withAlpha(0.08));
        const double x = sidebarWidth + 24, w = r.width - x - 24;
        painter.fillRoundedRect({x, m_tableTop, w, 30}, 7, theme.text.withAlpha(0.035));
        painter.fillRect({x, r.height - 88, w, 1 / painter.scale()}, theme.text.withAlpha(0.08));
    }
    bool keyPress(const KeyEvent& event) override {
        if (event.key == key::Escape) { if (m_owner.m_busy) m_owner.cancel(); else m_owner.clearSelection(); return true; }
        if (event.key == key::BackSpace && m_owner.m_page == ArchiveWindow::Page::Browse) { m_owner.up(); return true; }
        return false;
    }
private:
    ActionButton* button(const std::string& name, std::function<void()> action) { auto* b = addChild<ActionButton>(); b->setName(name); b->onClicked = std::move(action); return b; }
    void updateMaterial() {
        if (!window()) return;
        m_glass = window()->backgroundGlassAvailable();
        window()->setBackgroundColor(m_glass ? Color{0, 0, 0, 0} : Theme::current().background);
        window()->setTranslucentSubpixelText(!m_glass);
        window()->setBackgroundGlass(m_glass ? std::optional<GlassMaterial>(popupGlass(Theme::current().darkMode)) : std::nullopt);
        window()->setBackgroundBlur(m_glass ? localRect() : RectF{}, m_glass ? Theme::current().windowRadius : 0);
    }
    ArchiveWindow& m_owner;
    Header* m_header;
    Button* m_localMenu;
    Icon* m_archiveIcon;
    ActionButton *m_open, *m_new, *m_browse, *m_extract, *m_extractSelected, *m_verify, *m_reveal, *m_passwordToggle;
    ActionButton *m_up, *m_addFiles, *m_addFolder, *m_remove, *m_save, *m_cancel;
    Label *m_subtitle, *m_archiveLabel, *m_details, *m_passwordLabel, *m_hint, *m_heading, *m_location, *m_columns, *m_typeColumn, *m_sizeColumn, *m_empty, *m_status;
    TextField *m_password, *m_search;
    ComboBox* m_format;
    ListView* m_list;
    ProgressBar* m_progress;
    std::vector<Entry> m_rows;
    bool m_glass = false, m_passwordExpanded = false;
    double m_tableTop = 252;
};
ArchiveWindow::ArchiveWindow(Lingmo::base::dbus::Connection* menuBus) {
    m_window.setTitle(tr("Archive Manager")); m_window.setMinimumSize({820, 720}); m_window.resize({920, 740});
    auto drop = std::make_unique<FileDropArea>();
    m_view = drop->addChild<ArchiveView>(*this);
    drop->filter = [this](const std::string& url) { return !m_busy && Lingmo::base::url::isLocalFile(url); };
    drop->onDropped = [this](const std::vector<std::string>& urls) {
        std::vector<std::string> paths;
        for (const auto& url : urls) if (auto local = Lingmo::base::url::toLocalFile(url)) paths.push_back(*local);
        if (m_page == Page::Create) addSources(paths); else openArguments(paths);
    };
    m_window.setContentView(std::move(drop));
    m_window.onActiveChanged = [this] { m_window.requestPaint(); };
    m_window.onStatesChanged = [this] { m_window.requestLayout(); };
    m_window.onCloseRequested = [this] {
        if (m_busy) { m_closeWhenDone = true; cancel(); }
        else Application::instance()->quit();
    };
    m_about = std::make_unique<AboutDialog>(&m_window);
    m_window.addShortcut(ControlModifier, key::letter('o'), [this] { openChooser(); });
    m_window.addShortcut(ControlModifier, key::letter('n'), [this] { newArchive(); });
    m_window.addShortcut(ControlModifier, key::letter('e'), [this] { extractChooser(false); });
    m_window.addShortcut(ControlModifier, key::letter('t'), [this] { verifyArchive(); });
    m_window.addShortcut(ControlModifier, key::letter('a'), [this] { selectAll(); });
    m_window.addShortcut(ControlModifier, key::letter('f'), [this] { if (m_page == Page::Browse) m_view->focusSearch(); });
    m_window.addShortcut(ControlModifier, key::letter('q'), [this] { m_window.onCloseRequested(); });
    if (auto* bus = menuBus ? menuBus : Application::instance()->sessionBus()) {
        m_globalMenu = std::make_unique<GlobalMenu>(bus, [this] { return menuEntries(); }, std::function<void()>{});
        m_menuAvailability = m_globalMenu->availabilityChanged.connect([this](bool) { refresh(); });
    }
    m_language = Application::instance()->languageChanged.connect([this] { m_window.setTitle(tr("Archive Manager")); refresh(); });
    refresh();
}
ArchiveWindow::~ArchiveWindow() { if (m_cancel) *m_cancel = true; m_job.cancel(); }
void ArchiveWindow::show() { m_window.show(); m_view->setFocus(); }
void ArchiveWindow::refresh() { m_view->refresh(); refreshMenus(); }
void ArchiveWindow::refreshMenus() { if (m_globalMenu) m_globalMenu->update(); }
bool ArchiveWindow::openArguments(const std::vector<std::string>& paths, bool createMode) {
    if (m_busy) return false;
    if (paths.empty()) return true;
    std::error_code error;
    if (createMode || paths.size() > 1 || std::filesystem::is_directory(paths.front(), error)) { newArchive(); addSources(paths); }
    else openArchive(paths.front());
    return true;
}
void ArchiveWindow::openArchive(const std::string& path) {
    if (m_busy) return;
    if (path != m_path) m_view->clearPassword();
    m_path = path; m_directory.clear(); m_query.clear(); m_selection.clear(); m_listing = {}; m_page = Page::Browse;
    run(Operation::Inspect, [path](const Options& options) { return inspect(path, options); });
}
void ArchiveWindow::openChooser() {
    if (m_busy) return;
    m_chooser = std::make_unique<FileChooser>(&m_window); m_chooser->setMode(FileChooser::Mode::OpenFile);
    m_chooser->setTitle(tr("Open Archive"));
    m_chooser->setFilters({{tr("Archives"), {"*.zip", "*.tar", "*.tar.gz", "*.tgz", "*.tar.xz", "*.7z", "*.rar", "*.gz", "*.xz", "*.bz2", "*.zst"}, {}}, {tr("All files"), {"*"}, {}}});
    m_chooser->onAccepted = [weak = m_weak.getWeakPtr()](const auto& paths, const auto&) { if (weak && !paths.empty()) weak->openArchive(paths.front()); };
    m_chooser->open();
}
void ArchiveWindow::newArchive() { if (m_busy) return; m_page = Page::Create; m_status.clear(); refresh(); }
void ArchiveWindow::showArchive() { if (m_busy) return; m_page = Page::Browse; refresh(); }
void ArchiveWindow::addSources(const std::vector<std::string>& paths) {
    if (m_busy) return;
    m_page = Page::Create;
    for (const auto& path : paths) {
        std::error_code error;
        const auto absolute = std::filesystem::absolute(path, error).lexically_normal().string();
        if (!error && std::find(m_sources.begin(), m_sources.end(), absolute) == m_sources.end()) {
            Entry info; info.path = absolute;
            const auto status = std::filesystem::symlink_status(absolute, error);
            info.directory = !error && std::filesystem::is_directory(status);
            info.unsafe = error || (!info.directory && !std::filesystem::is_regular_file(status));
            if (!info.unsafe && !info.directory) {
                info.size = std::filesystem::file_size(absolute, error);
                if (error) { info.size = 0; info.unsafe = true; }
            }
            m_sourceInfo[absolute] = info;
            m_sources.push_back(absolute);
        }
    }
    m_status.clear(); refresh();
}
void ArchiveWindow::addChooser(bool folder) {
    if (m_busy) return;
    m_chooser = std::make_unique<FileChooser>(&m_window); m_chooser->setMode(folder ? FileChooser::Mode::OpenFolder : FileChooser::Mode::OpenFiles);
    m_chooser->onAccepted = [weak = m_weak.getWeakPtr()](const auto& paths, const auto&) { if (weak) weak->addSources(paths); };
    m_chooser->open();
}
void ArchiveWindow::removeSelected() {
    if (m_busy || m_page != Page::Create) return;
    std::erase_if(m_sources, [this](const auto& path) { return m_sourceSelection.count(path); });
    for (const auto& path : m_sourceSelection) m_sourceInfo.erase(path);
    m_sourceSelection.clear(); refresh();
}
void ArchiveWindow::createChooser() {
    if (m_busy || m_sources.empty() || m_page != Page::Create) return;
    const auto extension = formatExtension(m_view->format());
    m_chooser = std::make_unique<FileChooser>(&m_window); m_chooser->setMode(FileChooser::Mode::SaveFile);
    m_chooser->setTitle(tr("Create Archive")); m_chooser->setCurrentFile("Archive" + extension);
    m_chooser->setFilters({{tr("Archive"), {"*" + extension}, {}}});
    m_chooser->onAccepted = [weak = m_weak.getWeakPtr(), extension](const auto& paths, const auto&) {
        if (!weak || paths.empty()) return;
        auto path = paths.front(); if (!path.ends_with(extension)) path += extension;
        weak->createAt(path);
    };
    m_chooser->open();
}
void ArchiveWindow::createAt(const std::string& destination) {
    if (m_busy || m_sources.empty()) return;
    const auto format = m_view->format();
    run(Operation::Create, [sources = m_sources, destination, format](const Options& options) { return create(sources, destination, format, options); });
}
void ArchiveWindow::extractChooser(bool selectedOnly) {
    if (m_busy || m_listing.output.empty() || m_page != Page::Browse || (selectedOnly && m_selection.empty())) return;
    m_chooser = std::make_unique<FileChooser>(&m_window); m_chooser->setMode(FileChooser::Mode::OpenFolder);
    m_chooser->setTitle(tr("Choose Extraction Destination"));
    m_chooser->onAccepted = [weak = m_weak.getWeakPtr(), selectedOnly](const auto& paths, const auto&) { if (weak && !paths.empty()) weak->extractTo(paths.front(), selectedOnly); };
    m_chooser->open();
}
void ArchiveWindow::extractTo(const std::string& parent, bool selectedOnly) {
    if (m_busy || m_listing.output.empty() || (selectedOnly && m_selection.empty())) return;
    const std::vector<std::string> selection = selectedOnly ? std::vector<std::string>(m_selection.begin(), m_selection.end()) : std::vector<std::string>{};
    run(Operation::Extract, [path = m_path, parent, selection](const Options& options) { return extract(path, parent, selection, options); });
}
void ArchiveWindow::verifyArchive() {
    if (m_busy || m_listing.output.empty() || m_page != Page::Browse) return;
    run(Operation::Verify, [path = m_path](const Options& options) { return verify(path, options); });
}
void ArchiveWindow::cancel() { if (!m_busy || !m_cancel) return; *m_cancel = true; m_status = tr("Canceling…"); m_view->setProgressText(m_status); }
void ArchiveWindow::navigate(const std::string& path) {
    if (m_busy || m_page != Page::Browse) return;
    std::string normalized;
    if (!path.empty() && !safeArchivePath(path, normalized)) return;
    m_directory = normalized; m_query.clear(); m_selection.clear(); m_status.clear(); refresh();
}
void ArchiveWindow::up() { if (!m_directory.empty()) navigate(std::filesystem::path(m_directory).parent_path().generic_string()); }
void ArchiveWindow::select(const std::string& path) {
    if (m_busy) return;
    auto& selection = m_page == Page::Create ? m_sourceSelection : m_selection;
    if (!selection.erase(path)) selection.insert(path);
    m_status = std::to_string(selection.size()) + " " + tr("selected"); refresh();
}
bool ArchiveWindow::selected(const std::string& path) const { return (m_page == Page::Create ? m_sourceSelection : m_selection).count(path); }
void ArchiveWindow::selectAll() {
    if (m_busy) return;
    if (m_page == Page::Create) m_sourceSelection = {m_sources.begin(), m_sources.end()};
    else for (const auto& entry : children(m_listing.entries, m_directory, m_query)) if (!entry.unsafe) m_selection.insert(entry.path);
    refresh();
}
void ArchiveWindow::clearSelection() { m_selection.clear(); m_sourceSelection.clear(); m_status.clear(); refresh(); }
void ArchiveWindow::setQuery(const std::string& query) { if (m_busy) return; m_query = query; m_selection.clear(); refresh(); }
void ArchiveWindow::run(Operation operation, std::function<Result(const Options&)> work) {
    m_busy = true; m_cancel = std::make_shared<std::atomic_bool>(false); ++m_generation;
    m_status = operation == Operation::Inspect ? tr("Reading archive…") : operation == Operation::Create ? tr("Creating archive…")
        : operation == Operation::Extract ? tr("Extracting…") : tr("Checking archive…");
    refresh();
    Options options; options.canceled = m_cancel;
    if (operation != Operation::Create) options.password = m_view->password();
    const auto weak = m_weak.getWeakPtr(); const auto generation = m_generation;
    const auto loop = Application::instance()->loop().handle();
    options.progress = [loop, weak, generation, last = std::chrono::steady_clock::time_point{}](const Progress& progress) mutable {
        const auto now = std::chrono::steady_clock::now();
        if (now - last < std::chrono::milliseconds(100)) return;
        last = now;
        loop.post([weak, generation, progress] { if (weak && weak->m_busy && weak->m_generation == generation) weak->progress(progress); });
    };
    m_job = Lingmo::base::runInThread([work = std::move(work), options] { return work(options); },
        [weak, operation, generation](Result result) { if (weak && weak->m_generation == generation) weak->finish(operation, std::move(result)); });
}
void ArchiveWindow::progress(const Progress& progress) {
    m_status = std::to_string(progress.entries) + " " + tr("entries processed") + " · " + bytesText(progress.bytes) + "\n" + progress.path;
    m_view->setProgressText(m_status);
}
void ArchiveWindow::finish(Operation operation, Result result) {
    m_busy = false; m_cancel.reset();
    if (m_closeWhenDone) { Application::instance()->quit(); return; }
    if (!result.ok()) { if (result.error == Error::Password) m_view->expandPassword(); m_status = errorText(result); refresh(); return; }
    if (operation == Operation::Inspect) { m_listing = std::move(result); m_status = tr("Archive ready"); }
    else if (operation == Operation::Verify) m_status = std::string(tr("Integrity check passed")) + " · " + bytesText(result.totalBytes);
    else if (operation == Operation::Extract) { m_output = result.output; m_status = std::string(tr("Extracted to")) + " " + result.output; }
    else {
        m_output = result.output;
        openArchive(result.output);
        return;
    }
    refresh();
}
void ArchiveWindow::revealOutput() {
    if (m_busy || m_output.empty()) return;
    std::error_code error;
    const auto path = std::filesystem::is_directory(m_output, error) ? m_output : std::filesystem::path(m_output).parent_path().string();
    Lingmo::base::Process::startDetached("xdg-open", {path});
}
void ArchiveWindow::showAbout() {
    m_about->setName(tr("Archive Manager")); m_about->setIconSource("lingmo-archive-manager");
    m_about->setDescription(tr("Browse, create, extract and verify archives on the Lingmo desktop.")); m_about->show();
}
std::vector<GlobalMenu::Entry> ArchiveWindow::menuEntries() {
    using E = GlobalMenu::Entry;
    auto item = [](std::string text, std::function<void()> action, bool enabled = true, std::vector<std::string> shortcut = {}) {
        E entry; entry.label = std::move(text); entry.action = std::move(action); entry.enabled = enabled; entry.shortcut = std::move(shortcut); return entry;
    };
    const bool loaded = !m_listing.output.empty() && m_page == Page::Browse && !m_busy;
    E file; file.label = tr("File");
    file.children = {item(tr("Open Archive…"), [this] { openChooser(); }, !m_busy, {"Control", "o"}),
        item(tr("New Archive"), [this] { newArchive(); }, !m_busy, {"Control", "n"}),
        item(tr("Create…"), [this] { createChooser(); }, !m_busy && m_page == Page::Create && !m_sources.empty()),
        E{.separator = true}, item(tr("Quit"), [this] { m_window.onCloseRequested(); }, true, {"Control", "q"})};
    E edit; edit.label = tr("Edit");
    edit.children = {item(tr("Select All"), [this] { selectAll(); }, !m_busy, {"Control", "a"}),
        item(tr("Clear Selection"), [this] { clearSelection(); }, !m_busy),
        item(tr("Add Files…"), [this] { addChooser(false); }, !m_busy && m_page == Page::Create),
        item(tr("Add Folder…"), [this] { addChooser(true); }, !m_busy && m_page == Page::Create),
        item(tr("Remove"), [this] { removeSelected(); }, !m_busy && m_page == Page::Create && !m_sourceSelection.empty())};
    E archive; archive.label = tr("Archive");
    archive.children = {item(tr("Extract All…"), [this] { extractChooser(false); }, loaded, {"Control", "e"}),
        item(tr("Extract Selected…"), [this] { extractChooser(true); }, loaded && !m_selection.empty()),
        item(tr("Check Integrity"), [this] { verifyArchive(); }, loaded, {"Control", "t"}),
        item(tr("Reload Archive"), [this] { openArchive(m_path); }, !m_busy && !m_path.empty()),
        item(tr("Cancel"), [this] { cancel(); }, m_busy, {"Escape"})};
    E view; view.label = tr("View"); view.children = {item(tr("Archive Contents"), [this] { showArchive(); }, !m_busy && !m_path.empty()),
        item(tr("Up"), [this] { up(); }, loaded && !m_directory.empty(), {"BackSpace"}),
        item(tr("Show Output Folder"), [this] { revealOutput(); }, !m_busy && !m_output.empty())};
    E help; help.label = tr("Help"); help.children = {item(tr("About Archive Manager"), [this] { showAbout(); })};
    return {std::move(file), std::move(edit), std::move(archive), std::move(view), std::move(help)};
}
void ArchiveWindow::showMenu(View* anchor) {
    m_menu = std::make_unique<Menu>();
    for (const auto& group : menuEntries()) {
        auto* submenu = m_menu->addSubmenu(group.label);
        for (const auto& entry : group.children) {
            if (entry.separator) submenu->addSeparator();
            else { auto id = submenu->addItem(entry.label, entry.action); submenu->setItemEnabled(id, entry.enabled); }
        }
    }
    m_menu->popupFor(anchor);
}
}
