// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/process.h"
#include "base/timer.h"
#include "lingmo/appearance/appearance.h"
#include "pages/about/aboutinfo.h"
#include "ui/cell_layout.h"
#include "ui/clipboard.h"
#include "ui/dialog_window.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/view.h"
#include "widgets/card.h"
#include "widgets/formdialog.h"
#include "widgets/iconbutton.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"
#include "widgets/sections.h"
#include "widgets/standardbutton.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

namespace {

// The rule between the wordmark and the wallpaper, at the height of the text
// rather than of the picture beside it.
class VerticalRule : public View
{
protected:
    void paint(Painter& painter) override
    {
        painter.drawVerticalLine(geometry().width / 2, 0, geometry().height,
                                 Theme::current().disabledText.withAlpha(0.28), 1);
    }
};

// The current wallpaper, rounded and framed: the desktop's own picture is the
// most recognisable thing on this page. A wallpaper is usually a photo, so the
// picture is an ImageView (SVG files included), not the SVG-only Picture.
class WallpaperThumbnail : public View
{
public:
    explicit WallpaperThumbnail(std::string path)
    {
        m_picture = addChild<ImageView>(std::move(path));
        m_picture->setFillMode(ImageView::FillMode::PreserveAspectCrop);
        m_picture->setCornerRadius(Theme::current().bigRadius);
        m_frame = addChild<Frame>();
    }

protected:
    void layout() override
    {
        m_picture->setGeometry(localRect());
        m_frame->setGeometry(localRect());
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        // The shadow lifts the picture off the card; the frame is drawn by a
        // child after the picture, so its hairline stays visible.
        painter.drawShadow(localRect(), theme.bigRadius, 18,
                           Color {0, 0, 0, uint8_t(theme.darkMode ? 90 : 38)}, {0, 4});
    }

private:
    class Frame : public View
    {
    protected:
        void paint(Painter& painter) override
        {
            const Theme& theme = Theme::current();
            painter.drawSmoothRectangle(localRect(), theme.bigRadius, Color {0, 0, 0, 0}, 1,
                                        theme.text.withAlpha(0.18));
        }
    };

    ImageView* m_picture = nullptr;
    Frame* m_frame = nullptr;
};

// One fact in the system card: what it is above, the value below.
class InfoCell : public View
{
public:
    InfoCell(std::string key, std::string value)
    {
        // The grid's columns are as wide as their cells ask to be; without
        // this a cell takes its preferred width, which is nothing.
        setLayoutHints({.fillWidth = true});
        m_key = addChild<Label>(std::move(key));
        m_key->setColor(Theme::current().disabledText);
        m_key->setFontSize(metrics::points(9));
        m_value = addChild<Label>(std::move(value));
        m_value->setWrap(true);
        m_value->setFontSize(metrics::points(10));
        m_value->setWeight(500);
    }

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double keyHeight = m_key->heightForWidth(width);
        m_key->setGeometry({0, 0, width, keyHeight});
        m_value->setGeometry({0, keyHeight + metrics::xs, width, m_value->heightForWidth(width)});
    }

    // The grid measures a row at the width it has to fill, so this is what
    // decides the cell's height, not sizeHint(): before layout there is no
    // width to wrap the value at.
    double heightForWidth(double width) const override
    {
        return m_key->heightForWidth(width) + metrics::xs + m_value->heightForWidth(width);
    }

private:
    Label* m_key;
    Label* m_value;
};

class AuroraVersionCell : public View
{
public:
    AuroraVersionCell()
    {
        setName("about:aurora-version");
        setLayoutHints({.fillWidth = true});
        m_key = addChild<Label>(tr("Aurora Version"));
        m_key->setColor(Theme::current().disabledText);
        m_key->setFontSize(metrics::points(9));

        m_logo = addChild<ImageView>(dataPath("images/aurora.png"));
        m_logo->setFillMode(ImageView::FillMode::PreserveAspectFit);
        m_value = addChild<Label>(about::auroraVersion());
        m_value->setName("about:aurora-version-label");
        m_value->setFontSize(metrics::points(10));
        m_value->setWeight(500);
    }

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double keyHeight = m_key->heightForWidth(width);
        const double logoSize = metrics::points(18);
        const double valueY = keyHeight + metrics::xs;
        m_key->setGeometry({0, 0, width, keyHeight});
        m_logo->setGeometry({0, valueY, logoSize, logoSize});
        m_value->setGeometry({logoSize + metrics::s, valueY,
                              std::max(0.0, width - logoSize - metrics::s), logoSize});
    }

    double heightForWidth(double width) const override
    {
        return m_key->heightForWidth(width) + metrics::xs + metrics::points(18);
    }

private:
    Label* m_key;
    ImageView* m_logo;
    Label* m_value;
};

// A quiet rule between the software and hardware groups of the system card.
class Divider : public View
{
protected:
    void paint(Painter& painter) override
    {
        painter.fillRect({0, 0, geometry().width, 1}, Theme::current().disabledText.withAlpha(0.16));
    }
};

// A small line of text that behaves as a link.
class TextLink : public View
{
public:
    explicit TextLink(std::string text)
    {
        m_label = addChild<Label>(std::move(text));
        m_label->setFontSize(metrics::points(9));
        m_label->setColor(ColorId::Highlight);
        setCursorShape(CursorShape::Pointer);
    }

    std::function<void()> onClicked;

    SizeF sizeHint() const override { return m_label->sizeHint(); }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        m_label->setGeometry({0, 0, geometry().width, m_label->heightForWidth(geometry().width)});
    }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button == MouseButton::Left)
            m_pressed = true;
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        m_pressed = false;
        if (localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    Label* m_label = nullptr;
    bool m_pressed = false;
};

// The host name systemd accepts: letters, digits, hyphen-separated labels,
// dots between labels, at most 63 characters for the kernel's own name.
bool validHostName(const std::string &name)
{
    if (name.empty() || name.size() > 63)
        return false;
    int labelLength = 0;
    bool hyphen = false;
    for (const char c : name) {
        const bool alpha = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        if (alpha || (c >= '0' && c <= '9')) {
            ++labelLength;
            hyphen = false;
        } else if (c == '-') {
            if (labelLength == 0)
                return false;
            ++labelLength;
            hyphen = true;
        } else if (c == '.') {
            if (labelLength == 0 || hyphen)
                return false;
            labelLength = 0;
            hyphen = false;
        } else {
            return false;
        }
    }
    return labelLength > 0 && !hyphen;
}

std::string trimmedHostName(std::string value)
{
    const auto first = value.find_first_not_of(" \t");
    if (first == std::string::npos)
        return {};
    const auto last = value.find_last_not_of(" \t");
    return value.substr(first, last - first + 1);
}

// Renames the machine through systemd's hostnamed; polkit asks for
// authorization, and the dialog stays open until the bus answers.
class EditHostnameDialog : public FormDialog
{
public:
    EditHostnameDialog(SettingsWindow& parent, const std::string& current,
                       std::function<void(std::string)> changed)
        : FormDialog(parent, tr("Change computer name"), 420)
        , m_changed(std::move(changed))
        , m_alive(std::make_shared<char>())
    {
        content()->addChild<Label>(tr("A computer name identifies this machine on the network."));
        m_field = content()->addChild<TextField>(current);
        m_field->setPlaceholder(tr("Computer name"));
        m_field->setMaximumLength(63);
        m_field->setLayoutHints({.fillWidth = true});
        m_field->onAccepted = [this] { save(); };

        Label* hint = content()->addChild<Label>(
                tr("Letters, numbers, hyphens and dots only; 1–63 characters."));
        hint->setWrap(true);
        hint->setColor(ColorId::DisabledText);
        hint->setLayoutHints({.fillWidth = true});

        m_error = content()->addChild<Label>();
        m_error->setWrap(true);
        m_error->setLayoutHints({.fillWidth = true});
        m_error->setVisible(false);
        m_field->onTextChanged = [this] {
            if (!m_waiting) {
                m_error->setText({});
                m_error->setVisible(false);
            }
            update();
        };

        RowLayout* buttons = addButtonRow();
        m_cancel = buttons->addChild<DialogButton>(tr("Cancel"));
        m_cancel->onClicked = [this] { requestClose(); };
        m_save = buttons->addChild<DialogButton>(tr("Save"), true);
        m_save->onClicked = [this] { save(); };

        canClose = [this] { return !m_waiting; };
        onShown = [this] { m_field->setFocus(); };
        update();
    }

private:
    void update()
    {
        const bool valid = validHostName(trimmedHostName(m_field->text()));
        m_save->setEnabled(!m_waiting && valid);
        m_cancel->setEnabled(!m_waiting);
        m_field->setEnabled(!m_waiting);
    }

    void save()
    {
        if (m_waiting)
            return;
        const std::string name = trimmedHostName(m_field->text());
        if (!validHostName(name)) {
            showError(tr("That is not a valid computer name."));
            return;
        }
        m_field->setText(name);
        m_waiting = true;
        update();
        m_error->setVisible(false);

        base::dbus::Connection* bus = base::dbus::Connection::system(base::EventLoop::current());
        if (!bus) {
            fail(tr("The computer name could not be changed."));
            return;
        }
        auto proxy = std::make_shared<base::dbus::Proxy>(bus, "org.freedesktop.hostname1",
                                                         "/org/freedesktop/hostname1",
                                                         "org.freedesktop.hostname1");
        proxy->setInteractiveAuthorizationAllowed(true);
        proxy->setTimeout(std::chrono::seconds(120));
        m_pending = proxy->callAsync(
                "SetStaticHostname",
                [this, alive = std::weak_ptr<char>(m_alive), proxy, name](const base::dbus::Reply& reply) {
                    if (alive.expired())
                        return;
                    if (reply.isError()) {
                        fail(tr("The computer name could not be changed."));
                        return;
                    }
                    if (m_changed)
                        m_changed(name);
                    close();
                },
                name, true);
    }

    void showError(const std::string& message)
    {
        m_error->setText(message);
        m_error->setVisible(!message.empty());
        if (!m_waiting)
            update();
    }

    void fail(const std::string& message)
    {
        m_waiting = false;
        showError(message);
    }

    std::function<void(std::string)> m_changed;
    std::shared_ptr<char> m_alive;
    TextField* m_field = nullptr;
    Label* m_error = nullptr;
    DialogButton* m_cancel = nullptr;
    DialogButton* m_save = nullptr;
    base::dbus::PendingCall m_pending;
    bool m_waiting = false;
};

// The fuller report behind System's "More system information": the same
// facts as the page plus the kernel build, uptime, session and firmware.
class SystemDetailsDialog : public FormDialog
{
public:
    explicit SystemDetailsDialog(SettingsWindow& parent)
        : FormDialog(parent, tr("System details"), 520)
    {
        addGroup(tr("Device"), {
                {tr("PC Name"), about::hostName()},
                {tr("OS Version"), about::prettyProductName()},
                {tr("Computer Model"), about::computerModel()},
        });
        addGroup(tr("Software"), {
                {tr("Debian Version"), about::debianVersion()},
                {tr("Linux Kernel Version"), "Linux " + about::kernelVersion()},
                {tr("Kernel Build"), about::kernelBuild()},
                {tr("Aurora Version"), about::auroraVersion()},
                {tr("System Type"), about::architecture()},
                {tr("Session"), about::sessionType()},
                {tr("Uptime"), about::uptime()},
        });
        addGroup(tr("Hardware"), {
                {tr("CPU"), about::processor()},
                {tr("RAM"), about::memorySize()},
                {tr("Disk"), about::internalStorage()},
                {tr("Firmware"), about::firmwareVersion()},
        });

        RowLayout* buttons = addButtonRow();
        DialogButton* close = buttons->addChild<DialogButton>(tr("Close"));
        close->onClicked = [this] { requestClose(); };
    }

private:
    void addGroup(const std::string& title, std::vector<std::pair<std::string, std::string>> entries)
    {
        entries.erase(std::remove_if(entries.begin(), entries.end(),
                                     [](const auto& entry) { return entry.second.empty(); }),
                      entries.end());
        if (entries.empty())
            return;

        content()->addChild<SectionLabel>(title)->setLayoutHints({.fillWidth = true});
        GridLayout* grid = content()->addChild<GridLayout>(2);
        grid->setLayoutHints({.fillWidth = true});
        grid->setColumnSpacing(metrics::xl);
        grid->setRowSpacing(metrics::m);
        for (const auto& [key, value] : entries)
            grid->addChild<InfoCell>(key, value);
    }
};

class AboutPage : public Page
{
public:
    explicit AboutPage(SettingsWindow& window)
        : Page(window, tr("About"))
    {
        addBrand();
        addIdentity();
        addSystem();
        addActions();
    }

protected:
    void layout() override
    {
        // The wordmark is drawn dark for the light theme and white for the
        // dark one, so the file is swapped rather than the picture tinted.
        if (m_logo) {
            const std::string logo = logoForTheme();
            if (logo != m_logoPath) {
                m_logoPath = logo;
                m_logo->setPath(logo);
            }
        }
        Page::layout();
    }

private:
    static std::string logoForTheme()
    {
        return dataPath(Theme::current().darkMode ? "images/logo-dark.svg" : "images/logo-light.svg");
    }

    // The wordmark, the rule, and the wallpaper the session is showing.
    void addBrand()
    {
        Card* hero = content()->addChild<Card>();
        RowLayout* brand = hero->addChild<RowLayout>();
        brand->setSpacing(metrics::xl);
        brand->setLayoutHints({.horizontalAlignment = Alignment::Center});

        m_logoPath = logoForTheme();
        m_logo = brand->addChild<Picture>(m_logoPath);
        m_logo->setLayoutHints({.preferredWidth = 220, .preferredHeight = 84,
                                .verticalAlignment = Alignment::Center});

        VerticalRule* rule = brand->addChild<VerticalRule>();
        rule->setLayoutHints({.preferredWidth = 1, .preferredHeight = 56,
                              .verticalAlignment = Alignment::Center});

        WallpaperThumbnail* thumbnail = brand->addChild<WallpaperThumbnail>(m_appearance.wallpaper());
        thumbnail->setLayoutHints({.preferredWidth = 160, .preferredHeight = 96,
                                   .verticalAlignment = Alignment::Center});
    }

    void addIdentity()
    {
        Section* section = content()->addChild<Section>(tr("Device"));
        Card* card = section->addChild<Card>(Card::Style::List);
        m_pcName = card->addChild<InfoRow>(tr("PC Name"), about::hostName());
        m_pcName->setName("about:pc-name");
        IconButton* edit = m_pcName->addContent<IconButton>(dataPath("images/edit.svg"), 18);
        edit->setName("about:edit-pc-name");
        edit->setLayoutHints({.preferredWidth = 18, .preferredHeight = 18});
        edit->setColor(ColorId::Text);
        edit->setOpacity(0.65);
        edit->setToolTip(tr("Edit computer name"));
        edit->onClicked = [this] { editHostName(); };
        card->addChild<InfoRow>(tr("OS Version"), about::prettyProductName());
    }

    void editHostName()
    {
        if (m_hostnameDialog)
            return;
        m_hostnameDialog = std::make_unique<EditHostnameDialog>(
                settingsWindow(), about::hostName(), [this](std::string name) {
                    if (m_pcName)
                        m_pcName->setValue(std::move(name));
                });
        m_hostnameDialog->onClosed = [this] {
            // Lets the dialog finish the event that closed it before it goes.
            std::shared_ptr<EditHostnameDialog> closing(m_hostnameDialog.release());
            base::EventLoop::current()->post([closing] {});
        };
        m_hostnameDialog->show();
    }

    // Software above a rule, the processor across the card, then the resources
    // below it. The four-column grid this replaces squeezed the kernel and CPU
    // into three lines and left its last cell empty.
    void addSystem()
    {
        Section* section = content()->addChild<Section>(tr("System"));
        Card* card = section->addChild<Card>(Card::Style::Padded);
        ColumnLayout* groups = card->addChild<ColumnLayout>();
        groups->setSpacing(metrics::m);
        groups->setLayoutHints({.fillWidth = true});

        GridLayout* versions = groups->addChild<GridLayout>(2);
        versions->setLayoutHints({.fillWidth = true});
        versions->setColumnSpacing(metrics::xl);
        versions->setRowSpacing(metrics::m);
        versions->addChild<InfoCell>(tr("Debian Version"), about::debianVersion());
        versions->addChild<InfoCell>(tr("Linux Kernel Version"), "Linux " + about::kernelVersion());

        GridLayout* platform = groups->addChild<GridLayout>(2);
        platform->setLayoutHints({.fillWidth = true});
        platform->setColumnSpacing(metrics::xl);
        platform->setRowSpacing(metrics::m);
        platform->addChild<AuroraVersionCell>();
        platform->addChild<InfoCell>(tr("System Type"), about::architecture());

        groups->addChild<Divider>()->setLayoutHints({.fillWidth = true, .preferredHeight = 1});

        groups->addChild<InfoCell>(tr("CPU"), about::processor())
                ->setLayoutHints({.fillWidth = true});

        groups->addChild<Divider>()->setLayoutHints({.fillWidth = true, .preferredHeight = 1});

        const std::string model = about::computerModel();
        GridLayout* resources = groups->addChild<GridLayout>(model.empty() ? 2 : 3);
        resources->setLayoutHints({.fillWidth = true});
        resources->setColumnSpacing(metrics::xl);
        resources->setRowSpacing(metrics::m);
        resources->addChild<InfoCell>(tr("RAM"), about::memorySize());
        resources->addChild<InfoCell>(tr("Disk"), about::internalStorage());
        if (!model.empty())
            resources->addChild<InfoCell>(tr("Computer Model"), model);

        TextLink* more = section->addChild<TextLink>(tr("More system information"));
        more->setName("about:more-info");
        more->onClicked = [this] { openDetails(); };
    }

    void openDetails()
    {
        if (m_detailsDialog)
            return;
        m_detailsDialog = std::make_unique<SystemDetailsDialog>(settingsWindow());
        m_detailsDialog->onClosed = [this] {
            std::shared_ptr<SystemDetailsDialog> closing(m_detailsDialog.release());
            base::EventLoop::current()->post([closing] {});
        };
        m_detailsDialog->show();
    }

    void addActions()
    {
        m_copy = content()->addChild<StandardButton>(tr("Copy System Information"));
        m_copy->setName("about:copy-info");
        m_copy->onClicked = [this] {
            Clipboard::setText(systemInformation());
            m_copy->setText(tr("Copied"));
            m_copyReset.start();
        };
        m_copyReset.setSingleShot(true);
        m_copyReset.setInterval(std::chrono::milliseconds(1600));
        m_copyReset.onTimeout = [this] {
            if (m_copy)
                m_copy->setText(tr("Copy System Information"));
        };
    }

    std::string systemInformation() const
    {
        std::string text = about::prettyProductName() + "\n";
        const auto line = [&text](const std::string& key, const std::string& value) {
            if (!value.empty())
                text += key + ": " + value + "\n";
        };
        line(tr("PC Name"), about::hostName());
        line(tr("Computer Model"), about::computerModel());
        line(tr("Debian Version"), about::debianVersion());
        line(tr("Linux Kernel Version"), about::kernelVersion());
        line(tr("Aurora Version"), about::auroraVersion());
        line(tr("System Type"), about::architecture());
        line(tr("CPU"), about::processor());
        line(tr("RAM"), about::memorySize());
        line(tr("Disk"), about::internalStorage());
        return text;
    }

    Picture* m_logo = nullptr;
    std::string m_logoPath;
    InfoRow* m_pcName = nullptr;
    std::unique_ptr<EditHostnameDialog> m_hostnameDialog;
    std::unique_ptr<SystemDetailsDialog> m_detailsDialog;
    StandardButton* m_copy = nullptr;
    base::Timer m_copyReset;
    Lingmo::appearance::Appearance m_appearance;
};

} // namespace

static const PageRegistration registration("about", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<AboutPage>(window);
});

} // namespace settings
