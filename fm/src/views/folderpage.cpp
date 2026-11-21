// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/folderpage.h"
#include "views/computerview.h"
#include "views/previewclient.h"
#include "core/preferences.h"
#include "core/shortcutconvention.h"
#include "model/foldermodel.h"
#include "views/gridview.h"
#include "views/listview.h"
#include "views/pathbar.h"
#include "views/style.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/url.h"
#include "ui/header_backdrop.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"

#include <algorithm>

using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace fm {

static constexpr double kSmallSpacing = 6;
static constexpr double kLargeSpacing = 12;

FolderPage::FolderPage(FolderModelHost* host, const Start& start)
    : m_model(std::make_unique<FolderModel>(host))
    , m_preview(std::make_unique<PreviewClient>())
    , m_host(host)
{
    setName("folderPage");
    setClipsChildren(true);

    m_noResults = addChild<ui::Label>(tr("No Results"));
    m_noResults->setFontSize(15 * 4.0 / 3);
    m_noResults->setVisible(false);

    FolderModel& model = *m_model;
    const auto applySort = [this] {
        const Preferences& preferences = Preferences::instance();
        const bool trash = m_model->isTrash();
        m_model->setSortMode(trash ? preferences.trashSortMode() : preferences.sortMode());
        m_model->setSortDesc(trash ? preferences.trashSortDesc() : preferences.sortDesc());
    };
    m_subscriptions.push_back(Preferences::instance().changed.connect([this, applySort] {
        applySort();
        if (Preferences::instance().viewMethod() != m_viewMethod)
            createView();
    }));
    m_subscriptions.push_back(model.urlChanged.connect([this, applySort] {
        applySort();
        if (m_model->isComputer() && !m_computer)
            createComputerView();
        else if (!m_model->isComputer() && m_computer)
            createView();
        if (m_view) {
            m_view->reset();
            m_pendingViewPosition = m_model->viewPosition();
            focusView();
        }
        currentUrlChanged.emit();
    }));
    m_subscriptions.push_back(model.aboutToLeaveFolder.connect([this] {
        // Left again before the offset was reached: it still stands.
        if (m_view)
            m_model->setViewPosition(std::isnan(m_pendingViewPosition) ? m_view->contentY() : m_pendingViewPosition);
        m_pendingViewPosition = NAN;
    }));
    m_subscriptions.push_back(model.listingCompleted.connect([this] {
        restoreViewPosition(true);
        requestLayout();
    }));
    m_subscriptions.push_back(model.countChanged.connect([this] {
        restoreViewPosition(false);
        requestLayout();
    }));
    // Not a new location: the path editor keeps the keys while the scope changes.
    m_subscriptions.push_back(model.searchChanged.connect([this] { requestLayout(); }));
    m_subscriptions.push_back(model.openTabRequested.connect([this](const std::string& url) {
        if (onOpenInNewTab)
            onOpenInNewTab(url);
    }));

    createView();
    m_model->setUrl(start.url.empty() ? FolderModel::resolve("~") : start.url);
    if (!start.back.empty() || !start.forward.empty())
        m_model->restoreHistory(start.back, start.forward);
    if (!start.selection.empty())
        m_model->selectUrlsWhenListed(start.selection);
    applySort();
}

FolderPage::~FolderPage() = default;

void FolderPage::activatePreview()
{
    m_preview->activate();
}

std::string FolderPage::currentUrl() const
{
    return m_model->url();
}

std::string FolderPage::folderName() const
{
    return m_model->folderName();
}

std::string FolderPage::tabTitle() const
{
    if (!m_model->searchText().empty())
        return format(tr("Searching “{text}”"), {{"text", m_model->searchText()}});
    return folderName();
}

// Under search results, where the selected result is.
// A crumb opens its folder; the result's own crumb opens its folder with it
// selected. Presses on it do not reach the rows scrolling beneath.
class SearchPathStrip : public ui::View
{
public:
    explicit SearchPathStrip(FolderPage* page)
        : m_page(page)
    {
        m_crumbs = addChild<Breadcrumbs>(false);
        m_crumbs->onItemClicked = [this](const std::string& path) {
            if (path == m_path)
                m_page->model().showInFolder();
            else
                m_page->openUrl(path);
        };
    }

    void setBackdrop(ui::ScrollView* view)
    {
        if (m_backdrop) {
            std::shared_ptr<View> old = takeChild(m_backdrop);
            base::EventLoop::current()->post([old] {});
        }
        m_backdrop = addChild<ui::HeaderBackdrop>(view, ui::HeaderBackdrop::Edge::Bottom);
        m_backdrop->setSeparatorVisible(false);
        raiseChild(m_crumbs);
    }

    void update()
    {
        m_path = m_page->model().selectedPath();
        m_crumbs->setCrumbs(m_path.empty() ? std::vector<Crumb>() : pathCrumbs(base::url::fromLocalFile(m_path)));
        m_crumbs->setVisible(!m_path.empty());
        requestLayout();
    }

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const ui::RectF rect = localRect();
        if (m_backdrop) {
            m_backdrop->setBaseColor(ui::Theme::current().secondBackground);
            m_backdrop->setGeometry(rect);
        }
        // Crumb text then starts where the scope bar's label does.
        m_crumbs->setGeometry({kSmallSpacing / 2, 0, std::max(0.0, rect.width - kSmallSpacing / 2 - kLargeSpacing),
                               rect.height});
    }

private:
    FolderPage* m_page;
    Breadcrumbs* m_crumbs;
    ui::HeaderBackdrop* m_backdrop = nullptr;
    std::string m_path;
};

void FolderPage::retranslate()
{
    m_noResults->setText(tr("No Results"));
    m_model->retranslate();
    if (m_view)
        m_view->retranslate();
    if (m_computer)
        m_computer->retranslate();
    m_pathStrip->update();
    requestLayout();
}

void FolderPage::createView()
{
    m_viewMethod = Preferences::instance().viewMethod();
    if (m_computer) {
        std::shared_ptr<View> old = takeChild(m_computer);
        base::EventLoop::current()->post([old] {});
        m_computer = nullptr;
    }
    if (m_view) {
        std::shared_ptr<View> old = takeChild(m_view);
        base::EventLoop::current()->post([old] {});
    }
    if (m_backdrop) {
        std::shared_ptr<View> old = takeChild(m_backdrop);
        base::EventLoop::current()->post([old] {});
    }
    m_computer = nullptr;

    FolderView* view;
    if (m_viewMethod == Preferences::List)
        view = addChild<FolderListView>(*m_model);
    else
        view = addChild<FolderGridView>(*m_model);
    m_view = view;
    m_preview->attach(m_model.get(), view);
    view->onContextMenu = [this](ui::PointF position) {
        if (onContextMenu)
            onContextMenu(position);
    };
    view->onKeyPressed = [this](const ui::KeyEvent& event) { return handleKey(event); };

    m_backdrop = addChild<ui::HeaderBackdrop>(view);
    m_backdrop->setBaseColor(ui::Theme::current().secondBackground);
    m_backdrop->setSeparatorVisible(m_viewMethod != Preferences::List);
    if (!m_pathStrip) {
        m_pathStrip = addChild<SearchPathStrip>(this);
        m_subscriptions.push_back(m_model->selectionChanged.connect([this] { m_pathStrip->update(); }));
    }
    m_pathStrip->setBackdrop(view);
    raiseChild(m_pathStrip);
    raiseChild(m_noResults);
    requestLayout();
    focusView();
}

void FolderPage::createComputerView()
{
    if (m_view) {
        std::shared_ptr<View> old = takeChild(m_view);
        base::EventLoop::current()->post([old] {});
        m_view = nullptr;
    }
    if (m_backdrop) {
        std::shared_ptr<View> old = takeChild(m_backdrop);
        base::EventLoop::current()->post([old] {});
        m_backdrop = nullptr;
    }
    if (m_pathStrip)
        m_pathStrip->setVisible(false);
    m_computer = addChild<ComputerView>(
            [this](const std::string& url) { openUrl(url); },
            [this](const ComputerView::Entry& entry) {
                if (m_host && entry.url.starts_with("file:")) {
                    FileItem item = DirectoryLister::makeItem(base::url::toLocalFile(entry.url).value_or(std::string()));
                    if (entry.drive) {
                        item.storageUsed = entry.used;
                        item.storageTotal = entry.total;
                        item.filesystemType = entry.filesystem;
                        item.mountPoint = base::url::toLocalFile(entry.url).value_or(std::string());
                    }
                    m_host->showProperties({std::make_shared<const FileItem>(std::move(item))});
                }
            },
            [this](const ComputerView::Entry& entry) {
                if (m_computer)
                    m_computer->requestEject(entry.id);
            },
            [this](const std::string& message) {
                if (m_host)
                    m_host->notify(message);
            },
            [this](const std::string& url) {
                if (onOpenInNewTab)
                    onOpenInNewTab(url);
            });
    requestLayout();
    focusView();
}

void FolderPage::setHeaderHeight(double height)
{
    if (m_headerHeight == height)
        return;
    m_headerHeight = height;
    requestLayout();
}

void FolderPage::layout()
{
    const ui::RectF rect = localRect();
    if (m_computer) {
        m_computer->setGeometry(rect);
        m_noResults->setVisible(false);
        return;
    }
    if (m_view) {
        m_view->setGeometry(rect);
        if (m_viewMethod == Preferences::List)
            m_view->setTopInset(m_headerHeight + kSmallSpacing);
        else
            m_view->setTopInset(m_headerHeight);
        m_backdrop->setBaseColor(ui::Theme::current().secondBackground);
        m_backdrop->setVisible(m_view->topInset() > 0);
        m_backdrop->setGeometry({0, 0, rect.width, m_view->headerHeight()});
        const bool searching = !m_model->searchText().empty();
        m_pathStrip->setVisible(searching);
        m_pathStrip->setGeometry({0, rect.height - 28, rect.width, 28});
        m_view->setBottomInset(searching ? 28 : 0);
    }

    const FolderModel& model = *m_model;
    m_noResults->setVisible(!model.searchText().empty() && model.status() == FolderModel::Status::Ready
                            && model.count() == 0 && !model.searchBusy());
    const ui::SizeF size = m_noResults->sizeHint();
    m_noResults->setGeometry({ui::rounding::centeredStart(rect.width, size.width),
                              ui::rounding::centeredStart(rect.height, size.height), size.width, size.height});
}

void FolderPage::paint(ui::Painter& painter)
{
    painter.fillRect(localRect(), ui::Theme::current().secondBackground);
}

void FolderPage::openUrl(const std::string& url)
{
    const bool computer = url == "computer:///" || url.starts_with("computer:/");
    if (computer && !m_model->isComputer()) {
        m_model->setUrl("computer:///");
        focusView();
        return;
    }
    if (!computer && m_model->isComputer())
        createView();
    m_model->setUrl(url);
    focusView();
}

void FolderPage::focusView()
{
    if (m_view && m_view->contentView())
        m_view->contentView()->setFocus();
    else if (m_computer)
        m_computer->setFocus();
}

void FolderPage::goBack()
{
    m_model->goBack();
}

void FolderPage::goForward()
{
    m_model->goForward();
}

// Rows arrive in batches: the saved offset is applied as soon as it is in
// reach, so the folder does not first show its top, and the nearest one
// once the listing is complete.
void FolderPage::restoreViewPosition(bool complete)
{
    if (std::isnan(m_pendingViewPosition) || !m_view)
        return;

    m_view->forceLayout();
    const double maximum = m_view->maximumContentY();
    if (!complete && m_pendingViewPosition > maximum)
        return;
    m_view->setContentY(std::clamp(m_pendingViewPosition, 0.0, maximum));
    m_pendingViewPosition = NAN;
}

// The view's keys before the view itself.
bool FolderPage::handleKey(const ui::KeyEvent& event)
{
    if (m_preview->filterKey(event))
        return true;

    FolderModel& model = *m_model;
    const uint32_t modifiers = event.modifiers;
    const uint32_t key = event.key;
    const bool control = modifiers & ui::ControlModifier;

    if (key == ui::key::Space && modifiers == ui::NoModifier) {
        if (!event.repeat)
            m_preview->toggle();
        return true;
    }

    if (shortcuts::commandKeys()) {
        // Command reaches us as Ctrl; outside text fields these go to the file view.
        if (modifiers == ui::ControlModifier) {
            switch (key) {
            case ui::key::BackSpace:
                model.keyDeletePress();
                return true;
            case ui::key::Up:
                model.up();
                return true;
            case ui::key::Down:
                model.openSelected();
                return true;
            case '[':
                model.goBack();
                return true;
            case ']':
                model.goForward();
                return true;
            case 'i':
                model.openPropertiesDialog();
                return true;
            default:
                break;
            }
        }
        // Command+Shift+.; Shift makes the key ">" on most layouts.
        if (modifiers == (ui::ControlModifier | ui::ShiftModifier) && (key == '.' || key == '>')) {
            model.setShowHiddenFiles(!model.showHiddenFiles());
            return true;
        }
    }

    // Alt+Up, Alt+Left, Alt+Right and Alt+Enter, as in other file managers.
    if (modifiers == ui::AltModifier) {
        switch (key) {
        case ui::key::Up:
            model.up();
            return true;
        case ui::key::Left:
            model.goBack();
            return true;
        case ui::key::Right:
            model.goForward();
            return true;
        case ui::key::Return:
        case ui::key::KeypadEnter:
            model.openPropertiesDialog();
            return true;
        default:
            break;
        }
    }

    if (key == ui::key::Return || key == ui::key::KeypadEnter) {
        model.openSelected();
    } else if (key == 'c' && control) {
        model.copy();
    } else if (key == 'x' && control) {
        model.cut();
    } else if (key == 'w' && control) {
        if (onCloseRequested)
            onCloseRequested();
    } else if (key == 'v' && control) {
        model.paste();
    } else if (key == style::key::F2) {
        model.requestRename.emit();
    } else if (key == 'l' && control) {
        if (onRequestPathEditor)
            onRequestPathEditor();
    } else if (key == 'a' && control) {
        model.selectAll();
    } else if (key == ui::key::BackSpace) {
        model.up();
    } else if (key == ui::key::Delete) {
        model.keyDeletePress();
    } else if (key == style::key::F5) {
        model.refresh();
    } else if (key == 'h' && control) {
        model.setShowHiddenFiles(!model.showHiddenFiles());
    } else if (key == 'z' && control) {
        if (modifiers & ui::ShiftModifier)
            model.redo();
        else
            model.undo();
    } else {
        if (key >= 'a' && key <= 'z' && !control)
            model.keyboardSearch(event.text);
        // The event still reaches the view.
        return false;
    }
    // Taken here, before the view sees the key.
    return true;
}

} // namespace fm
