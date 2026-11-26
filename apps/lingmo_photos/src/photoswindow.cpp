// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "photoswindow.h"
#include "globalmenu.h"
#include "editengine.h"
#include <base/paths.h>
#include <base/i18n.h>
#include <base/event_loop.h>
#include <ui/about_dialog.h>
#include <ui/animation.h>
#include <ui/application.h>
#include <ui/button.h>
#include <ui/image_view.h>
#include <ui/label.h>
#include <ui/layout.h>
#include <ui/menu.h>
#include <ui/search_field.h>
#include <ui/scroll_view.h>
#include <ui/round_image_button.h>
#include <ui/text_field.h>
#include <ui/slider.h>
#include <ui/sidebar.h>
#include <ui/theme.h>
#include <ui/window.h>
#include <ui/window_header.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <system_error>

namespace Photos {
using namespace Lingmo::ui;
using Lingmo::I18n::tr;
namespace {
std::string pageTitle(const std::string& id)
{
    if (id == "photos") return tr("Photos");
    if (id == "recent") return tr("Recently Added");
    if (id == "favorites") return tr("Favorites");
    if (id == "albums") return tr("Albums");
    if (id == "folders") return tr("Folders");
    return tr("Recently Deleted");
}

std::string imageDate(const std::filesystem::file_time_type& stamp)
{
    const auto systemTime = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        stamp - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    const std::time_t value = std::chrono::system_clock::to_time_t(systemTime);
    std::tm local{};
    if (!localtime_r(&value, &local))
        return {};
    std::ostringstream result;
    result << std::put_time(&local, "%Y-%m-%d");
    return result.str();
}

class PhotoTile final : public View {
public:
    PhotoTile(Photo photo, std::function<void()> open, std::function<void(int64_t, PointF)> contextMenu)
        : m_photo(std::move(photo)), m_open(std::move(open)), m_contextMenu(std::move(contextMenu)) {
        setTracksHover(true); setCursorShape(CursorShape::Pointer);
        m_image = addChild<ImageView>(m_photo.path); m_image->setThumbnail(true); m_image->setAutoTransform(true); m_image->setFillMode(ImageView::FillMode::PreserveAspectCrop); m_image->setCornerRadius(10);
        m_label = addChild<Label>(m_photo.name); m_label->setElide(Label::Elide::Middle);
    }
    SizeF sizeHint() const override { return {150, 140}; }
    bool acceptsPointer() const override { return true; }
protected:
    void layout() override { m_image->setGeometry({0, 0, geometry().width, 112}); m_label->setGeometry({0, 116, geometry().width, 22}); }
    void paint(Painter& painter) override {
        const auto& theme = Theme::current();
        const auto fill = isHovered() ? theme.text.withAlpha(theme.darkMode ? .12 : .07) : theme.text.withAlpha(theme.darkMode ? .07 : .035);
        painter.drawSmoothRectangle(localRect(), theme.mediumRadius, fill);
    }
    void pointerRelease(const PointerEvent& event) override {
        if (!localRect().contains(event.position))
            return;
        if (event.button == MouseButton::Left && m_open)
            m_open();
        else if (event.button == MouseButton::Right && m_contextMenu)
            m_contextMenu(m_photo.id, mapToWindow(event.position));
    }
private:
    Photo m_photo; std::function<void()> m_open; std::function<void(int64_t, PointF)> m_contextMenu; ImageView* m_image = nullptr; Label* m_label = nullptr;
};

class Viewer final : public View {
public:
    Viewer(std::function<void(bool)> slideShow, std::function<void(bool)> flipChanged,
           std::function<std::string(const std::string&, const EditParams&)> exportCopy)
        : m_slideShow(std::move(slideShow)), m_flipChanged(std::move(flipChanged)),
          m_exportCopy(std::move(exportCopy))
    {
        m_image = addChild<ImageView>(); m_image->setFillMode(ImageView::FillMode::PreserveAspectFit); m_image->setAutoTransform(true);
        m_imageError = addChild<Label>(tr("Could not load image")); m_imageError->setVisible(false);
        m_image->onStatusChanged = [this](ImageView::Status status) {
            m_imageError->setVisible(status == ImageView::Status::Error);
            requestLayout();
        };
        m_editor = addChild<Row>(); m_editor->setSpacing(10); m_editor->setMargins({16, 6, 16, 6});
        m_brightness = addSlider(tr("Brightness"), -1, 1, 0);
        m_contrast = addSlider(tr("Contrast"), -1, 1, 0);
        m_saturation = addSlider(tr("Saturation"), -1, 1, 0);
        m_temperature = addSlider(tr("Temperature"), -1, 1, 0);
        m_editor->addStretch();
        auto* rotate = m_editor->addChild<Button>(tr("Rotate 90°")); rotate->setToolTip(tr("Rotate 90°")); rotate->onClicked = [this] { m_rotation = (m_rotation + 1) % 4; m_rotationLabel->setText(std::to_string(m_rotation * 90) + "°"); };
        m_rotationLabel = m_editor->addChild<Label>("0°"); m_rotationLabel->setColor(ColorId::DisabledText);
        auto* reset = m_editor->addChild<Button>(tr("Reset Adjustments")); reset->onClicked = [this] { resetAdjustments(); };
        auto* save = m_editor->addChild<Button>(tr("Save Copy")); save->onClicked = [this] { if (m_exportCopy && !m_path.empty()) m_saveStatus->setText(m_exportCopy(m_path, editParams())); };
        m_saveStatus = m_editor->addChild<Label>(); m_saveStatus->setColor(ColorId::DisabledText); m_saveStatus->setLayoutHints({.fillWidth = true});
        m_editor->setVisible(false);
    }
    void setPhoto(const Photo& photo)
    {
        m_path = photo.path;
        m_image->setSource(photo.path);
        resetAdjustments();
        m_zoom = 1;
        m_flipped = false;
        updateTransform();
        if (m_flipChanged)
            m_flipChanged(false);
    }
    void setEditing(bool editing) {
        m_editing = editing;
        m_editor->setVisible(editing);
        m_saveStatus->setText(editing ? tr("Changes are applied when saving a copy.") : "");
        requestLayout();
        requestPaint();
    }
    void zoomOut() { setZoom(m_zoom - .15); }
    void zoomIn() { setZoom(m_zoom + .15); }
    void resetZoom() { setZoom(1); }
    void flip() { m_flipped = !m_flipped; updateTransform(); if (m_flipChanged) m_flipChanged(m_flipped); }
    bool isFlipped() const { return m_flipped; }
    bool isSlideShowActive() const { return m_sliding; }
    void toggleSlideShow() { setSlideShow(!m_sliding); }
    void setSlideShow(bool enabled) { m_sliding = enabled; if (m_slideShow) m_slideShow(enabled); }
    EditParams editParams() const { return {m_brightness->value(), m_contrast->value(), m_saturation->value(), m_temperature->value(), m_rotation, m_flipped}; }
protected:
    void layout() override {
        const double editorHeight = m_editing ? 68 : 0;
        const double imageTop = m_editing ? 84 : 18;
        m_editor->setGeometry({8, 8, std::max(0.0, geometry().width - 16), editorHeight});
        m_image->setGeometry({24, imageTop, std::max(0.0, geometry().width - 48),
                              std::max(0.0, geometry().height - imageTop - 18)});
        m_imageError->setGeometry({24, geometry().height / 2 - 18,
                                   std::max(0.0, geometry().width - 48), 36});
        m_image->setSourceSize({std::max(0.0, geometry().width - 64),
                                std::max(0.0, geometry().height - imageTop - 32)});
    }
    void paint(Painter& painter) override {
        const auto& theme = Theme::current();
        painter.fillRect(localRect(), theme.background);
        if (m_editing)
            painter.drawSmoothRectangle({8, 8, std::max(0.0, geometry().width - 16), 68},
                                        theme.mediumRadius, theme.secondBackground);
    }
private:
    Slider* addSlider(const std::string& title, double from, double to, double value) {
        auto* group = m_editor->addChild<Column>();
        group->setSpacing(2);
        group->setLayoutHints({.fillWidth = true});
        auto* label = group->addChild<Label>(title);
        label->setFontSize(11);
        label->setColor(ColorId::DisabledText);
        auto* slider = group->addChild<Slider>();
        slider->setRange(from, to);
        slider->setValue(value);
        slider->setValueToolTip([title](double v) { return title + ": " + std::to_string(v); });
        return slider;
    }
    void resetAdjustments() {
        m_brightness->setValue(0);
        m_contrast->setValue(0);
        m_saturation->setValue(0);
        m_temperature->setValue(0);
        m_rotation = 0;
        m_rotationLabel->setText("0°");
        m_flipped = false;
        updateTransform();
    }
    void setZoom(double value) { m_zoom = std::clamp(value, .5, 3.0); updateTransform(); }
    void updateTransform() { if (m_image) m_image->setTransform({{}, (m_flipped ? -1 : 1) * m_zoom}); }
    std::function<void(bool)> m_slideShow;
    std::function<void(bool)> m_flipChanged;
    std::function<std::string(const std::string&, const EditParams&)> m_exportCopy;
    ImageView* m_image = nullptr;
    Label* m_imageError = nullptr;
    Row* m_editor = nullptr;
    Label* m_rotationLabel = nullptr;
    Label* m_saveStatus = nullptr;
    Slider *m_brightness = nullptr, *m_contrast = nullptr, *m_saturation = nullptr, *m_temperature = nullptr;
    std::string m_path;
    int m_rotation = 0;
    double m_zoom = 1;
    bool m_flipped = false, m_sliding = false, m_editing = false;
};

class Root final : public View {
public:
    Root(PhotosWindow& owner, PhotoStore& store, LibraryScanner& scanner) : m_owner(owner), m_store(store), m_scanner(scanner) {
        m_owner.setMenuAction([this](int id) { menuAction(id); });
        m_sidebar = addChild<SidebarList>(std::vector<SidebarList::Entry>{{"photos", tr("Photos"), "image-x-generic"}, {"recent", tr("Recently Added"), "document-open-recent"}, {"favorites", tr("Favorites"), "starred"}, {"albums", tr("Albums"), "folder"}, {"folders", tr("Folders"), "folder-open"}, {"trash", tr("Recently Deleted"), "user-trash"}});
        m_sidebar->onActivated = [this](const std::string& id) { m_page = id; m_headerTitle->setText(pageTitle(id)); m_headerTitle->setToolTip({}); refresh(); };
        m_appTitle = addChild<Label>(tr("Photos")); m_appTitle->setFontSize(18); m_appTitle->setWeight(600); m_appTitle->setVerticallyCentered(true);
        m_header = addChild<WindowHeader>(46);
        auto* headerContent = m_header->setContent<Row>();
        headerContent->setSpacing(5); headerContent->setMargins({12, 0, 12, 0});
        m_sidebarGap = headerContent->addChild<View>(); m_sidebarGap->setLayoutHints({.preferredWidth = 220});
        m_viewerBack = headerContent->addChild<RoundImageButton>("chevron-left", 30); m_viewerBack->setToolTip(tr("Back to Library")); m_viewerBack->onClicked = [this] { closeViewer(); }; m_viewerBack->setVisible(false);
        m_headerTitle = headerContent->addChild<Label>(tr("Photos")); m_headerTitle->setFontSize(16); m_headerTitle->setVerticallyCentered(true); m_headerTitle->setElide(Label::Elide::Middle); m_headerTitle->setLayoutHints({.fillWidth = true});
        m_headerPrevious = headerContent->addChild<RoundImageButton>("chevron-left", 30); m_headerPrevious->setToolTip(tr("Previous")); m_headerPrevious->onClicked = [this] { moveViewer(-1); }; m_headerPrevious->setVisible(false);
        m_headerNext = headerContent->addChild<RoundImageButton>("chevron-right", 30); m_headerNext->setToolTip(tr("Next")); m_headerNext->onClicked = [this] { moveViewer(1); }; m_headerNext->setVisible(false);
        m_headerZoomOut = headerContent->addChild<RoundImageButton>("", 30); m_headerZoomOut->setSource("zoom-out"); m_headerZoomOut->setIconMargins(4); m_headerZoomOut->setToolTip(tr("Zoom Out")); m_headerZoomOut->onClicked = [this] { m_viewer->zoomOut(); }; m_headerZoomOut->setVisible(false);
        m_headerZoomIn = headerContent->addChild<RoundImageButton>("", 30); m_headerZoomIn->setSource("zoom-in"); m_headerZoomIn->setIconMargins(4); m_headerZoomIn->setToolTip(tr("Zoom In")); m_headerZoomIn->onClicked = [this] { m_viewer->zoomIn(); }; m_headerZoomIn->setVisible(false);
        m_headerReset = headerContent->addChild<RoundImageButton>("", 30); m_headerReset->setSource("zoom-fit-best"); m_headerReset->setIconMargins(4); m_headerReset->setToolTip(tr("Reset Zoom")); m_headerReset->onClicked = [this] { m_viewer->resetZoom(); }; m_headerReset->setVisible(false);
        m_headerFlip = headerContent->addChild<RoundImageButton>("", 30); m_headerFlip->setSource("object-flip-horizontal"); m_headerFlip->setIconMargins(4); m_headerFlip->setToolTip(tr("Flip")); m_headerFlip->onClicked = [this] { m_viewer->flip(); m_headerFlip->setChecked(m_viewer->isFlipped()); }; m_headerFlip->setVisible(false);
        m_editButton = headerContent->addChild<RoundImageButton>("", 30); m_editButton->setSource("document-edit"); m_editButton->setIconMargins(4); m_editButton->setToolTip(tr("Edit Photo")); m_editButton->onClicked = [this] { setEditing(!m_editing); }; m_editButton->setVisible(false);
        m_headerFullScreen = headerContent->addChild<RoundImageButton>("", 30); m_headerFullScreen->setSource("view-fullscreen"); m_headerFullScreen->setIconMargins(4); m_headerFullScreen->setToolTip(tr("Full Screen")); m_headerFullScreen->onClicked = [this] { toggleFullScreen(); }; m_headerFullScreen->setVisible(false);
        m_headerSlideshow = headerContent->addChild<RoundImageButton>("", 30); m_headerSlideshow->setSource("media-playback-start"); m_headerSlideshow->setIconMargins(4); m_headerSlideshow->setToolTip(tr("Slideshow")); m_headerSlideshow->onClicked = [this] { m_viewer->toggleSlideShow(); }; m_headerSlideshow->setVisible(false);
        m_headerFavorite = headerContent->addChild<RoundImageButton>("starred", 30); m_headerFavorite->setToolTip(tr("Favorite")); m_headerFavorite->onClicked = [this] { toggleViewerFavorite(); }; m_headerFavorite->setVisible(false);
        m_headerRestore = headerContent->addChild<RoundImageButton>("", 30); m_headerRestore->setSource("view-refresh"); m_headerRestore->setIconMargins(4); m_headerRestore->setToolTip(tr("Restore")); m_headerRestore->onClicked = [this] { restoreViewer(); }; m_headerRestore->setVisible(false);
        m_headerTrash = headerContent->addChild<RoundImageButton>("user-trash", 30); m_headerTrash->setToolTip(tr("Move to Trash")); m_headerTrash->onClicked = [this] { trashViewer(); }; m_headerTrash->setVisible(false);
        m_header->setMinimizeVisible(true); m_header->setMaximizeVisible(true); m_header->setCloseVisible(true);
        m_search = addChild<SearchField>(); m_search->setPlaceholder(tr("Search photos")); m_search->onTextChanged = [this] { refresh(); };
        m_import = addChild<Button>(tr("Import")); m_import->onClicked = [this] { scanPictures(); };
        m_albumName = addChild<TextField>(); m_albumName->setPlaceholder(tr("New album name"));
        m_createAlbum = addChild<Button>(tr("Create Album")); m_createAlbum->onClicked = [this] { createAlbum(); }; m_albumName->onAccepted = [this] { createAlbum(); }; m_albumName->setVisible(false); m_createAlbum->setVisible(false);
        m_quickBar = addChild<Row>(); m_quickBar->setSpacing(8);
        auto* quickImport = m_quickBar->addChild<RoundImageButton>("", 28); quickImport->setSource("document-import"); quickImport->setToolTip(tr("Import Photos")); quickImport->onClicked = [this] { scanPictures(); };
        auto* quickRecent = m_quickBar->addChild<RoundImageButton>("", 28); quickRecent->setSource("document-open-recent"); quickRecent->setToolTip(tr("Recently Added")); quickRecent->onClicked = [this] { selectPage("recent"); };
        auto* quickFavorite = m_quickBar->addChild<RoundImageButton>("", 28); quickFavorite->setSource("starred"); quickFavorite->setToolTip(tr("Favorites")); quickFavorite->onClicked = [this] { selectPage("favorites"); };
        auto* quickTrash = m_quickBar->addChild<RoundImageButton>("", 28); quickTrash->setSource("user-trash"); quickTrash->setToolTip(tr("Recently Deleted")); quickTrash->onClicked = [this] { selectPage("trash"); };
        m_scroll = addChild<ScrollView>(); m_content = m_scroll->setContent<Column>(); m_content->setSpacing(20); m_content->setMargins({28, 16, 28, 24});
        m_status = addChild<Label>(); m_status->setColor(ColorId::DisabledText);
        m_viewer = addChild<Viewer>([this](bool enabled) { setSlideShow(enabled); },
                                    [this](bool flipped) { m_headerFlip->setChecked(flipped); },
                                    [this](const std::string& path, const EditParams& params) { return exportCopy(path, params); });
        m_viewer->setVisible(false);
        m_subscriptions.push_back(m_scanner.changed.connect([this] { if (m_viewer->isVisible()) m_refreshPending = true; else refresh(); }));
        m_page = "photos"; refresh();
    }
    ~Root() override {
        if (m_slideTimer)
            Application::instance()->loop().stopTimer(m_slideTimer);
    }
    void menuAction(int id) {
        if (!m_viewer->isVisible())
            return;
        switch (id) {
        case 7: closeViewer(); break;
        case 8: moveViewer(-1); break;
        case 9: moveViewer(1); break;
        case 10: m_viewer->zoomOut(); break;
        case 11: m_viewer->zoomIn(); break;
        case 12: m_viewer->resetZoom(); break;
        case 13: m_viewer->flip(); m_headerFlip->setChecked(m_viewer->isFlipped()); break;
        case 14: toggleFullScreen(); break;
        case 15: m_viewer->toggleSlideShow(); break;
        case 16: toggleViewerFavorite(); break;
        case 17: trashViewer(); break;
        case 18: restoreViewer(); break;
        case 20: setEditing(!m_editing); break;
        default: break;
        }
    }
    void layout() override {
        const double side = 220;
        const bool browsing = !m_viewer->isVisible();
        const bool albums = m_page == "albums";
        const bool album = m_page.rfind("album:", 0) == 0;
        const double width = geometry().width;
        const double height = geometry().height;

        m_header->setGeometry({0, 0, width, 46});
        m_sidebar->setVisible(browsing);
        m_appTitle->setVisible(browsing);
        m_sidebarGap->setVisible(browsing);
        m_appTitle->setGeometry({20, 0, side - 40, 46});
        m_sidebar->setGeometry({0, 46, side, std::max(0.0, height - 46)});
        m_viewerBack->setVisible(!browsing);
        for (auto* action : {m_headerPrevious, m_headerNext, m_headerZoomOut, m_headerZoomIn, m_headerReset,
                             m_headerFlip, m_editButton, m_headerFullScreen, m_headerSlideshow, m_headerFavorite,
                             m_headerTrash})
            action->setVisible(!browsing);
        m_headerRestore->setVisible(!browsing && m_viewerIndex < m_visible.size() &&
                                    m_visible[m_viewerIndex].trashed);

        m_search->setGeometry({std::max(side + 16, width - 372), 56, 240, 36});
        m_import->setGeometry({width - 106, 56, 82, 36});
        m_albumName->setGeometry({side + 28, 56, 240, 36});
        m_createAlbum->setGeometry({side + 276, 56, 116, 36});
        m_quickBar->setGeometry({side + 136, std::max(0.0, height - 34), 136, 28});
        m_scroll->setVisible(browsing);
        m_search->setVisible(browsing && !albums && !album && m_page != "trash");
        m_import->setVisible(browsing && !albums && !album);
        m_albumName->setVisible(browsing && albums);
        m_createAlbum->setVisible(browsing && albums);
        m_quickBar->setVisible(browsing && m_page == "photos");
        m_status->setVisible(browsing);
        m_scroll->setGeometry({side, 102, std::max(0.0, width - side),
                               std::max(0.0, height - 148)});
        m_status->setGeometry({side + 28, std::max(0.0, height - 30), 240, 24});

        const double contentWidth = std::max(0.0, width - side - 56);
        const int columns = std::max(1, int((contentWidth + 12) / 162));
        if (columns != m_gridColumns) {
            m_gridColumns = columns;
            for (auto* grid : m_photoGrids)
                grid->setColumns(m_gridColumns);
        }
        m_viewer->setGeometry({browsing ? side : 0, 46, std::max(0.0, width - (browsing ? side : 0)),
                               std::max(0.0, height - 46)});
    }
    void paint(Painter& painter) override {
        const auto& theme = Theme::current();
        if (m_viewer->isVisible()) {
            painter.fillRect(localRect(), theme.background);
            return;
        }
        const bool glass = window() && window()->backgroundGlassAvailable();
        const double side = 220;
        painter.fillRect({side, 0, std::max(0.0, geometry().width - side), geometry().height}, theme.background);
        painter.fillRect({0, 0, side, geometry().height}, theme.secondBackground.withAlpha(glass ? .72 : 1));
        painter.fillRect({side, 0, 1, geometry().height}, theme.text.withAlpha(.08));
    }
private:
    void refresh() {
        while (!m_content->children().empty()) m_content->takeChild(m_content->children().front().get());
        m_photoGrids.clear();
        const bool album = m_page.rfind("album:", 0) == 0;
        std::vector<Photo> photos;
        if (album) {
            photos = m_store.albumPhotos(std::stoll(m_page.substr(6)));
        } else if (m_page != "albums") {
            if (!m_search->text().empty() && m_page != "trash") {
                photos = m_store.search(m_search->text());
                if (m_page == "favorites")
                    std::erase_if(photos, [](const Photo& photo) { return !photo.favorite; });
                else if (m_page == "folders") {
                    const char* home = std::getenv("HOME");
                    const std::string pictures = home ? std::string(home) + "/Pictures" : std::string();
                    std::erase_if(photos, [&pictures](const Photo& photo) { return photo.folder != pictures; });
                }
            } else if (m_page == "favorites")
                photos = m_store.photos(true);
            else if (m_page == "trash")
                photos = m_store.photos(false, true);
            else if (m_page == "folders")
                photos = m_store.folder(std::getenv("HOME") ? std::string(std::getenv("HOME")) + "/Pictures" : std::string());
            else
                photos = m_store.photos();
        }
        m_visible = photos;
        if (m_page == "albums") {
            const auto albums = m_store.albums();
            if (albums.empty())
                m_content->addChild<Label>(tr("No albums yet"));
            for (const auto& item : albums) {
                auto* row = m_content->addChild<Row>(); row->setSpacing(8);
                auto* name = row->addChild<TextField>(item.name); name->setLayoutHints({.fillWidth = true}); name->onAccepted = [this, id = item.id, name] { if (!name->text().empty()) { m_store.renameAlbum(id, name->text()); refresh(); } };
                auto* open = row->addChild<Button>(tr("Open")); open->onClicked = [this, item] { m_page = "album:" + std::to_string(item.id); m_sidebar->setCurrent("albums"); m_headerTitle->setText(item.name); refresh(); };
                auto* remove = row->addChild<Button>(tr("Delete")); remove->onClicked = [this, id = item.id] { m_store.deleteAlbum(id); refresh(); };
            }
        } else if (album && photos.empty()) {
            m_content->addChild<Label>(tr("This album is empty"));
        } else if (photos.empty()) {
            m_content->addChild<Label>(m_search->text().empty()
                                           ? (m_page == "photos" || m_page == "recent"
                                                  ? tr("No photos yet")
                                                  : tr("No photos in this collection"))
                                           : tr("No photos match your search"));
        }
        std::map<std::string, Grid*> groups;
        for (size_t index = 0; index < photos.size(); ++index) { auto photo = photos[index]; const std::string key = photo.captured.empty() ? tr("Undated") : photo.captured; Grid* group = groups.count(key) ? groups[key] : nullptr; if (!group) { m_content->addChild<Label>(key)->setWeight(600); group = m_content->addChild<Grid>(m_gridColumns); group->setColumnSpacing(12); group->setRowSpacing(12); m_photoGrids.push_back(group); groups.emplace(key, group); } group->addChild<PhotoTile>(photo, [this, index] { openViewer(index); }, [this](int64_t photoId, PointF position) { showPhotoMenu(photoId, position); }); }
        m_status->setText(m_page == "albums" ? std::to_string(m_store.albums().size()) + " " + tr("albums") : std::to_string(photos.size()) + " " + tr("photos")); requestLayout(); requestPaint();
    }
    void showPhotoMenu(int64_t photoId, PointF position) {
        if (m_page == "trash")
            return;
        const bool inAlbum = m_page.rfind("album:", 0) == 0;
        const int64_t currentAlbum = inAlbum ? std::stoll(m_page.substr(6)) : 0;
        m_photoMenu = std::make_unique<Menu>();
        if (inAlbum) {
            m_photoMenu->addItem(tr("Remove from Album"), [this, currentAlbum, photoId] {
                m_store.removeFromAlbum(currentAlbum, photoId);
                refresh();
            });
        }
        const auto albums = m_store.albums();
        const bool hasDestination = std::any_of(albums.begin(), albums.end(), [currentAlbum](const Album& album) {
            return album.id != currentAlbum;
        });
        if (hasDestination) {
            Menu* addToAlbum = m_photoMenu->addSubmenu(tr("Add to Album"));
            for (const auto& album : albums) {
                if (album.id == currentAlbum)
                    continue;
                addToAlbum->addItem(album.name, [this, albumId = album.id, photoId] {
                    m_store.addToAlbum(albumId, photoId);
                    refresh();
                });
            }
        } else if (!inAlbum) {
            m_photoMenu->addItem(tr("Create Album"), [this] { selectPage("albums"); });
        }
        m_photoMenu->popup(window(), position);
    }
public:
    void showOpenError(const std::string& message)
    {
        m_status->setText(std::string(tr("Unable to open image")) + ": " + message);
        requestPaint();
    }
    void openPhoto(const Photo& photo)
    {
        m_page = photo.trashed ? "trash" : "photos";
        m_sidebar->setCurrent(m_page);
        m_search->clear();
        m_headerTitle->setText(activePageTitle());
        m_headerTitle->setToolTip({});
        m_owner.window().setTitle(photo.name);
        refresh();
        const auto found = std::find_if(m_visible.begin(), m_visible.end(),
                                        [&photo](const Photo& item) { return item.id == photo.id; });
        if (found == m_visible.end()) {
            showOpenError(photo.path);
            return;
        }
        openViewer(size_t(std::distance(m_visible.begin(), found)), false);
    }
private:
    void createAlbum() { const auto name = m_albumName->text(); if (name.empty()) return; if (m_store.createAlbum(name) != 0) { m_albumName->clear(); refresh(); } }
    std::string exportCopy(const std::string& path, const EditParams& params) {
        const std::filesystem::path source(path);
        const auto target = source.parent_path() / (source.stem().string() + " (edited).png");
        std::string error;
        if (!saveEditedCopy(path, target.string(), params, &error))
            return std::string(tr("Could not save edited copy")) + (error.empty() ? "" : ": " + error);
        return std::string(tr("Copy saved")) + ": " + target.filename().string();
    }
    void selectPage(const std::string& id) { m_page = id; m_sidebar->setCurrent(id); m_headerTitle->setText(pageTitle(id)); m_headerTitle->setToolTip({}); refresh(); }
    void scanPictures() { const char* home = std::getenv("HOME"); if (home) m_scanner.scan(std::string(home) + "/Pictures"); }
    std::string activePageTitle() const
    {
        if (m_page.rfind("album:", 0) == 0) {
            const auto id = std::stoll(m_page.substr(6));
            const auto albums = m_store.albums();
            const auto found = std::find_if(albums.begin(), albums.end(),
                                            [id](const Album& album) { return album.id == id; });
            if (found != albums.end())
                return found->name;
        }
        return pageTitle(m_page);
    }
    void openViewer(size_t index, bool animate = true) {
        if (index >= m_visible.size()) return;
        m_viewerIndex = index;
        const Photo& photo = m_visible[index];
        m_viewer->setPhoto(photo);
        m_headerTitle->setText(photo.name);
        m_headerTitle->setToolTip(photo.path);
        m_owner.window().setTitle(photo.name);
        m_headerPrevious->setEnabled(index > 0);
        m_headerNext->setEnabled(index + 1 < m_visible.size());
        m_headerFavorite->setChecked(photo.favorite);
        m_headerFavorite->setToolTip(photo.favorite ? tr("Unfavorite") : tr("Favorite"));
        m_headerTrash->setToolTip(photo.trashed ? tr("Delete Permanently") : tr("Move to Trash"));
        m_headerRestore->setVisible(photo.trashed);
        m_headerFlip->setChecked(false);
        m_headerSlideshow->setChecked(m_viewer->isSlideShowActive());
        m_headerFullScreen->setChecked(m_owner.window().states().fullScreen);
        m_viewer->setVisible(true); m_owner.setViewerActive(true); m_editing = false; m_editButton->setChecked(false); m_viewer->setEditing(false);
        requestLayout();
        if (!animate || !window()) {
            m_viewer->setOpacity(1);
            m_viewer->setTransform({});
            return;
        }
        m_viewer->setOpacity(0);
        m_viewer->setTransform({{}, .96});
        m_transition = std::make_unique<Animation>(window());
        m_transition->setDuration(std::chrono::milliseconds(220));
        m_transition->setEasing(Animation::Easing::OutCubic);
        m_transition->onUpdate = [this](double value) { m_viewer->setOpacity(value); m_viewer->setTransform({{}, .96 + .04 * value}); };
        m_transition->start();
    }
    void closeViewer() {
        if (!m_viewer->isVisible()) return;
        m_viewer->setSlideShow(false);
        setEditing(false);
        if (m_owner.window().states().fullScreen)
            toggleFullScreen();
        if (!window()) { finishCloseViewer(); return; }
        m_transition = std::make_unique<Animation>(window());
        m_transition->setDuration(std::chrono::milliseconds(180));
        m_transition->setEasing(Animation::Easing::InOutCubic);
        m_transition->onUpdate = [this](double value) { m_viewer->setOpacity(1 - value); m_viewer->setTransform({{}, 1 - .04 * value}); };
        m_transition->onFinished = [this] { finishCloseViewer(); };
        m_transition->start();
    }
    void moveViewer(int delta) { const auto next = int(m_viewerIndex) + delta; if (next >= 0 && next < int(m_visible.size())) openViewer(size_t(next)); }
    void toggleViewerFavorite() {
        if (m_viewerIndex >= m_visible.size())
            return;
        const auto id = m_visible[m_viewerIndex].id;
        const bool wasFavorite = m_visible[m_viewerIndex].favorite;
        m_store.setFavorite(id, !wasFavorite);
        if (m_page == "favorites" && wasFavorite) {
            m_page = "photos";
            m_sidebar->setCurrent(m_page);
            m_headerTitle->setText(pageTitle(m_page));
        }
        refresh();
        const auto found = std::find_if(m_visible.begin(), m_visible.end(),
                                        [id](const Photo& photo) { return photo.id == id; });
        if (found != m_visible.end())
            openViewer(size_t(std::distance(m_visible.begin(), found)));
        else
            closeViewer();
    }
    void trashViewer() { if (m_viewerIndex < m_visible.size()) { if (m_page == "trash") m_store.purge(m_visible[m_viewerIndex].id); else m_store.setTrashed(m_visible[m_viewerIndex].id, true); closeViewer(); refresh(); } }
    void restoreViewer() { if (m_viewerIndex < m_visible.size()) { m_store.setTrashed(m_visible[m_viewerIndex].id, false); closeViewer(); refresh(); } }
    void setEditing(bool enabled) { m_editing = enabled; m_viewer->setEditing(enabled); m_editButton->setChecked(enabled); }
    void toggleFullScreen() {
        const bool fullScreen = !m_owner.window().states().fullScreen;
        m_owner.window().setFullScreen(fullScreen);
        m_headerFullScreen->setChecked(fullScreen);
    }
    void setSlideShow(bool enabled) {
        if (m_slideTimer) {
            Application::instance()->loop().stopTimer(m_slideTimer);
            m_slideTimer = 0;
        }
        if (enabled && window()) {
            m_slideTimer = Application::instance()->loop().startTimer(std::chrono::seconds(5), true, [this] {
                if (!m_viewer->isVisible() || m_visible.empty()) {
                    m_viewer->setSlideShow(false);
                    return;
                }
                openViewer((m_viewerIndex + 1) % m_visible.size());
            });
        }
        m_headerSlideshow->setChecked(enabled);
    }
    void finishCloseViewer() {
        m_viewer->setVisible(false);
        m_owner.setViewerActive(false);
        m_editing = false;
        m_viewer->setEditing(false);
        m_viewer->setOpacity(1);
        m_viewer->setTransform({});
        m_owner.window().setTitle(tr("Photos"));
        m_headerTitle->setText(activePageTitle());
        m_headerTitle->setToolTip({});
        requestLayout();
        if (m_refreshPending) {
            m_refreshPending = false;
            refresh();
        }
    }
    PhotosWindow& m_owner; PhotoStore& m_store; LibraryScanner& m_scanner; SidebarList* m_sidebar = nullptr; Label* m_appTitle = nullptr; WindowHeader* m_header = nullptr; View* m_sidebarGap = nullptr; Label* m_headerTitle = nullptr; RoundImageButton* m_viewerBack = nullptr; RoundImageButton* m_editButton = nullptr; RoundImageButton *m_headerPrevious = nullptr, *m_headerNext = nullptr, *m_headerZoomOut = nullptr, *m_headerZoomIn = nullptr, *m_headerReset = nullptr, *m_headerFlip = nullptr, *m_headerFullScreen = nullptr, *m_headerSlideshow = nullptr, *m_headerFavorite = nullptr, *m_headerRestore = nullptr, *m_headerTrash = nullptr; SearchField* m_search = nullptr; Button* m_import = nullptr; TextField* m_albumName = nullptr; Button* m_createAlbum = nullptr; Row* m_quickBar = nullptr; ScrollView* m_scroll = nullptr; Column* m_content = nullptr; Label* m_status = nullptr; Viewer* m_viewer = nullptr; std::unique_ptr<Menu> m_photoMenu; std::unique_ptr<Animation> m_transition; std::vector<Photo> m_visible; std::vector<Grid*> m_photoGrids; size_t m_viewerIndex = 0; int m_gridColumns = 4; int m_slideTimer = 0; std::string m_page; bool m_editing = false; bool m_refreshPending = false; std::vector<base::Subscription> m_subscriptions;
};
}
PhotosWindow::PhotosWindow(std::string database) : m_store(std::move(database)), m_scanner(m_store) {
    m_window = std::make_unique<Window>(); m_window->setTitle(tr("Photos")); m_window->setMinimumSize({860, 560}); m_window->resize({1120, 720});
    m_about = std::make_unique<AboutDialog>(m_window.get()); m_about->setName(tr("Photos")); m_about->setIconSource("photo"); m_about->setDescription(tr("Manage, browse and edit your photo library."));
    m_menu = std::make_unique<GlobalMenu>(Application::instance()->sessionBus(), [this](int32_t id) { if (id == 2) { const char* home = std::getenv("HOME"); if (home) m_scanner.scan(std::string(home) + "/Pictures"); } else if (id == 3) Application::instance()->quit(); else if (id == 5) showAbout(); else if (m_menuAction) m_menuAction(id); });
    m_root = new Root(*this, m_store, m_scanner); m_window->setContentView(std::unique_ptr<View>(m_root));
    if (m_window->backgroundGlassAvailable()) {
        m_window->setBackgroundColor(Color{0, 0, 0, 0});
        GlassMaterial glass;
        glass.blurSigma = 20;
        if (Theme::current().darkMode) { glass.tintRed = glass.tintGreen = 0.18; glass.tintBlue = 0.20; }
        m_window->setBackgroundGlass(glass);
    } else {
        m_window->setBackgroundGlass(std::nullopt);
        m_window->setBackgroundColor(Theme::current().background);
    }
    m_window->setBackgroundBlur({0, 0, 220, 720}, Theme::current().windowRadius);
    const char* home = std::getenv("HOME"); if (home) m_scanner.scan(std::string(home) + "/Pictures");
}
PhotosWindow::~PhotosWindow() = default;
void PhotosWindow::show() { m_window->show(); }
void PhotosWindow::showAbout() { m_about->show(); }
Window& PhotosWindow::window() { return *m_window; }
void PhotosWindow::openFile(const std::string& path)
{
    auto* root = static_cast<Root*>(m_root);
    std::error_code error;
    auto absolutePath = std::filesystem::absolute(path, error);
    if (error) {
        root->showOpenError(path + " (" + error.message() + ")");
        return;
    }
    absolutePath = absolutePath.lexically_normal();
    if (!std::filesystem::is_regular_file(absolutePath, error) || error) {
        root->showOpenError(path + (error ? " (" + error.message() + ")" : ""));
        return;
    }

    int width = 0;
    int height = 0;
    if (!gdk_pixbuf_get_file_info(absolutePath.c_str(), &width, &height)) {
        width = 0;
        height = 0;
    }

    const auto bytes = std::filesystem::file_size(absolutePath, error);
    if (error) {
        root->showOpenError(path + " (" + error.message() + ")");
        return;
    }
    const auto modified = std::filesystem::last_write_time(absolutePath, error);
    if (error) {
        root->showOpenError(path + " (" + error.message() + ")");
        return;
    }

    Photo photo;
    photo.path = absolutePath.string();
    photo.name = absolutePath.filename().string();
    photo.folder = absolutePath.parent_path().string();
    photo.captured = imageDate(modified);
    photo.format = absolutePath.extension().string();
    photo.width = width;
    photo.height = height;
    photo.bytes = static_cast<int64_t>(bytes);
    try {
        m_store.upsert(photo);
        const auto stored = m_store.photoByPath(photo.path);
        if (!stored) {
            root->showOpenError(path);
            return;
        }
        root->openPhoto(*stored);
    } catch (const std::exception& exception) {
        root->showOpenError(path + " (" + exception.what() + ")");
    }
}
void PhotosWindow::setViewerActive(bool active) { if (m_menu) m_menu->setViewerActive(active); }
void PhotosWindow::refresh() { if (m_root) m_root->requestLayout(); }
} // namespace Photos
