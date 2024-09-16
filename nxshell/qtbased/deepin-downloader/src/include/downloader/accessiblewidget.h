// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSIBLE_WIDGET_H
#define ACCESSIBLE_WIDGET_H

#include <QAccessibleInterface>
#include <QAccessibleWidget>

class QLabel;
class QPushButton;
class QCheckBox;
class QStandardItem;
class DswitchButton;
class AccessibleWidget : public QAccessibleWidget
        , public QAccessibleTextInterface
{
public:
    explicit AccessibleWidget(QWidget *widget);
    ~AccessibleWidget();

    void *interface_cast(QAccessible::InterfaceType t) Q_DECL_OVERRIDE;
    QString text(QAccessible::Text t) const Q_DECL_OVERRIDE;

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;
    Qt::CheckState checkState(int startOffset, int endOffset) const ;
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
class AccessibleButton : public AccessibleWidget
{
public:
    explicit AccessibleButton(QPushButton *button);
    ~AccessibleButton();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    QPushButton *m_button;
};

// 可访问的复选框
class AccessibleCheckBox : public AccessibleWidget
{
public:
    explicit AccessibleCheckBox(QCheckBox *checkbox);
    ~AccessibleCheckBox();
    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;
    Qt::CheckState checkState(int startOffset, int endOffset) const;


private:
    QCheckBox *m_checkbox;
};

// 可访问的switchButton
class AccessibleStandardSwitchButton : public AccessibleWidget
{
public:
    explicit AccessibleStandardSwitchButton(QStandardItem *item);
    ~AccessibleStandardSwitchButton();
    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    DswitchButton *m_button;
};

// 可访问的Item
class AccessibleStandardItem : public AccessibleWidget
{
public:
    explicit AccessibleStandardItem(QStandardItem *item);
    ~AccessibleStandardItem();
    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    QStandardItem *m_item;
};

#endif // ACCESSIBLE_WIDGET_H
