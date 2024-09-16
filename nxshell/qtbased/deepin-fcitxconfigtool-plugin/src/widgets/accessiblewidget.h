// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSIBLE_WIDGET_H
#define ACCESSIBLE_WIDGET_H

#include <QAccessibleInterface>
#include <QAccessibleWidget>
#include "keysettingsitem.h"
#include <DFloatingButton>
#include <DSearchEdit>
#include <DCommandLinkButton>

class FcitxTitleLabel;
class QLabel;
class QPushButton;
class QToolButton;
class QComboBox;

class AccessibleWidget : public QAccessibleWidget
        , public QAccessibleTextInterface
{
public:
    explicit AccessibleWidget(QWidget *widget);
    ~AccessibleWidget();

    void *interface_cast(QAccessible::InterfaceType t) Q_DECL_OVERRIDE;
    QString text(QAccessible::Text t) const Q_DECL_OVERRIDE;

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;
    void selection(int selectionIndex, int *startOffset, int *endOffset) const Q_DECL_OVERRIDE;
    int selectionCount() const Q_DECL_OVERRIDE;
    void addSelection(int startOffset, int endOffset) Q_DECL_OVERRIDE;
    void removeSelection(int selectionIndex) Q_DECL_OVERRIDE;
    void setSelection(int selectionIndex, int startOffset, int endOffset) Q_DECL_OVERRIDE;
    int cursorPosition() const Q_DECL_OVERRIDE;
    void setCursorPosition(int position) Q_DECL_OVERRIDE;
    int characterCount() const Q_DECL_OVERRIDE;
    QRect characterRect(int offset) const Q_DECL_OVERRIDE;
    int offsetAtPoint(const QPoint &point) const Q_DECL_OVERRIDE;
    void scrollToSubstring(int startIndex, int endIndex) Q_DECL_OVERRIDE;
    QString attributes(int offset, int *startOffset, int *endOffset) const Q_DECL_OVERRIDE;

private:
    QWidget *m_widget;
};

// 可访问的标签
class AccessibleLabel : public AccessibleWidget
{
public:
    explicit AccessibleLabel(QLabel *label);
    ~AccessibleLabel();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    QLabel *m_label;
};

// 可访问的按钮
class AccessiblePButton : public AccessibleWidget
{
public:
    explicit AccessiblePButton(QPushButton *button);
    ~AccessiblePButton();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    QPushButton *m_button;
};

// 可访问的按钮
class AccessibleTButton : public AccessibleWidget
{
public:
    explicit AccessibleTButton(QToolButton *button);
    ~AccessibleTButton();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    QToolButton *m_button;
};

// 可访问的按钮
class AccessibleComboBox : public AccessibleWidget
{
public:
    explicit AccessibleComboBox(QComboBox *combobox);
    ~AccessibleComboBox();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    QComboBox *m_combobox;
};

// 可访问的按钮
class AccessibleKeyLabelWidget: public AccessibleWidget
{
public:
    explicit AccessibleKeyLabelWidget(dcc_fcitx_configtool::widgets::FcitxKeyLabelWidget *keylablewidget);
    ~AccessibleKeyLabelWidget();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    dcc_fcitx_configtool::widgets::FcitxKeyLabelWidget *m_keylablewidget;
};

// 可访问的按钮
class AccessibleKeySettingsItem: public AccessibleWidget
{
public:
    explicit AccessibleKeySettingsItem(dcc_fcitx_configtool::widgets::FcitxKeySettingsItem *keysettingsitem);
    ~AccessibleKeySettingsItem();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    dcc_fcitx_configtool::widgets::FcitxKeySettingsItem *m_keysettingsitem;
};

// 可访问的按钮
class AccessibleComBoboxSettingsItem: public AccessibleWidget
{
public:
    explicit AccessibleComBoboxSettingsItem(dcc_fcitx_configtool::widgets::FcitxComBoboxSettingsItem *comboboxsettingsitem);
    ~AccessibleComBoboxSettingsItem();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    dcc_fcitx_configtool::widgets::FcitxComBoboxSettingsItem *m_comboboxsettingsitem;
};

// 可访问的按钮
class AccessibleDFloatingButton: public AccessibleWidget
{
public:
    explicit AccessibleDFloatingButton(DFloatingButton *button);
    ~AccessibleDFloatingButton();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    DFloatingButton *m_button;
};

// 可访问的按钮
class AccessibleDSearchEdit: public AccessibleWidget
{
public:
    explicit AccessibleDSearchEdit(DSearchEdit *searchedit);
    ~AccessibleDSearchEdit();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    DSearchEdit *m_searchedit;
};

// 可访问的按钮
class AccessibleDCommandLinkButton: public AccessibleWidget
{
public:
    explicit AccessibleDCommandLinkButton(DCommandLinkButton *commandLinkbutton);
    ~AccessibleDCommandLinkButton();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    DCommandLinkButton *m_commandLinkbutton;
};

// 可访问的按钮
class AccessibleTitleLabel : public AccessibleWidget
{
public:
    explicit AccessibleTitleLabel(FcitxTitleLabel *label);
    ~AccessibleTitleLabel();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    FcitxTitleLabel *m_label;
};


#endif // ACCESSIBLE_WIDGET_H
