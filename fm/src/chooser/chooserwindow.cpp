// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chooser/chooserwindow.h"
#include "chooser/filechooser.h"
#include "dialogs/unlockdialog.h"
#include "model/pathbarmodel.h"
#include "model/places.h"
#include "views/colors.h"
#include "views/completionpopup.h"
#include "views/listview.h"
#include "views/pathbar.h"
#include "views/style.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/url.h"
#include "ui/application.h"
#include "ui/check_box.h"
#include "ui/color_provider.h"
#include "ui/combo_box.h"
#include "ui/dialog_window.h"
#include "ui/header_button.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/progress_bar.h"
#include "ui/cell_layout.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>

namespace fm {

using ui::rounding::centeredStart;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

static constexpr double kWidth = 860;
static constexpr double kHeight = 520;
static constexpr double kSideBarWidth = 180;
static constexpr double kSmallSpacing = 6;
static constexpr double kLargeSpacing = 12;
static constexpr double kFieldHeight = 30;

// The chooser's places: favourites, disks and local mounts; remote places
// cannot be handed to the portal as local paths.
class ChooserPlaces : public ui::View
{
public:
    ChooserPlaces(FileChooser& chooser, PlacesModel& places)
        : m_chooser(chooser)
        , m_places(places)
    {
        rebuild();
    }

    bool acceptsPointer() const override { return true; }

    void rebuild()
    {
        for (const auto& child : children())
            child->setVisible(false);
        m_rows.clear();
        std::vector<std::shared_ptr<View>> gone;
        while (!children().empty())
            gone.push_back(takeChild(children().front().get()));
        base::EventLoop::current()->post([gone] {});

        m_favorites = addChild<ui::Label>(tr("Favorites"));
        m_favorites->setWeight(700);
        m_favorites->setFontSize(9 * 4.0 / 3);
        std::string category = "Favorites";
        for (int i = 0; i < m_places.count(); ++i) {
            const PlacesModel::Place& place = m_places.at(i);
            Row row;
            row.index = i;
            if (place.category != category) {
                category = place.category;
                if (!category.empty()) {
                    row.section = addChild<ui::Label>(tr(category.c_str()));
                    row.section->setWeight(700);
                    row.section->setFontSize(9 * 4.0 / 3);
                }
            }
            row.usable = place.url.starts_with("file:") || place.setupNeeded();
            row.icon = addChild<ui::ThemeIcon>(place.iconName, 20);
            row.icon->setThemeSize(20);
            row.label = addChild<ui::Label>(place.displayName());
            row.label->setElide(ui::Label::Elide::Right);
            row.icon->setVisible(row.usable);
            row.label->setVisible(row.usable);
            m_rows.push_back(row);
        }
        requestLayout();
        requestPaint();
    }

protected:
    double rowHeight() const { return std::max(34.0, style::fontHeight() + 12); }

    void layout() override
    {
        const ui::Theme& theme = ui::Theme::current();
        const double width = geometry().width;
        const double inset = kSmallSpacing * 1.5;
        double y = kLargeSpacing;
        m_favorites->setColor(theme.disabledText);
        m_favorites->setGeometry({inset + kSmallSpacing, y + centeredStart(32, m_favorites->sizeHint().height),
                                  width - inset - kSmallSpacing, m_favorites->sizeHint().height});
        y += 32;
        const std::string current = m_chooser.directory();
        for (Row& row : m_rows) {
            if (row.section) {
                row.section->setColor(theme.disabledText);
                const double height = row.section->sizeHint().height;
                row.section->setGeometry({inset + kSmallSpacing, y + centeredStart(36, height),
                                          width - inset - kSmallSpacing, height});
                y += 36;
            }
            const double height = row.usable ? rowHeight() : 0;
            row.rect = {0, y, width, height};
            const PlacesModel::Place& place = m_places.at(row.index);
            row.checked = place.path() == current;
            const ui::Color color = row.checked ? theme.highlight : theme.text;
            if (row.usable) {
                row.icon->setColor(color);
                row.icon->setGeometry({inset + kSmallSpacing, y + centeredStart(height, 20), 20, 20});
                row.label->setColor(color);
                const double labelX = inset + kSmallSpacing + 20 + kSmallSpacing;
                const double labelHeight = row.label->sizeHint().height;
                row.label->setGeometry({labelX, y + centeredStart(height, labelHeight),
                                        std::max(0.0, width - inset - kSmallSpacing - labelX), labelHeight});
            }
            // A ListView spaces its rows, the hidden ones too.
            y += height + 3;
        }
    }

    void paint(ui::Painter& painter) override
    {
        const ui::Theme& theme = ui::Theme::current();
        const double inset = kSmallSpacing * 1.5;
        for (const Row& row : m_rows) {
            if (!row.usable)
                continue;
            ui::Color fill {0, 0, 0, 0};
            if (&row - m_rows.data() == m_pressed)
                fill = ui::color(ui::ColorId::ItemPressed);
            else if (row.checked)
                fill = style::textAlpha(0.05);
            if (fill.a)
                painter.drawSmoothRectangle({inset, row.rect.y, row.rect.width - inset * 2, row.rect.height},
                                           theme.mediumRadius, fill);
        }
    }

    void pointerPress(const ui::PointerEvent& event) override
    {
        m_pressed = rowAt(event.position);
        requestPaint();
    }

    void pointerRelease(const ui::PointerEvent& event) override
    {
        const int pressed = m_pressed;
        m_pressed = -1;
        requestPaint();
        if (pressed < 0 || rowAt(event.position) != pressed)
            return;
        const int index = m_rows[size_t(pressed)].index;
        const PlacesModel::Place& place = m_places.at(index);
        if (place.setupNeeded())
            m_places.requestSetup(index);
        else
            m_chooser.navigate(place.path());
    }

private:
    struct Row
    {
        int index = 0;
        bool usable = false;
        bool checked = false;
        ui::Label* section = nullptr;
        ui::ThemeIcon* icon = nullptr;
        ui::Label* label = nullptr;
        ui::RectF rect;
    };

    int rowAt(ui::PointF point) const
    {
        for (size_t i = 0; i < m_rows.size(); ++i) {
            if (m_rows[i].usable && m_rows[i].rect.contains(point))
                return int(i);
        }
        return -1;
    }

    FileChooser& m_chooser;
    PlacesModel& m_places;
    ui::Label* m_favorites = nullptr;
    std::vector<Row> m_rows;
    int m_pressed = -1;
};

// The location field: closes when it loses the keys or Escape is pressed.
class LocationField : public ui::TextField
{
public:
    std::function<bool(const ui::KeyEvent&)> onKey;
    std::function<void()> onFocusLost;

protected:
    bool keyPress(const ui::KeyEvent& event) override
    {
        if (onKey && onKey(event))
            return true;
        return TextField::keyPress(event);
    }

    void focusOut() override
    {
        TextField::focusOut();
        if (onFocusLost)
            onFocusLost();
    }
};

// The file manager's breadcrumbs, or the location field in their place;
// dragging them moves the window, which has no title bar.
class ChooserPathBar : public ui::View
{
public:
    explicit ChooserPathBar(FileChooser& chooser)
        : m_chooser(chooser)
    {
        crumbs = addChild<Breadcrumbs>(false);
        crumbs->onItemClicked = [this](const std::string& path) { m_chooser.navigate(path); };
        crumbs->onEmptyClicked = [this] { openEditor(); };
        crumbs->onDragStarted = [this] {
            if (window())
                window()->startMove();
        };
        editor = addChild<LocationField>();
        editor->setName("locationField");
        editor->setSelectByMouse(true);
        editor->setInputPurpose(ui::TextPurpose::Url);
        editor->setVisible(false);
        editor->onAccepted = [this] { m_chooser.navigate(editor->text()); };
        editor->onFocusLost = [this] {
            completion->hide();
            closeEditor();
        };
        editor->onKey = [this](const ui::KeyEvent& event) {
            if (completion->handleKey(event, true))
                return true;
            if (event.key == ui::key::Escape) {
                closeEditor();
                return true;
            }
            return false;
        };
        editor->onTextEdited = [this] { completion->edited(true); };
        editor->onTextChanged = [this] { completion->textChanged(); };
        completion = std::make_unique<CompletionPopup>(editor);
        completion->onActivated = [this](const std::string& path) { m_chooser.navigate(path); };
    }

    void setUrl(const std::string& url)
    {
        std::string resolved;
        crumbs->setCrumbs(pathCrumbs(url, &resolved));
        m_url = resolved;
    }

    void openEditor()
    {
        completion->reset();
        editor->setText(m_url);
        editor->setVisible(true);
        crumbs->setVisible(false);
        editor->setFocus();
        editor->selectAll();
    }

    void closeEditor()
    {
        if (!editor->isVisible())
            return;
        editor->setVisible(false);
        crumbs->setVisible(true);
        if (onClosed)
            onClosed();
    }

    bool isEditing() const { return editor->isVisible(); }

    Breadcrumbs* crumbs;
    LocationField* editor;
    std::unique_ptr<CompletionPopup> completion;
    std::function<void()> onClosed;

protected:
    void layout() override
    {
        crumbs->setGeometry(localRect());
        editor->setGeometry(localRect());
    }

private:
    FileChooser& m_chooser;
    std::string m_url;
};

class ChooserRoot : public ui::View
{
public:
    ChooserRoot(ChooserWindow* window, FileChooser& chooser)
        : m_owner(window)
        , m_chooser(chooser)
    {
        m_places = std::make_unique<PlacesModel>();
        places = addChild<ChooserPlaces>(chooser, *m_places);

        back = addChild<ui::HeaderButton>(std::string(), 30);
        back->setGlyph("chevron-left", 16);
        back->onClicked = [this] { m_chooser.back(); };
        forward = addChild<ui::HeaderButton>(std::string(), 30);
        forward->setGlyph("chevron-right", 16);
        forward->onClicked = [this] { m_chooser.forward(); };
        for (ui::HeaderButton* button : {back, forward})
            button->setPlateAlphas({0.17, 0.26}, {0.21, 0.15});
        pathBar = addChild<ChooserPathBar>(chooser);
        pathBar->onClosed = [this] { list->contentView()->setFocus(); };

        list = addChild<FolderListView>(chooser.folder());
        list->setName("fileList");
        list->setChooserMode(chooser.multiple());
        list->setBackdropBase(ui::ColorId::Background);
        list->onActivated = [this](int row) { m_chooser.activate(row); };
        list->onKeyPressed = [this](const ui::KeyEvent& event) { return listKey(event); };
        busy = addChild<ui::BusyIndicator>();
        busy->setVisible(false);

        saveAsLabel = addChild<ui::Label>(tr("Save As:"));
        nameField = addChild<ui::TextField>(chooser.fileName());
        nameField->setName("fileNameField");
        nameField->setSelectByMouse(true);
        nameField->onTextEdited = [this] { m_chooser.setFileName(nameField->text()); };
        nameField->onAccepted = [this] { m_chooser.accept(); };
        filterLabel = addChild<ui::Label>(tr("File Type:"));
        std::vector<std::string> filterNames;
        for (const base::json::Value& filter : chooser.filters())
            filterNames.push_back(filter["name"].toString());
        filterCombo = addChild<ui::ComboBox>(filterNames);
        filterCombo->setName("filterCombo");
        filterCombo->setCurrentIndex(chooser.filterIndex());
        filterCombo->onActivated = [this](int index) { m_chooser.setFilterIndex(index); };

        for (const base::json::Value& choice : chooser.choices()) {
            ChoiceRow row;
            const std::string id = choice["id"].toString();
            const base::json::Value::Array& options = choice["options"].toArray();
            if (options.empty()) {
                row.check = addChild<ui::CheckBox>(choice["label"].toString());
                row.check->setChecked(choice["selected"].toString() == "true");
                row.check->onToggled = [this, id](bool checked) { m_chooser.setChoice(id, checked ? "true" : "false"); };
            } else {
                row.label = addChild<ui::Label>(choice["label"].toString());
                std::vector<std::string> labels;
                std::vector<std::string> ids;
                int selected = 0;
                for (const base::json::Value& option : options) {
                    if (option["id"].toString() == choice["selected"].toString())
                        selected = int(labels.size());
                    labels.push_back(option["label"].toString());
                    ids.push_back(option["id"].toString());
                }
                row.combo = addChild<ui::ComboBox>(labels);
                row.combo->setCurrentIndex(selected);
                row.combo->onActivated = [this, id, ids](int index) {
                    if (index >= 0 && index < int(ids.size()))
                        m_chooser.setChoice(id, ids[size_t(index)]);
                };
            }
            m_choices.push_back(row);
        }

        error = addChild<ui::Label>();
        error->setWrap(true);
        error->setVisible(false);

        newFolder = addChild<ui::DialogButton>(tr("New Folder"));
        newFolder->setVisible(canCreateFolder());
        newFolder->onClicked = [this] { m_owner->beginNewFolder(); };
        summary = addChild<ui::Label>();
        summary->setElide(ui::Label::Elide::Middle);
        summary->setVisible(chooser.multiple());
        cancel = addChild<ui::DialogButton>(tr("Cancel"));
        cancel->setName("cancelButton");
        cancel->onClicked = [this] { m_chooser.cancel(); };
        accept = addChild<ui::DialogButton>(chooser.acceptLabel(), true);
        accept->setName("acceptButton");
        accept->onClicked = [this] { m_chooser.accept(); };

        m_subscriptions.push_back(chooser.locationChanged.connect([this] { locationChanged(); }));
        m_subscriptions.push_back(chooser.fileNameChanged.connect([this] {
            if (nameField->text() != m_chooser.fileName())
                nameField->setText(m_chooser.fileName());
        }));
        m_subscriptions.push_back(chooser.selectionChanged.connect([this] { summary->setText(m_chooser.selectionSummary()); }));
        m_subscriptions.push_back(chooser.errorChanged.connect([this] {
            error->setText(m_chooser.error());
            error->setVisible(!m_chooser.error().empty());
            requestLayout();
        }));
        m_subscriptions.push_back(chooser.folder().statusChanged.connect([this] { updateBusy(); }));
        m_subscriptions.push_back(m_places->changed.connect([this] { places->rebuild(); }));
        m_subscriptions.push_back(m_places->deviceSetupDone.connect([this](const std::string& path) { m_chooser.navigate(path); }));
        m_subscriptions.push_back(m_places->passphraseRequested.connect([this](std::string udi, std::string name, bool retry) {
            m_owner->m_unlock = std::make_unique<UnlockVolumeDialog>(this->window(), *m_places, std::move(udi),
                                                                     std::move(name), retry);
            m_owner->m_unlock->show();
        }));
        m_subscriptions.push_back(m_places->deviceOperationFailed.connect([this](const std::string& message) {
            m_owner->m_deviceError = std::make_unique<ui::ConfirmDialog>(this->window(), std::string(), message);
            m_owner->deviceErrorTexts();
            m_owner->m_deviceError->setCancelVisible(false);
            m_owner->m_deviceError->show();
        }));
        locationChanged();
        summary->setText(chooser.selectionSummary());
    }

    bool canCreateFolder() const { return m_chooser.saveMode() || m_chooser.directoryMode(); }
    bool acceptsPointer() const override { return true; }

    // The location, the list's selection, the name typed and the filter stay.
    void retranslate()
    {
        saveAsLabel->setText(tr("Save As:"));
        filterLabel->setText(tr("File Type:"));
        newFolder->setText(tr("New Folder"));
        cancel->setText(tr("Cancel"));
        accept->setText(m_chooser.acceptLabel());
        summary->setText(m_chooser.selectionSummary());
        m_chooser.folder().retranslate();
        list->retranslate();
        m_places->retranslate();
        pathBar->setUrl(m_chooser.folder().url());
        requestLayout();
    }

    void focusFirst()
    {
        // Saving starts in the name with its stem selected, so typing renames and Enter saves.
        if (m_chooser.saveMode()) {
            nameField->setFocus();
            const std::string& text = nameField->text();
            const size_t dot = text.rfind('.');
            nameField->select(0, dot != std::string::npos && dot > 0 ? dot : text.size());
        } else {
            list->contentView()->setFocus();
        }
    }

    ChooserPlaces* places;
    ui::HeaderButton* back;
    ui::HeaderButton* forward;
    ChooserPathBar* pathBar;
    FolderListView* list;
    ui::BusyIndicator* busy;
    ui::Label* saveAsLabel;
    ui::TextField* nameField;
    ui::Label* filterLabel;
    ui::ComboBox* filterCombo;
    ui::Label* error;
    ui::DialogButton* newFolder;
    ui::Label* summary;
    ui::DialogButton* cancel;
    ui::DialogButton* accept;

protected:
    void layout() override
    {
        const ui::Theme& theme = ui::Theme::current();
        const ui::RectF rect = localRect();
        places->setGeometry({0, 0, kSideBarWidth, rect.height});

        const double left = kSideBarWidth;
        const double width = rect.width - left;
        const double inner = width - 2 * kLargeSpacing;

        // The header row.
        double y = kLargeSpacing;
        back->setEnabled(m_chooser.folder().canGoBack());
        forward->setEnabled(m_chooser.folder().canGoForward());
        double x = left + kLargeSpacing;
        back->setGeometry({x, y, 30, 30});
        x += 30 + kSmallSpacing / 2;
        forward->setGeometry({x, y, 30, 30});
        x += 30 + kSmallSpacing / 2 + kSmallSpacing / 2;
        pathBar->setGeometry({x, y, left + width - kLargeSpacing - x, 30});
        y += 30 + kLargeSpacing;

        // From the bottom: the buttons, the error, the fields.
        double bottom = rect.height - kLargeSpacing;
        const double buttonHeight = std::max({cancel->sizeHint().height, accept->sizeHint().height,
                                              newFolder->isVisible() ? newFolder->sizeHint().height : 0.0});
        const double buttonsY = bottom - buttonHeight;
        double right = left + width - kLargeSpacing;
        const ui::SizeF acceptSize = accept->sizeHint();
        accept->setGeometry({right - acceptSize.width, buttonsY + centeredStart(buttonHeight, acceptSize.height),
                             acceptSize.width, acceptSize.height});
        right -= acceptSize.width + kSmallSpacing;
        const ui::SizeF cancelSize = cancel->sizeHint();
        cancel->setGeometry({right - cancelSize.width, buttonsY + centeredStart(buttonHeight, cancelSize.height),
                             cancelSize.width, cancelSize.height});
        right -= cancelSize.width + kSmallSpacing;
        double buttonX = left + kLargeSpacing;
        if (newFolder->isVisible()) {
            const ui::SizeF size = newFolder->sizeHint();
            newFolder->setGeometry({buttonX, buttonsY + centeredStart(buttonHeight, size.height), size.width,
                                    size.height});
            buttonX += size.width + kSmallSpacing;
        }
        if (summary->isVisible()) {
            summary->setColor(theme.disabledText);
            const double summaryX = buttonX + kSmallSpacing;
            const double height = summary->sizeHint().height;
            summary->setGeometry({summaryX, buttonsY + centeredStart(buttonHeight, height),
                                  std::max(0.0, std::min(summary->sizeHint().width, right - summaryX)), height});
        }
        bottom = buttonsY - kLargeSpacing * 2;

        if (error->isVisible()) {
            error->setColor(ui::color(ui::ColorId::DestructiveAction));
            const double height = error->heightForWidth(inner);
            error->setGeometry({left + kLargeSpacing, bottom - height, inner, height});
            bottom -= height + kLargeSpacing;
        }

        const bool fields = m_chooser.saveMode() || !m_chooser.filters().empty() || !m_chooser.choices().empty();
        if (fields) {
            // One left edge for the fields of both rows.
            const double labelWidth = std::max(saveAsLabel->sizeHint().width, filterLabel->sizeHint().width);
            std::vector<double> heights;
            const bool save = m_chooser.saveMode();
            const bool filter = !m_chooser.filters().empty();
            double height = 0;
            auto addRow = [&](double rowHeight) {
                if (height > 0)
                    height += kSmallSpacing;
                height += rowHeight;
            };
            if (save)
                addRow(kFieldHeight);
            if (filter)
                addRow(kFieldHeight);
            for (const ChoiceRow& row : m_choices)
                addRow(row.check ? row.check->sizeHint().height : kFieldHeight);
            double rowY = bottom - height;
            bottom = rowY - kLargeSpacing;
            const double fieldX = left + kLargeSpacing + labelWidth + kSmallSpacing;
            const double fieldWidth = left + width - kLargeSpacing - fieldX;
            saveAsLabel->setVisible(save);
            nameField->setVisible(save);
            if (save) {
                placeLabel(saveAsLabel, left + kLargeSpacing, rowY, labelWidth);
                nameField->setGeometry({fieldX, rowY, fieldWidth, kFieldHeight});
                rowY += kFieldHeight + kSmallSpacing;
            }
            filterLabel->setVisible(filter);
            filterCombo->setVisible(filter);
            if (filter) {
                placeLabel(filterLabel, left + kLargeSpacing, rowY, labelWidth);
                filterCombo->setGeometry({fieldX, rowY, fieldWidth, kFieldHeight});
                rowY += kFieldHeight + kSmallSpacing;
            }
            for (ChoiceRow& row : m_choices) {
                if (row.check) {
                    const ui::SizeF size = row.check->sizeHint();
                    row.check->setGeometry({left + kLargeSpacing, rowY, size.width, size.height});
                    rowY += size.height + kSmallSpacing;
                } else {
                    const double labelWidthHere = row.label->sizeHint().width;
                    placeLabel(row.label, left + kLargeSpacing, rowY, labelWidthHere);
                    const double comboX = left + kLargeSpacing + labelWidthHere + kSmallSpacing;
                    row.combo->setGeometry({comboX, rowY, left + width - kLargeSpacing - comboX, kFieldHeight});
                    rowY += kFieldHeight + kSmallSpacing;
                }
            }
        } else {
            saveAsLabel->setVisible(false);
            nameField->setVisible(false);
            filterLabel->setVisible(false);
            filterCombo->setVisible(false);
        }

        const ui::RectF listRect {left, y, width, std::max(0.0, bottom - y)};
        list->setGeometry(listRect);
        busy->setGeometry({listRect.x + centeredStart(listRect.width, 22), listRect.y + centeredStart(listRect.height, 22),
                           22, 22});
    }

    void paint(ui::Painter& painter) override
    {
        const ui::Theme& theme = ui::Theme::current();
        const ui::RectF rect = localRect();
        painter.fillRect({0, 0, kSideBarWidth, rect.height}, ui::color(FmColor::ChooserSidebarBackground));
        painter.fillRect({kSideBarWidth, 0, rect.width - kSideBarWidth, rect.height}, theme.secondBackground);
    }

    // The top strip stands in for the title bar: a drag there moves the window.
    void pointerPress(const ui::PointerEvent& event) override
    {
        m_press = event.position;
        m_pressActive = event.position.y < kLargeSpacing * 2;
    }

    void pointerMove(const ui::PointerEvent& event) override
    {
        if (m_pressActive && (std::abs(event.position.x - m_press.x) > 4 || std::abs(event.position.y - m_press.y) > 4)) {
            m_pressActive = false;
            if (window())
                window()->startMove();
        }
    }

    bool keyPress(const ui::KeyEvent& event) override
    {
        const uint32_t modifiers = event.modifiers;
        if (event.key == ui::key::Escape && !pathBar->isEditing()) {
            m_chooser.cancel();
            return true;
        }
        if (modifiers == ui::AltModifier) {
            if (event.key == ui::key::Up) {
                m_chooser.up();
                return true;
            }
            if (event.key == ui::key::Left) {
                m_chooser.back();
                return true;
            }
            if (event.key == ui::key::Right) {
                m_chooser.forward();
                return true;
            }
        }
        if (modifiers == ui::ControlModifier && event.key == ui::key::letter('h')) {
            m_chooser.setShowHidden(!m_chooser.showHidden());
            return true;
        }
        if (modifiers == ui::ControlModifier && event.key == ui::key::letter('l')) {
            pathBar->openEditor();
            return true;
        }
        if (modifiers == (ui::ControlModifier | ui::ShiftModifier) && event.key == ui::key::letter('n')
            && canCreateFolder()) {
            m_owner->beginNewFolder();
            return true;
        }
        return false;
    }

private:
    struct ChoiceRow
    {
        ui::Label* label = nullptr;
        ui::ComboBox* combo = nullptr;
        ui::CheckBox* check = nullptr;
    };

    static void placeLabel(ui::Label* label, double x, double y, double width)
    {
        label->setColor(ui::Theme::current().text);
        const double height = label->sizeHint().height;
        label->setGeometry({x, y + centeredStart(kFieldHeight, height), width, height});
    }

    // Return opens the current row or accepts; Ctrl+A selects every file.
    bool listKey(const ui::KeyEvent& event)
    {
        if (event.key == ui::key::Return || event.key == ui::key::KeypadEnter) {
            const int row = list->currentRow();
            if (m_chooser.multiple() || row < 0)
                m_chooser.accept();
            else
                m_chooser.activate(row);
            return true;
        }
        if (event.key == ui::key::letter('a') && (event.modifiers & ui::ControlModifier)) {
            m_chooser.selectAll();
            return true;
        }
        return false;
    }

    void locationChanged()
    {
        list->setCurrentIndex(-1);
        pathBar->setUrl(m_chooser.folder().url());
        pathBar->closeEditor();
        places->requestLayout();
        places->requestPaint();
        updateBusy();
        requestLayout();
    }

    void updateBusy()
    {
        const bool listing = m_chooser.folder().status() == FolderModel::Status::Listing;
        busy->setVisible(listing);
        busy->setRunning(listing);
    }

    ChooserWindow* m_owner;
    FileChooser& m_chooser;
    std::unique_ptr<PlacesModel> m_places;
    std::vector<ChoiceRow> m_choices;
    std::vector<base::Subscription> m_subscriptions;
    ui::PointF m_press;
    bool m_pressActive = false;
};

ChooserWindow::ChooserWindow(FileChooser& chooser)
    : m_chooser(chooser)
{
    m_window = std::make_unique<ui::Window>();
    m_window->setTitle(chooser.title());
    // Not a file manager window: the dock does not count it, Quit and Hide leave it alone.
    m_window->setSkipTaskbar(true);
    m_window->setMinimumSize({kWidth, kHeight});
    m_window->setMaximumSize({kWidth, kHeight});
    m_window->resize({kWidth, kHeight});
    // Opaque on purpose: the chooser does not use the blurred window background.
    m_window->setBackgroundColor(ui::ColorId::Background);
    auto root = std::make_unique<ChooserRoot>(this, chooser);
    m_root = root.get();
    m_window->setContentView(std::move(root));
    m_window->onCloseRequested = [this] { m_chooser.cancel(); };

    const std::string parent = chooser.request()["parent_window"].toString();
    if (parent.starts_with("wayland:"))
        m_window->setTransientForForeign(parent, chooser.request()["modal"].toBool(true));

    m_subscriptions.push_back(ui::Application::instance()->languageChanged.connect([this] { retranslate(); }));
    m_subscriptions.push_back(chooser.confirmOverwrite.connect([this](const std::string& name) {
        m_overwrite = std::make_unique<ui::ConfirmDialog>(m_window.get(), std::string());
        m_overwriteName = name;
        overwriteTexts();
        m_overwrite->setDestructive(true);
        m_overwrite->onAccepted = [this] { m_chooser.accept(true); };
        m_overwrite->show();
    }));
}

ChooserWindow::~ChooserWindow() = default;

void ChooserWindow::retranslate()
{
    m_window->setTitle(m_chooser.title());
    m_root->retranslate();
    if (m_overwrite)
        overwriteTexts();
    if (m_deviceError)
        deviceErrorTexts();
    if (m_unlock)
        m_unlock->retranslate();
    if (m_newFolder && m_newFolder->window() && m_newFolderRetranslate)
        m_newFolderRetranslate();
}

void ChooserWindow::overwriteTexts()
{
    m_overwrite->setTitle(tr("Replace File?"));
    m_overwrite->setText(format(tr("“{name}” already exists. Replace it?"), {{"name", m_overwriteName}}));
    m_overwrite->setConfirmText(tr("Replace"));
}

void ChooserWindow::deviceErrorTexts()
{
    m_deviceError->setTitle(tr("Device unavailable"));
    m_deviceError->setConfirmText(tr("Close"));
}

void ChooserWindow::show()
{
    m_window->show();
    m_root->focusFirst();
    m_window->activate();
}

void ChooserWindow::beginNewFolder()
{
    m_newFolder = std::make_unique<ui::DialogWindow>(m_window.get(), tr("New Folder"), 340);
    if (ui::Window* window = m_newFolder->window())
        window->setBackgroundColor(ui::ColorId::SecondBackground);

    class Content : public ui::ColumnLayout
    {
    public:
        explicit Content(ChooserWindow* owner)
        {
            setSpacing(16);
            setPadding({0, 8, 0, 0});
            field = addChild<ui::TextField>();
            field->setSelectByMouse(true);
            field->setLayoutHints({.fillWidth = true});
            auto* actions = addChild<ui::RowLayout>();
            actions->setSpacing(8);
            actions->setLayoutHints({.horizontalAlignment = ui::Alignment::End});
            cancel = actions->addChild<ui::DialogButton>();
            create = actions->addChild<ui::DialogButton>(std::string(), true);
            retranslate();
            create->setEnabled(false);
            cancel->onClicked = [owner] { owner->m_newFolder->close(); };
            const auto accept = [this, owner] {
                if (field->text().empty())
                    return;
                const std::string name = field->text();
                owner->m_newFolder->close();
                owner->m_chooser.newFolder(name);
            };
            create->onClicked = accept;
            field->onAccepted = accept;
            field->onTextChanged = [this] { create->setEnabled(!field->text().empty()); };
        }

        void retranslate()
        {
            field->setPlaceholder(tr("Folder name"));
            cancel->setText(tr("Cancel"));
            create->setText(tr("Create"));
        }

        ui::TextField* field;
        ui::DialogButton* cancel;
        ui::DialogButton* create;
    };

    auto* content = m_newFolder->setContent<Content>(this);
    // The name typed stays.
    m_newFolderRetranslate = [this, content] {
        m_newFolder->setTitle(tr("New Folder"));
        content->retranslate();
    };
    if (ui::Window* window = m_newFolder->window())
        window->addShortcut(ui::NoModifier, ui::key::Escape, [this] { m_newFolder->close(); });
    m_newFolder->show();
    content->field->setFocus();
}

} // namespace fm
