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
#include <ui/search_field.h>
#include <ui/scroll_view.h>
#include <ui/round_image_button.h>
#include <ui/text_field.h>
#include <ui/slider.h>
#include <ui/sidebar.h>
#include <ui/theme.h>
#include <ui/window.h>
#include <ui/window_header.h>
#include <cstdlib>
#include <filesystem>

namespace Photos {
using namespace Lingmo::ui;
using Lingmo::I18n::tr;
namespace {
class PhotoTile final : public View {
public:
    PhotoTile(Photo photo, std::function<void()> open)
        : m_photo(std::move(photo)), m_open(std::move(open)) {
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
    void pointerRelease(const PointerEvent& event) override { if (event.button == MouseButton::Left && m_open) m_open(); }
private:
    Photo m_photo; std::function<void()> m_open; ImageView* m_image = nullptr; Label* m_label = nullptr;
};

class EditPreviewOverlay final : public View {
public:
    void setAdjustments(double brightness, double contrast, double saturation, double temperature)
    {
        m_brightness = brightness;
        m_contrast = contrast;
        m_saturation = saturation;
        m_temperature = temperature;
        requestPaint();
    }

protected:
    void paint(Painter& painter) override
    {
        if (m_brightness < 0)
            painter.fillRect(localRect(), Color{0, 0, 0, static_cast<uint8_t>(-m_brightness * 120)});
        else if (m_brightness > 0)
            painter.fillRect(localRect(), Color{255, 255, 255, static_cast<uint8_t>(m_brightness * 100)});
        if (m_temperature < 0)
            painter.fillRect(localRect(), Color{35, 105, 220, static_cast<uint8_t>(-m_temperature * 70)});
        else if (m_temperature > 0)
            painter.fillRect(localRect(), Color{255, 145, 55, static_cast<uint8_t>(m_temperature * 70)});
        if (m_contrast != 0) {
            const auto alpha = static_cast<uint8_t>(std::abs(m_contrast) * 45);
            painter.drawSmoothRectangle(localRect(), Theme::current().mediumRadius,
                                        m_contrast > 0 ? Color{255, 255, 255, alpha} : Color{0, 0, 0, alpha});
        }
        if (m_saturation < 0)
            painter.fillRect(localRect(), Color{128, 128, 128, static_cast<uint8_t>(-m_saturation * 35)});
        else if (m_saturation > 0)
            painter.drawSmoothRectangle(localRect(), Theme::current().mediumRadius,
                                        Color{255, 40, 120, static_cast<uint8_t>(m_saturation * 25)});
    }

private:
    double m_brightness = 0;
    double m_contrast = 0;
    double m_saturation = 0;
    double m_temperature = 0;
};

class Viewer final : public View {
public:
    Viewer(std::function<void()> close, std::function<void()> previous, std::function<void()> next,
           std::function<void()> favorite, std::function<void()> trash,
           std::function<void(bool)> fullScreen, std::function<void(bool)> slideShow, std::function<void()> restore,
           std::function<void(const std::string&, const EditParams&)> exportCopy)
        : m_close(std::move(close)), m_previous(std::move(previous)), m_next(std::move(next)), m_favorite(std::move(favorite)), m_trash(std::move(trash)), m_fullScreen(std::move(fullScreen)), m_slideShow(std::move(slideShow)), m_restore(std::move(restore)), m_exportCopy(std::move(exportCopy)) {
        m_back = addChild<RoundImageButton>("chevron-left"); m_back->setToolTip(tr("Back")); m_back->onClicked = m_close;
        m_prev = addChild<RoundImageButton>("chevron-left"); m_prev->setToolTip(tr("Previous")); m_prev->onClicked = m_previous;
        m_nextButton = addChild<RoundImageButton>("chevron-right"); m_nextButton->setToolTip(tr("Next")); m_nextButton->onClicked = m_next;
        m_favoriteButton = addChild<RoundImageButton>("starred"); m_favoriteButton->setToolTip(tr("Favorite")); m_favoriteButton->onClicked = m_favorite;
        m_trashButton = addChild<RoundImageButton>("user-trash"); m_trashButton->setToolTip(tr("Move to Trash")); m_trashButton->onClicked = m_trash;
        m_zoomOut = addChild<RoundImageButton>("zoom-out"); m_zoomOut->setToolTip(tr("Zoom Out")); m_zoomOut->onClicked = [this] { setZoom(m_zoom - .15); };
        m_zoomIn = addChild<RoundImageButton>("zoom-in"); m_zoomIn->setToolTip(tr("Zoom In")); m_zoomIn->onClicked = [this] { setZoom(m_zoom + .15); };
        m_resetZoom = addChild<RoundImageButton>("zoom-fit-best"); m_resetZoom->setToolTip(tr("Reset Zoom")); m_resetZoom->onClicked = [this] { setZoom(1); };
        m_flip = addChild<RoundImageButton>("object-flip-horizontal"); m_flip->setToolTip(tr("Flip")); m_flip->onClicked = [this] { m_flipped = !m_flipped; updateTransform(); };
        m_fullScreenButton = addChild<RoundImageButton>("view-fullscreen"); m_fullScreenButton->setToolTip(tr("Full Screen")); m_fullScreenButton->onClicked = [this] { m_fullScreen(!m_fullScreenState); m_fullScreenState = !m_fullScreenState; };
        m_slideButton = addChild<RoundImageButton>("media-playback-start"); m_slideButton->setToolTip(tr("Slideshow")); m_slideButton->onClicked = [this] { m_sliding = !m_sliding; m_slideShow(m_sliding); };
        m_restoreButton = addChild<RoundImageButton>("view-refresh"); m_restoreButton->setToolTip(tr("Restore")); m_restoreButton->onClicked = m_restore;
        // Viewer actions live in the global menu. Keep only the navigation
        // affordance in the content area so the image remains the focus.
        m_back->setVisible(false); m_prev->setVisible(false); m_nextButton->setVisible(false);
        m_favoriteButton->setVisible(false); m_trashButton->setVisible(false);
        m_zoomOut->setVisible(false); m_zoomIn->setVisible(false);
        m_resetZoom->setVisible(false); m_flip->setVisible(false);
        m_fullScreenButton->setVisible(false); m_slideButton->setVisible(false);
        m_restoreButton->setVisible(false);
        m_image = addChild<ImageView>(); m_image->setFillMode(ImageView::FillMode::PreserveAspectFit); m_image->setAutoTransform(true);
        m_previewOverlay = addChild<EditPreviewOverlay>(); m_previewOverlay->setVisible(false);
        m_info = addChild<Label>(); m_info->setColor(ColorId::DisabledText); m_info->setWrap(true);
        m_editor = addChild<Column>(); m_editor->setSpacing(8); m_editor->setMargins({16, 12, 16, 12});
        m_editorTitle = m_editor->addChild<Label>(tr("Edit Photo")); m_editorTitle->setWeight(600);
        m_brightness = addSlider(tr("Brightness"), -1, 1, 0);
        m_contrast = addSlider(tr("Contrast"), -1, 1, 0);
        m_saturation = addSlider(tr("Saturation"), -1, 1, 0);
        m_temperature = addSlider(tr("Temperature"), -1, 1, 0);
        auto* rotate = m_editor->addChild<Button>(tr("Rotate 90°")); rotate->onClicked = [this] { m_rotation = (m_rotation + 1) % 4; updateEditInfo(); };
        auto* reset = m_editor->addChild<Button>(tr("Reset Adjustments")); reset->onClicked = [this] { m_brightness->setValue(0); m_contrast->setValue(0); m_saturation->setValue(0); m_temperature->setValue(0); m_rotation = 0; updateEditInfo(); };
        auto* save = m_editor->addChild<Button>(tr("Save Copy")); save->onClicked = [this] { if (m_exportCopy && !m_path.empty()) m_exportCopy(m_path, {m_brightness->value(), m_contrast->value(), m_saturation->value(), m_temperature->value(), m_rotation}); };
        m_editor->setVisible(false);
    }
    void setPhoto(const Photo& photo, size_t index, size_t count) {
        m_path = photo.path; m_sourceWidth = photo.width; m_sourceHeight = photo.height; m_photoInfo = photo.name + "\n" + photo.folder + "\n" + photo.captured + "\n" + photo.format + "  " + std::to_string(photo.bytes) + " bytes"; m_image->setSource(photo.path); m_image->setSourceSize({geometry().width * .72, geometry().height * .72}); m_info->setText(m_photoInfo); m_prev->setEnabled(index > 0); m_nextButton->setEnabled(index + 1 < count); m_favoriteButton->setToolTip(photo.favorite ? tr("Unfavorite") : tr("Favorite")); m_trashButton->setToolTip(photo.trashed ? tr("Delete Permanently") : tr("Move to Trash")); m_restoreButton->setVisible(photo.trashed); m_trashButton->setVisible(!photo.trashed); m_zoom = 1; m_flipped = false; m_editing = false; m_rotation = 0; m_brightness->setValue(0); m_contrast->setValue(0); m_saturation->setValue(0); m_temperature->setValue(0); updateTransform();
    }
    void setEditing(bool editing) { m_editing = editing; m_editor->setVisible(editing); m_previewOverlay->setVisible(editing && m_sourceWidth > 0 && m_sourceHeight > 0); updateEditInfo(); requestLayout(); }
    void zoomOut() { setZoom(m_zoom - .15); }
    void zoomIn() { setZoom(m_zoom + .15); }
    void resetZoom() { setZoom(1); }
    void flip() { m_flipped = !m_flipped; updateTransform(); }
    void toggleSlideShow() { m_sliding = !m_sliding; m_slideShow(m_sliding); }
protected:
    void layout() override {
        const double panel = m_editing ? 300 : 0; const RectF imageRect{24, 72, std::max(0.0, geometry().width - 300 - panel), std::max(0.0, geometry().height - 110)}; RectF pictureRect = imageRect; const double sourceWidth = m_sourceWidth > 0 ? m_sourceWidth : 1; const double sourceHeight = m_sourceHeight > 0 ? m_sourceHeight : 1; const double scale = std::min(imageRect.width / sourceWidth, imageRect.height / sourceHeight); pictureRect.width = m_sourceWidth > 0 && m_sourceHeight > 0 ? sourceWidth * scale : 0; pictureRect.height = m_sourceWidth > 0 && m_sourceHeight > 0 ? sourceHeight * scale : 0; pictureRect.x = imageRect.x + (imageRect.width - pictureRect.width) / 2; pictureRect.y = imageRect.y + (imageRect.height - pictureRect.height) / 2; m_back->setGeometry({24, 18, 80, 36}); m_prev->setGeometry({120, 18, 76, 36}); m_nextButton->setGeometry({200, 18, 76, 36}); m_zoomOut->setGeometry({300, 18, 82, 36}); m_zoomIn->setGeometry({388, 18, 82, 36}); m_resetZoom->setGeometry({476, 18, 72, 36}); m_flip->setGeometry({556, 18, 72, 36}); m_fullScreenButton->setGeometry({636, 18, 106, 36}); m_slideButton->setGeometry({750, 18, 110, 36}); m_restoreButton->setGeometry({geometry().width - 370, 18, 110, 36}); m_favoriteButton->setGeometry({geometry().width - 250, 18, 100, 36}); m_trashButton->setGeometry({geometry().width - 140, 18, 116, 36}); m_image->setGeometry(imageRect); m_previewOverlay->setGeometry(pictureRect); m_info->setGeometry({geometry().width - 250 - panel, 80, 220, 180}); m_editor->setGeometry({geometry().width - panel, 60, panel, std::max(0.0, geometry().height - 80)});
    }
    void paint(Painter& painter) override { painter.fillRect(localRect(), Theme::current().background); if (m_editing) painter.drawSmoothRectangle({geometry().width - 316, 52, 304, std::max(0.0, geometry().height - 64)}, Theme::current().hugeRadius, Theme::current().secondBackground); }
private:
    Slider* addSlider(const std::string& title, double from, double to, double value) { auto* label = m_editor->addChild<Label>(title); label->setColor(ColorId::DisabledText); auto* slider = m_editor->addChild<Slider>(); slider->setRange(from, to); slider->setValue(value); slider->setValueToolTip([title](double v) { return title + ": " + std::to_string(v); }); slider->onMoved = [this](double) { updateEditInfo(); }; return slider; }
    void updateEditInfo() { m_previewOverlay->setAdjustments(m_brightness->value(), m_contrast->value(), m_saturation->value(), m_temperature->value()); if (!m_editing) { m_info->setText(m_photoInfo); return; } m_info->setText(std::string(tr("Edit mode")) + "\n" + tr("Preview updates as you adjust the controls.") + "\n" + tr("Rotation") + ": " + std::to_string(m_rotation * 90) + "°\n" + tr("Brightness") + ": " + std::to_string(m_brightness->value()) + "\n" + tr("Contrast") + ": " + std::to_string(m_contrast->value()) + "\n" + tr("Saturation") + ": " + std::to_string(m_saturation->value()) + "\n" + tr("Temperature") + ": " + std::to_string(m_temperature->value()) + "\n\n" + m_photoInfo); }
    void setZoom(double value) { m_zoom = std::clamp(value, .5, 3.0); updateTransform(); }
    void updateTransform() { if (m_image) m_image->setTransform({{}, (m_flipped ? -1 : 1) * m_zoom}); }
    std::function<void()> m_close, m_previous, m_next, m_favorite, m_trash, m_restore; std::function<void(bool)> m_fullScreen, m_slideShow; std::function<void(const std::string&, const EditParams&)> m_exportCopy; RoundImageButton *m_back, *m_prev, *m_nextButton, *m_favoriteButton, *m_trashButton, *m_zoomOut, *m_zoomIn, *m_resetZoom, *m_flip, *m_fullScreenButton, *m_slideButton, *m_restoreButton; ImageView* m_image; EditPreviewOverlay* m_previewOverlay; Label* m_info; Column* m_editor; Label* m_editorTitle; Slider *m_brightness, *m_contrast, *m_saturation, *m_temperature; std::string m_photoInfo, m_path; int m_sourceWidth = 0, m_sourceHeight = 0, m_rotation = 0; double m_zoom = 1; bool m_flipped = false, m_fullScreenState = false, m_sliding = false, m_editing = false;
};

class Root final : public View {
public:
    Root(PhotosWindow& owner, PhotoStore& store, LibraryScanner& scanner) : m_owner(owner), m_store(store), m_scanner(scanner) {
        m_owner.setMenuAction([this](int id) { menuAction(id); });
        m_sidebar = addChild<SidebarList>(std::vector<SidebarList::Entry>{{"photos", tr("Photos"), "image-x-generic"}, {"recent", tr("Recently Added"), "document-open-recent"}, {"favorites", tr("Favorites"), "starred"}, {"albums", tr("Albums"), "folder"}, {"folders", tr("Folders"), "folder-open"}, {"trash", tr("Recently Deleted"), "user-trash"}});
        m_sidebar->onActivated = [this](const std::string& id) { m_page = id; m_headerTitle->setText(id == "photos" ? tr("Photos") : id == "recent" ? tr("Recently Added") : id == "favorites" ? tr("Favorites") : id == "albums" ? tr("Albums") : id == "folders" ? tr("Folders") : tr("Recently Deleted")); refresh(); };
        m_appTitle = addChild<Label>(tr("Photos")); m_appTitle->setFontSize(18); m_appTitle->setWeight(600); m_appTitle->setVerticallyCentered(true);
        m_header = addChild<WindowHeader>(46);
        auto* headerContent = m_header->setContent<Row>();
        auto* sidebarGap = headerContent->addChild<View>(); sidebarGap->setLayoutHints({.preferredWidth = 220});
        m_viewerBack = headerContent->addChild<RoundImageButton>("chevron-left", 28); m_viewerBack->setToolTip(tr("Back to Library")); m_viewerBack->onClicked = [this] { closeViewer(); }; m_viewerBack->setVisible(false);
        m_editButton = headerContent->addChild<RoundImageButton>("", 28); m_editButton->setSource("document-edit"); m_editButton->setIconMargins(4); m_editButton->setToolTip(tr("Edit Photo")); m_editButton->onClicked = [this] { m_editing = !m_editing; m_viewer->setEditing(m_editing); m_editButton->setChecked(m_editing); }; m_editButton->setVisible(false);
        m_headerTitle = headerContent->addChild<Label>(); m_headerTitle->setFontSize(17); m_headerTitle->setVerticallyCentered(true); m_headerTitle->setLayoutHints({.fillWidth = true});
        m_headerPrevious = headerContent->addChild<RoundImageButton>("chevron-left", 24); m_headerPrevious->setToolTip(tr("Previous")); m_headerPrevious->onClicked = [this] { moveViewer(-1); }; m_headerPrevious->setVisible(false);
        m_headerNext = headerContent->addChild<RoundImageButton>("chevron-right", 24); m_headerNext->setToolTip(tr("Next")); m_headerNext->onClicked = [this] { moveViewer(1); }; m_headerNext->setVisible(false);
        m_headerZoomOut = headerContent->addChild<RoundImageButton>("", 24); m_headerZoomOut->setSource("zoom-out"); m_headerZoomOut->setIconMargins(4); m_headerZoomOut->setToolTip(tr("Zoom Out")); m_headerZoomOut->onClicked = [this] { m_viewer->zoomOut(); }; m_headerZoomOut->setVisible(false);
        m_headerZoomIn = headerContent->addChild<RoundImageButton>("", 24); m_headerZoomIn->setSource("zoom-in"); m_headerZoomIn->setIconMargins(4); m_headerZoomIn->setToolTip(tr("Zoom In")); m_headerZoomIn->onClicked = [this] { m_viewer->zoomIn(); }; m_headerZoomIn->setVisible(false);
        m_headerReset = headerContent->addChild<RoundImageButton>("refresh", 24); m_headerReset->setToolTip(tr("Reset Zoom")); m_headerReset->onClicked = [this] { m_viewer->resetZoom(); }; m_headerReset->setVisible(false);
        m_headerFlip = headerContent->addChild<RoundImageButton>("", 24); m_headerFlip->setSource("object-flip-horizontal"); m_headerFlip->setIconMargins(4); m_headerFlip->setToolTip(tr("Flip")); m_headerFlip->onClicked = [this] { m_viewer->flip(); }; m_headerFlip->setVisible(false);
        m_headerFullScreen = headerContent->addChild<RoundImageButton>("", 24); m_headerFullScreen->setSource("view-fullscreen"); m_headerFullScreen->setIconMargins(4); m_headerFullScreen->setToolTip(tr("Full Screen")); m_headerFullScreen->onClicked = [this] { m_owner.window().setFullScreen(!m_owner.window().states().fullScreen); }; m_headerFullScreen->setVisible(false);
        m_headerSlideshow = headerContent->addChild<RoundImageButton>("", 24); m_headerSlideshow->setSource("media-playback-start"); m_headerSlideshow->setIconMargins(4); m_headerSlideshow->setToolTip(tr("Slideshow")); m_headerSlideshow->onClicked = [this] { m_viewer->toggleSlideShow(); }; m_headerSlideshow->setVisible(false);
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
        m_viewer = addChild<Viewer>([this] { closeViewer(); }, [this] { moveViewer(-1); }, [this] { moveViewer(1); }, [this] { toggleViewerFavorite(); }, [this] { trashViewer(); }, [this](bool enabled) { m_owner.window().setFullScreen(enabled); }, [this](bool enabled) { setSlideShow(enabled); }, [this] { restoreViewer(); }, [this](const std::string& path, const EditParams& params) { exportCopy(path, params); }); m_viewer->setVisible(false);
        m_subscriptions.push_back(m_scanner.changed.connect([this] { refresh(); }));
        m_page = "photos"; refresh();
    }
    void menuAction(int id) {
        if (!m_viewer->isVisible()) return;
        switch (id) { case 7: closeViewer(); break; case 8: moveViewer(-1); break; case 9: moveViewer(1); break; case 10: m_viewer->zoomOut(); break; case 11: m_viewer->zoomIn(); break; case 12: m_viewer->resetZoom(); break; case 13: m_viewer->flip(); break; case 14: m_owner.window().setFullScreen(!m_owner.window().states().fullScreen); break; case 15: m_viewer->toggleSlideShow(); break; case 16: toggleViewerFavorite(); break; case 17: trashViewer(); break; case 18: restoreViewer(); break; case 20: m_editing = !m_editing; m_viewer->setEditing(m_editing); if (m_editButton) m_editButton->setChecked(m_editing); break; default: break; }
    }
    void layout() override {
        const double side = 220; const bool home = m_page == "photos"; const bool albums = m_page == "albums"; const bool browsing = !m_viewer->isVisible(); m_header->setGeometry({0, 0, geometry().width, 46}); m_appTitle->setGeometry({20, 0, side - 40, 46}); m_sidebar->setGeometry({0, 46, side, std::max(0.0, geometry().height - 46)}); m_search->setGeometry({std::max(side + 320.0, geometry().width - 360), 56, 240, 36}); m_import->setGeometry({std::max(side + 570.0, geometry().width - 110), 56, 82, 36}); m_albumName->setGeometry({side + 28, 56, 240, 36}); m_createAlbum->setGeometry({side + 276, 56, 116, 36}); m_quickBar->setGeometry({side + 136, std::max(0.0, geometry().height - 34), 136, 28}); m_scroll->setVisible(browsing); m_search->setVisible(browsing && !albums); m_import->setVisible(browsing && !albums); m_albumName->setVisible(browsing && albums); m_createAlbum->setVisible(browsing && albums); m_quickBar->setVisible(browsing && home); m_status->setVisible(browsing); m_viewerBack->setVisible(!browsing); m_editButton->setVisible(!browsing); for (auto* action : {m_headerPrevious, m_headerNext, m_headerZoomOut, m_headerZoomIn, m_headerReset, m_headerFlip, m_headerFullScreen, m_headerSlideshow}) action->setVisible(!browsing); m_scroll->setGeometry({side, 102, std::max(0.0, geometry().width - side), std::max(0.0, geometry().height - 148)}); m_status->setGeometry({side + 28, std::max(0.0, geometry().height - 30), 96, 24}); m_viewer->setGeometry({side, 46, std::max(0.0, geometry().width - side), std::max(0.0, geometry().height - 46)});
    }
    void paint(Painter& painter) override {
        const auto& theme = Theme::current();
        const bool glass = window() && window()->backgroundGlassAvailable();
        const double side = 220;
        painter.fillRect({side, 0, std::max(0.0, geometry().width - side), geometry().height}, theme.background);
        painter.fillRect({0, 0, side, geometry().height}, theme.secondBackground.withAlpha(glass ? .72 : 1));
        painter.fillRect({side, 0, 1, geometry().height}, theme.text.withAlpha(.08));
    }
private:
    void refresh() {
        while (!m_content->children().empty()) m_content->takeChild(m_content->children().front().get());
        const bool album = m_page.rfind("album:", 0) == 0;
        auto photos = album ? m_store.albumPhotos(std::stoll(m_page.substr(6))) : m_search->text().empty() ? (m_page == "favorites" ? m_store.photos(true) : m_page == "trash" ? m_store.photos(false, true) : m_page == "folders" ? m_store.folder(std::getenv("HOME") ? std::string(std::getenv("HOME")) + "/Pictures" : std::string()) : m_store.photos()) : m_store.search(m_search->text());
        m_visible = photos;
        if (m_page == "albums") {
            for (const auto& item : m_store.albums()) {
                auto* row = m_content->addChild<Row>(); row->setSpacing(8);
                auto* name = row->addChild<TextField>(item.name); name->setLayoutHints({.fillWidth = true}); name->onAccepted = [this, id = item.id, name] { if (!name->text().empty()) { m_store.renameAlbum(id, name->text()); refresh(); } };
                auto* open = row->addChild<Button>(tr("Open")); open->onClicked = [this, item] { m_page = "album:" + std::to_string(item.id); m_sidebar->setCurrent("albums"); m_headerTitle->setText(item.name); refresh(); };
                auto* remove = row->addChild<Button>(tr("Delete")); remove->onClicked = [this, id = item.id] { m_store.deleteAlbum(id); refresh(); };
            }
        }
        std::map<std::string, Grid*> groups;
        for (size_t index = 0; index < photos.size(); ++index) { auto photo = photos[index]; const std::string key = photo.captured.empty() ? tr("Undated") : photo.captured; Grid* group = groups.count(key) ? groups[key] : nullptr; if (!group) { m_content->addChild<Label>(key)->setWeight(600); group = m_content->addChild<Grid>(5); group->setColumnSpacing(12); group->setRowSpacing(12); groups.emplace(key, group); } group->addChild<PhotoTile>(photo, [this, index] { openViewer(index); }); }
        m_status->setText(std::to_string(photos.size()) + " " + tr("photos")); requestLayout(); requestPaint();
    }
    void createAlbum() { const auto name = m_albumName->text(); if (name.empty()) return; if (m_store.createAlbum(name) != 0) { m_albumName->clear(); refresh(); } }
    void exportCopy(const std::string& path, const EditParams& params) { std::filesystem::path source(path); std::filesystem::path target = source.parent_path() / (source.stem().string() + " (edited).png"); std::string error; saveEditedCopy(path, target.string(), params, &error); }
    void selectPage(const std::string& id) { m_page = id; m_sidebar->setCurrent(id); m_headerTitle->setText(id == "recent" ? tr("Recently Added") : id == "favorites" ? tr("Favorites") : id == "albums" ? tr("Albums") : tr("Recently Deleted")); refresh(); }
    void scanPictures() { const char* home = std::getenv("HOME"); if (home) m_scanner.scan(std::string(home) + "/Pictures"); }
    void openViewer(size_t index) {
        if (index >= m_visible.size()) return;
        m_viewerIndex = index;
        m_viewer->setPhoto(m_visible[index], index, m_visible.size());
        m_viewer->setVisible(true); m_owner.setViewerActive(true); m_editing = false; m_viewer->setEditing(false);
        m_viewer->setOpacity(0);
        m_viewer->setTransform({{}, .96});
        requestLayout();
        if (!window()) { m_viewer->setOpacity(1); m_viewer->setTransform({}); return; }
        m_transition = std::make_unique<Animation>(window());
        m_transition->setDuration(std::chrono::milliseconds(220));
        m_transition->setEasing(Animation::Easing::OutCubic);
        m_transition->onUpdate = [this](double value) { m_viewer->setOpacity(value); m_viewer->setTransform({{}, .96 + .04 * value}); };
        m_transition->start();
    }
    void closeViewer() {
        if (!m_viewer->isVisible()) return;
        if (!window()) { m_viewer->setVisible(false); m_owner.setViewerActive(false); m_editing = false; m_viewer->setEditing(false); requestLayout(); return; }
        m_transition = std::make_unique<Animation>(window());
        m_transition->setDuration(std::chrono::milliseconds(180));
        m_transition->setEasing(Animation::Easing::InOutCubic);
        m_transition->onUpdate = [this](double value) { m_viewer->setOpacity(1 - value); m_viewer->setTransform({{}, 1 - .04 * value}); };
        m_transition->onFinished = [this] { m_viewer->setVisible(false); m_owner.setViewerActive(false); m_editing = false; m_viewer->setEditing(false); m_viewer->setOpacity(1); m_viewer->setTransform({}); requestLayout(); };
        m_transition->start();
    }
    void moveViewer(int delta) { const auto next = int(m_viewerIndex) + delta; if (next >= 0 && next < int(m_visible.size())) openViewer(size_t(next)); }
    void toggleViewerFavorite() { if (m_viewerIndex < m_visible.size()) { m_store.setFavorite(m_visible[m_viewerIndex].id, !m_visible[m_viewerIndex].favorite); refresh(); openViewer(m_viewerIndex); } }
    void trashViewer() { if (m_viewerIndex < m_visible.size()) { if (m_page == "trash") m_store.purge(m_visible[m_viewerIndex].id); else m_store.setTrashed(m_visible[m_viewerIndex].id, true); closeViewer(); refresh(); } }
    void restoreViewer() { if (m_viewerIndex < m_visible.size()) { m_store.setTrashed(m_visible[m_viewerIndex].id, false); closeViewer(); refresh(); } }
    void setSlideShow(bool enabled) { if (enabled && window()) { m_slideTimer = Application::instance()->loop().startTimer(std::chrono::seconds(5), true, [this] { moveViewer(1); }); } else if (m_slideTimer) { Application::instance()->loop().stopTimer(m_slideTimer); m_slideTimer = 0; } }
    PhotosWindow& m_owner; PhotoStore& m_store; LibraryScanner& m_scanner; SidebarList* m_sidebar = nullptr; Label* m_appTitle = nullptr; WindowHeader* m_header = nullptr; Label* m_headerTitle = nullptr; RoundImageButton* m_viewerBack = nullptr; RoundImageButton* m_editButton = nullptr; RoundImageButton *m_headerPrevious = nullptr, *m_headerNext = nullptr, *m_headerZoomOut = nullptr, *m_headerZoomIn = nullptr, *m_headerReset = nullptr, *m_headerFlip = nullptr, *m_headerFullScreen = nullptr, *m_headerSlideshow = nullptr; SearchField* m_search = nullptr; Button* m_import = nullptr; TextField* m_albumName = nullptr; Button* m_createAlbum = nullptr; Row* m_quickBar = nullptr; ScrollView* m_scroll = nullptr; Column* m_content = nullptr; Label* m_status = nullptr; Viewer* m_viewer = nullptr; std::unique_ptr<Animation> m_transition; std::vector<Photo> m_visible; size_t m_viewerIndex = 0; int m_slideTimer = 0; std::string m_page; bool m_editing = false; std::vector<base::Subscription> m_subscriptions;
};
}
PhotosWindow::PhotosWindow(std::string database) : m_store(std::move(database)), m_scanner(m_store) {
    m_window = std::make_unique<Window>(); m_window->setTitle(tr("Photos")); m_window->setMinimumSize({860, 560}); m_window->resize({1120, 720});
    m_about = std::make_unique<AboutDialog>(m_window.get()); m_about->setName(tr("Photos")); m_about->setIconSource("lingmo-photos"); m_about->setDescription(tr("Manage, browse and edit your photo library."));
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
void PhotosWindow::setViewerActive(bool active) { if (m_menu) m_menu->setViewerActive(active); }
void PhotosWindow::refresh() { if (m_root) m_root->requestLayout(); }
} // namespace Photos
