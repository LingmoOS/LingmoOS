// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/timer.h"
#include "desktop/widgets/widget.h"

namespace Lingmo::ui {
class TextArea;
class Label;
}

namespace fm {

// A small editable note. Its text belongs to this card's instance id, so two
// note cards on the same desktop keep separate text.
class NotesWidget : public DesktopWidget
{
public:
    NotesWidget(WidgetLayer& layer, WidgetSize size);
    ~NotesWidget() override;

protected:
    void layoutContent() override;
    void retranslateContent() override;
    void instanceIdChanged() override;

private:
    void load();
    void save();
    std::string notePath() const;

    ui::TextArea* m_text = nullptr;
    ui::Label* m_title = nullptr;
    base::Timer m_saveTimer;
    bool m_loaded = false;
};

} // namespace fm
