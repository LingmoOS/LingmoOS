// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/input/checkrow.h"

#include "ui/icon.h"
#include "ui/theme.h"

using namespace Lingmo::ui;

namespace settings {

namespace {

class IconSlot : public View
{
public:
    IconSlot() { m_icon = addChild<ThemeIcon>(std::string(), 18); }

    ThemeIcon* icon() const { return m_icon; }

    SizeF sizeHint() const override { return {24, 24}; }

protected:
    void layout() override { m_icon->setGeometry({3, 3, 18, 18}); }

private:
    ThemeIcon* m_icon;
};

} // namespace

CheckRow::CheckRow(std::string text)
    : ListRow(std::move(text))
{
    m_check = addContent<TrailingIcon>("check", 18);
    m_check->setVisible(false);
}

void CheckRow::setChecked(bool checked)
{
    if (checked == m_checked)
        return;
    m_checked = checked;
    m_check->setVisible(checked);
    requestLayout();
}

void CheckRow::setIcon(const std::string& name)
{
    if (!m_icon)
        m_icon = setLeading<IconSlot>()->icon();
    m_icon->setIconName(name);
}

void CheckRow::layout()
{
    m_check->icon()->setColor(Theme::current().highlight);
    ListRow::layout();
}

} // namespace settings
