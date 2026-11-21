// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processdetails.h"

#include "components.h"
#include "monitor.h"
#include "procreader.h"

#include "base/i18n.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/list_view.h"
#include "ui/text_area.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;

// The dialog is 612 high: the toolkit's 44 header and 16 bottom padding around this.
static constexpr double kContentHeight = 612 - 44 - 16;
static constexpr double kMemberHeight = 44;

// One process of an application: its program, memory and PID.
class MemberRow : public View
{
public:
    MemberRow()
        : m_row(addChild<RowLayout>())
        , m_name(m_row->addChild<Label>())
        , m_memory(m_row->addChild<Label>())
        , m_pid(m_row->addChild<Label>())
    {
        m_row->setSpacing(12);
        m_name->setLayoutHints({.fillWidth = true});
        m_pid->setLayoutHints({.preferredWidth = 64});
        m_pid->setTextAlignment(Label::TextAlignment::Right);
    }

    void bind(const ProcessRecord& member, bool last)
    {
        m_name->setText(ProcessList::programName(member));
        m_memory->setText(Monitor::formatBytes(double(member.memory)));
        m_pid->setText(std::to_string(member.pid));
        m_last = last;
        requestLayout();
        requestPaint();
    }

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        m_memory->setColor(theme.disabledText);
        m_pid->setColor(theme.disabledText);
        m_row->setGeometry(localRect().adjusted(16, 0, -16, 0));
    }

    void paint(Painter& painter) override
    {
        if (!m_last)
            paintSeparator(painter, 16, geometry().height, geometry().width - 32);
    }

private:
    RowLayout* m_row;
    Label* m_name;
    Label* m_memory;
    Label* m_pid;
    bool m_last = false;
};

// A card whose content fills it.
class FilledPlate : public Plate
{
public:
    using Plate::Plate;

protected:
    void layout() override
    {
        for (const auto& child : children())
            child->setGeometry(localRect());
    }
};

// As the Qt version: a ColumnLayout 12 below the header, with the dialog's
// 16 at the sides and the bottom.
class ProcessDetails::Content : public View
{
public:
    explicit Content(bool application)
        : m_application(application)
        , m_column(addChild<ColumnLayout>())
    {
        m_column->setSpacing(12);
        auto* header = m_column->addChild<RowLayout>();
        header->setSpacing(12);
        header->setLayoutHints({.fillWidth = true});
        m_icon = header->addChild<ThemeIcon>("application-x-executable", 40);
        m_icon->setLayoutHints({.preferredWidth = 40, .preferredHeight = 40});
        auto* names = header->addChild<ColumnLayout>();
        names->setSpacing(4);
        names->setLayoutHints({.fillWidth = true});
        m_name = addLabel(names, {}, 12, 500);
        m_name->setLayoutHints({.fillWidth = true});
        m_pid = addLabel(names, {}, 9);

        m_card = m_column->addChild<ListCard>();
        m_card->setLayoutHints({.fillWidth = true});
        for (const char* text : {trNoop("User"), trNoop("State"), trNoop("Processes"), trNoop("CPU"), trNoop("Memory"),
                                 trNoop("Threads"), trNoop("Nice value"), trNoop("Parent process")})
            m_rows.push_back(m_card->addRow(tr(text)));
        m_rows[1]->setVisible(!application);
        m_rows[2]->setVisible(application);
        m_rows[6]->setVisible(!application);
        m_rows[7]->setVisible(!application);
        m_card->updateSeparators();

        m_section = m_column->addChild<SectionLabel>(application ? tr("Processes") : tr("Command"));
        m_section->setLayoutHints({.fillWidth = true});
        m_plate = m_column->addChild<FilledPlate>(Theme::current().bigRadius);
        m_plate->setLayoutHints({.fillWidth = true, .fillHeight = true});
        m_plate->setClipsChildren(true);

        if (application) {
            m_members = m_plate->addChild<ListView>();
            m_members->setRowHeight(kMemberHeight);
            m_members->createItem = [] { return std::make_unique<MemberRow>(); };
            m_members->bindItem = [this](View* item, size_t row) {
                if (row < m_memberRecords.size())
                    static_cast<MemberRow*>(item)->bind(m_memberRecords[row], row + 1 == m_memberRecords.size());
            };
        } else {
            // The rows' text inset above; the card is the frame.
            m_command = m_plate->addChild<TextArea>();
            m_command->setReadOnly(true);
            m_command->setWrapMode(TextArea::WrapMode::WrapAnywhere);
            m_command->setPadding({16, 12, 16, 12});
            m_command->setFrameVisible(false);
        }
    }

    void update(const ProcessRecord* process)
    {
        m_icon->setIconName(process && !process->iconName.empty() ? process->iconName : "application-x-executable");
        m_name->setText(process ? process->displayName : m_application ? tr("Application quit") : tr("Process exited"));
        m_rows[0]->setValue(process && !process->user.empty() ? process->user : "—");
        const std::string state = process ? ProcReader::stateName(process->state) : std::string();
        m_rows[1]->setValue(state.empty() ? tr("Exited") : state);
        m_rows[2]->setValue(process && !process->members.empty() ? std::to_string(process->members.size()) : "—");
        m_rows[3]->setValue(Monitor::formatPercent(process ? process->cpu : 0));
        m_rows[4]->setValue(Monitor::formatBytes(process ? double(process->memory) : 0));
        m_rows[5]->setValue(process && process->threads ? std::to_string(process->threads) : "—");
        m_rows[6]->setValue(process ? std::to_string(process->nice) : "—");
        m_rows[7]->setValue(process && process->parentPid ? std::to_string(process->parentPid) : "—");
        if (m_members) {
            m_memberRecords = process ? process->members : std::vector<ProcessRecord>();
            m_members->setRowCount(m_memberRecords.size());
            m_members->reload();
        } else {
            const std::string command = process ? (!process->command.empty() ? process->command : process->executable)
                                                : std::string();
            const std::string text = command.empty() ? "—" : command;
            if (text != m_command->text())
                m_command->setText(text);
        }
        requestLayout();
    }

    void setPid(int pid) { m_pid->setText(format(tr("PID {pid}"), {{"pid", std::to_string(pid)}})); }

    double heightForWidth(double) const override { return kContentHeight; }
    SizeF sizeHint() const override { return {0, kContentHeight}; }

protected:
    void layout() override
    {
        m_pid->setColor(Theme::current().disabledText);
        m_column->setGeometry(localRect().adjusted(0, 12, 0, 0));
    }

private:
    bool m_application;
    ColumnLayout* m_column;
    ThemeIcon* m_icon;
    Label* m_name;
    Label* m_pid;
    ListCard* m_card;
    std::vector<InfoRow*> m_rows;
    SectionLabel* m_section;
    FilledPlate* m_plate;
    ListView* m_members = nullptr;
    std::vector<ProcessRecord> m_memberRecords;
    TextArea* m_command = nullptr;
};

ProcessDetails::ProcessDetails(Window* parent, Monitor& monitor, const Selection& target)
    : m_monitor(monitor)
    , m_target(target)
    , m_dialog(parent, target.application ? tr("Application Details") : tr("Process Details"), 480)
{
    auto content = std::make_unique<Content>(target.application);
    m_content = content.get();
    m_content->setPid(target.pid);
    m_dialog.setContentView(std::move(content));
    m_dialog.onClosed = [this] {
        if (onClosed)
            onClosed();
    };
    m_updated = m_monitor.updated.connect([this] { update(); });
    update();
}

void ProcessDetails::show()
{
    m_dialog.show();
    if (Window* window = m_dialog.window())
        window->addShortcut(Lingmo::ui::NoModifier, Lingmo::ui::key::Escape, [this] { m_dialog.close(); });
}

void ProcessDetails::update()
{
    m_content->update(m_target.application ? m_monitor.application(m_target.pid, m_target.startTicks)
                                           : m_monitor.process(m_target.pid, m_target.startTicks));
}
