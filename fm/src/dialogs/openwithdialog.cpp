// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialogs/openwithdialog.h"

#include "base/i18n.h"
#include "ui/check_box.h"
#include "ui/dialog_window.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/list_view.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"

#include <algorithm>

namespace fm {

using ui::rounding::centeredStart;
using Lingmo::i18n::tr;

// One application: its 32 px icon and name in a 52 px row, lit when current.
class ApplicationRow : public ui::View
{
public:
    explicit ApplicationRow(OpenWithDialog* dialog, std::function<void(size_t, int)> pressed)
        : m_pressed(std::move(pressed))
    {
        (void)dialog;
        m_icon = addChild<ui::ThemeIcon>(std::string(), 32);
        m_name = addChild<ui::Label>();
        m_name->setElide(ui::Label::Elide::Middle);
    }

    void bind(size_t row, const launcher::Application& application, bool selected)
    {
        m_row = row;
        m_selected = selected;
        m_icon->setIconName(application.icon);
        m_name->setText(application.name);
        m_name->setColor(selected ? ui::ColorId::HighlightedText : ui::ColorId::Text);
        requestLayout();
        requestPaint();
    }

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const ui::RectF rect = localRect().adjusted(10, 10, -10, -10);
        m_icon->setGeometry({rect.x, rect.y + centeredStart(rect.height, 32), 32, 32});
        const ui::SizeF hint = m_name->sizeHint();
        m_name->setGeometry({rect.x + 32 + 12, rect.y + centeredStart(rect.height, hint.height),
                             std::max(0.0, rect.width - 44), hint.height});
    }

    void paint(ui::Painter& painter) override
    {
        if (m_selected)
            painter.drawSmoothRectangle(localRect(), ui::Theme::current().mediumRadius, ui::Theme::current().highlight);
    }

    void pointerPress(const ui::PointerEvent& event) override { m_pressed(m_row, event.clickCount); }

private:
    std::function<void(size_t, int)> m_pressed;
    ui::ThemeIcon* m_icon;
    ui::Label* m_name;
    size_t m_row = 0;
    bool m_selected = false;
};

OpenWithDialog::OpenWithDialog(std::string path, std::string mimeType)
    : FramelessDialog(tr("Open With"), true)
    , m_path(std::move(path))
    , m_mimeType(std::move(mimeType))
{
    m_applications = launcher::applicationsFor(m_path, m_mimeType);
    setContentFactory([this] { return createContent(); }, 400 + 12 * 2, 16);
    onCloseRequested = [this] { done(); };
}

void OpenWithDialog::retranslate()
{
    setTitle(tr("Open With"));
    if (!isShown())
        return;
    const std::string current = m_current >= 0 && m_current < int(m_applications.size())
            ? m_applications[size_t(m_current)].desktopFile
            : std::string();
    const bool isDefault = m_default && m_default->isChecked();
    m_applications = launcher::applicationsFor(m_path, m_mimeType);
    const auto found = std::find_if(m_applications.begin(), m_applications.end(),
                                    [&current](const launcher::Application& application) {
                                        return application.desktopFile == current;
                                    });
    m_current = found != m_applications.end() ? int(found - m_applications.begin()) : 0;
    rebuildContent();
    m_default->setChecked(isDefault);
}

void OpenWithDialog::done()
{
    hide();
    if (onDone)
        onDone();
}

void OpenWithDialog::openApplication()
{
    if (m_current < 0 || m_current >= int(m_applications.size()))
        return;
    const launcher::Application& application = m_applications[size_t(m_current)];
    if (m_default && m_default->isChecked())
        launcher::setDefaultApplication(m_mimeType, application.desktopFile);
    launcher::launchApplication(application.desktopFile, m_path);
    done();
}

std::unique_ptr<ui::View> OpenWithDialog::createContent()
{
    auto layout = std::make_unique<ui::ColumnLayout>();
    layout->setSpacing(16);

    auto* title = layout->addChild<ui::Label>(tr("Open With"));
    title->setFontSize(14);
    title->setWeight(600);
    title->setColor(ui::ColorId::Text);
    ui::LayoutHints titleHints {.fillWidth = true};
    titleHints.margins.right = 32;
    title->setLayoutHints(titleHints);

    m_list = layout->addChild<ui::ListView>();
    m_list->setLayoutHints({.fillWidth = true, .preferredHeight = 250});
    m_list->setRowHeight(52);
    m_list->setSpacing(4);
    m_list->createItem = [this] {
        return std::make_unique<ApplicationRow>(this, [this](size_t row, int clicks) {
            m_current = int(row);
            m_list->reload();
            if (clicks == 2)
                openApplication();
        });
    };
    m_list->bindItem = [this](ui::View* item, size_t row) {
        static_cast<ApplicationRow*>(item)->bind(row, m_applications[row], int(row) == m_current);
    };
    m_list->setRowCount(m_applications.size());
    if (m_applications.empty()) {
        auto* empty = m_list->addChild<ui::Label>(tr("No applications"));
        const ui::SizeF hint = empty->sizeHint();
        empty->setGeometry({centeredStart(400 - 8, hint.width), centeredStart(250, hint.height), hint.width,
                            hint.height});
    }

    m_default = layout->addChild<ui::CheckBox>(tr("Set as default"));
    m_default->setPadding({});
    m_default->setEnabled(!m_applications.empty());

    auto* actions = layout->addChild<ui::RowLayout>();
    actions->setSpacing(8);
    actions->setLayoutHints({.horizontalAlignment = ui::Alignment::End});
    auto* cancel = actions->addChild<ui::DialogButton>(tr("Cancel"));
    cancel->onClicked = [this] { done(); };
    auto* open = actions->addChild<ui::DialogButton>(tr("Open"), true);
    open->setEnabled(!m_applications.empty());
    open->onClicked = [this] { openApplication(); };
    return layout;
}

} // namespace fm
