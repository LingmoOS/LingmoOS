// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"
#include "widgets/formdialog.h"

namespace Lingmo::ui {
class ColumnLayout;
class DialogButton;
class Label;
class ScrollView;
}

namespace settings {

class Language;
class SearchField;

// The catalog of languages not yet preferred, searched as it is typed; the
// chosen one is added at the end of the preferred languages.
class AddLanguageDialog : public FormDialog
{
public:
    AddLanguageDialog(SettingsWindow& parent, Language& language);

private:
    void updateRows();
    void updateStates();
    void select(const std::string& code);

    Language& m_language;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    Lingmo::base::Timer m_searchTimer;
    SearchField* m_search;
    Lingmo::ui::ScrollView* m_scroll;
    Lingmo::ui::ColumnLayout* m_rows;
    Lingmo::ui::Label* m_empty;
    Lingmo::ui::Label* m_error;
    Lingmo::ui::DialogButton* m_cancel;
    Lingmo::ui::DialogButton* m_add;
    std::string m_selectedCode;
    bool m_adding = false;
};

} // namespace settings
