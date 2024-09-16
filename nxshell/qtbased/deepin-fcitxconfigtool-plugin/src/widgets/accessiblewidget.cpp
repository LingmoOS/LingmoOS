// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accessiblewidget.h"
#include "titlelabel.h"
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>

using namespace Dtk::Widget;
using namespace dcc_fcitx_configtool::widgets;

AccessibleWidget::AccessibleWidget(QWidget *widget)
    : QAccessibleWidget(widget)
    , m_widget(widget)
{

}

AccessibleWidget::~AccessibleWidget()
{

}

void *AccessibleWidget::interface_cast(QAccessible::InterfaceType t)
{
    switch (t) {
    case QAccessible::ActionInterface:
        return static_cast<QAccessibleActionInterface *>(this);
    case QAccessible::TextInterface:
        return static_cast<QAccessibleTextInterface *>(this);
    default:
        return nullptr;
    }
}

QString AccessibleWidget::text(QAccessible::Text t) const
{
    switch (t) {
    case QAccessible::Name:
        return m_widget->accessibleName();
    case QAccessible::Description:
        return m_widget->accessibleDescription();
    default:
        return QString();
    }
}

QString AccessibleWidget::text(int startOffset, int endOffset) const
{
    Q_UNUSED(startOffset)
    Q_UNUSED(endOffset)

    return QString();
}

void AccessibleWidget::selection(int selectionIndex, int *startOffset, int *endOffset) const
{
    Q_UNUSED(selectionIndex)
    Q_UNUSED(startOffset)
    Q_UNUSED(endOffset)
}

int AccessibleWidget::selectionCount() const
{
    return 0;
}

void AccessibleWidget::addSelection(int startOffset, int endOffset)
{
    Q_UNUSED(startOffset)
    Q_UNUSED(endOffset)
}

void AccessibleWidget::removeSelection(int selectionIndex)
{
    Q_UNUSED(selectionIndex)
}

void AccessibleWidget::setSelection(int selectionIndex, int startOffset, int endOffset)
{
    Q_UNUSED(selectionIndex)
    Q_UNUSED(startOffset)
    Q_UNUSED(endOffset)
}

int AccessibleWidget::cursorPosition() const
{
    return 0;
}

void AccessibleWidget::setCursorPosition(int position)
{
    Q_UNUSED(position)
}

int AccessibleWidget::characterCount() const
{
    return 0;
}

QRect AccessibleWidget::characterRect(int offset) const
{
    Q_UNUSED(offset)

    return QRect();
}

int AccessibleWidget::offsetAtPoint(const QPoint &point) const
{
    Q_UNUSED(point)

    return 0;
}

void AccessibleWidget::scrollToSubstring(int startIndex, int endIndex)
{
    Q_UNUSED(startIndex)
    Q_UNUSED(endIndex)
}

QString AccessibleWidget::attributes(int offset, int *startOffset, int *endOffset) const
{
    Q_UNUSED(offset)
    Q_UNUSED(startOffset)
    Q_UNUSED(endOffset)

    return QString();
}

AccessibleLabel::AccessibleLabel(QLabel *label)
    : AccessibleWidget(label)
    , m_label(label)
{

}

AccessibleLabel::~AccessibleLabel()
{

}

QString AccessibleLabel::text(int startOffset, int endOffset) const
{
    if (Q_NULLPTR != m_label)
        return m_label->text();

    return AccessibleWidget::text(startOffset, endOffset);
}

AccessiblePButton::AccessiblePButton(QPushButton *button)
    : AccessibleWidget(button)
    , m_button(button)
{

}

AccessiblePButton::~AccessiblePButton()
{

}

QString AccessiblePButton::text(int startOffset, int endOffset) const
{
    if (Q_NULLPTR != m_button)
        return m_button->text();

    return AccessibleWidget::text(startOffset, endOffset);
}



AccessibleTButton::AccessibleTButton(QToolButton *button)
    : AccessibleWidget(button)
    , m_button(button)
{

}

AccessibleTButton::~AccessibleTButton()
{

}

QString AccessibleTButton::text(int startOffset, int endOffset) const
{
    if (Q_NULLPTR != m_button)
        return m_button->text();

    return AccessibleWidget::text(startOffset, endOffset);
}

AccessibleComboBox::AccessibleComboBox(QComboBox *combobox)
    : AccessibleWidget(combobox)
    , m_combobox(combobox)
{

}

AccessibleComboBox::~AccessibleComboBox()
{

}

QString AccessibleComboBox::text(int startOffset, int endOffset) const
{
    if (Q_NULLPTR != m_combobox)
        return m_combobox->currentText();

    return AccessibleWidget::text(startOffset, endOffset);
}

AccessibleKeyLabelWidget::AccessibleKeyLabelWidget(dcc_fcitx_configtool::widgets::FcitxKeyLabelWidget *keylablewidget)
    : AccessibleWidget(keylablewidget)
    , m_keylablewidget(keylablewidget)
{

}

AccessibleKeyLabelWidget::~AccessibleKeyLabelWidget()
{

}

QString AccessibleKeyLabelWidget::text(int startOffset, int endOffset) const
{
    if (Q_NULLPTR != m_keylablewidget)
        return m_keylablewidget->getKeyToStr();

    return AccessibleKeyLabelWidget::text(startOffset, endOffset);
}

AccessibleComBoboxSettingsItem::AccessibleComBoboxSettingsItem(dcc_fcitx_configtool::widgets::FcitxComBoboxSettingsItem *comboboxsettingsitem)
    : AccessibleWidget(comboboxsettingsitem)
    , m_comboboxsettingsitem(comboboxsettingsitem)
{

}

AccessibleComBoboxSettingsItem::~AccessibleComBoboxSettingsItem()
{

}

QString AccessibleComBoboxSettingsItem::text(int startOffset, int endOffset) const
{
    if (Q_NULLPTR != m_comboboxsettingsitem)
        return m_comboboxsettingsitem->getLabelText();

    return AccessibleComBoboxSettingsItem::text(startOffset, endOffset);
}

AccessibleKeySettingsItem::AccessibleKeySettingsItem(dcc_fcitx_configtool::widgets::FcitxKeySettingsItem *keysettingsitem)
    : AccessibleWidget(keysettingsitem)
    , m_keysettingsitem(keysettingsitem)
{

}

AccessibleKeySettingsItem::~AccessibleKeySettingsItem()
{

}

QString AccessibleKeySettingsItem::text(int startOffset, int endOffset) const
{
    if (Q_NULLPTR != m_keysettingsitem)
        return m_keysettingsitem->getLabelText();

    return AccessibleKeySettingsItem::text(startOffset, endOffset);
}

AccessibleDFloatingButton::AccessibleDFloatingButton(DFloatingButton *button)
    : AccessibleWidget(button)
    , m_button(button)
{

}

AccessibleDFloatingButton::~AccessibleDFloatingButton()
{

}

QString AccessibleDFloatingButton::text(int startOffset, int endOffset) const
{
    if (Q_NULLPTR != m_button)
        return "DFloatingButton";

    return AccessibleDFloatingButton::text(startOffset, endOffset);
}

AccessibleDSearchEdit::AccessibleDSearchEdit(DSearchEdit *searchedit)
    : AccessibleWidget(searchedit)
    , m_searchedit(searchedit)
{

}

AccessibleDSearchEdit::~AccessibleDSearchEdit()
{

}

QString AccessibleDSearchEdit::text(int startOffset, int endOffset) const
{
    if (Q_NULLPTR != m_searchedit)
        return m_searchedit->text();

    return AccessibleDSearchEdit::text(startOffset, endOffset);
}

AccessibleDCommandLinkButton::AccessibleDCommandLinkButton(DCommandLinkButton *commandLinkbutton)
    : AccessibleWidget(commandLinkbutton)
    , m_commandLinkbutton(commandLinkbutton)
{

}

AccessibleDCommandLinkButton::~AccessibleDCommandLinkButton()
{

}

QString AccessibleDCommandLinkButton::text(int startOffset, int endOffset) const
{
    if (Q_NULLPTR != m_commandLinkbutton)
        return m_commandLinkbutton->text();

    return AccessibleDCommandLinkButton::text(startOffset, endOffset);
}


AccessibleTitleLabel::AccessibleTitleLabel(FcitxTitleLabel *label)
    : AccessibleWidget(label)
    , m_label(label)
{

}

AccessibleTitleLabel::~AccessibleTitleLabel()
{

}

QString AccessibleTitleLabel::text(int startOffset, int endOffset) const
{
    if (Q_NULLPTR != m_label)
        return m_label->text();

    return AccessibleTitleLabel::text(startOffset, endOffset);
}
