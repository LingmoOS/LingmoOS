// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef ACCESSIBLE_WIDGET_H
#define ACCESSIBLE_WIDGET_H

#include <QAccessibleInterface>
#include <QAccessibleWidget>

class QLabel;
class QPushButton;
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
class AccessibleButton : public AccessibleWidget
{
public:
    explicit AccessibleButton(QPushButton *button);
    ~AccessibleButton();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    QPushButton *m_button;
};

// 可访问的按钮
class AccessibleComboBox : public AccessibleWidget
{
public:
    explicit AccessibleComboBox(QComboBox *comboBox);
    ~AccessibleComboBox();

    QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    QComboBox *m_comboBox;
};

#endif // ACCESSIBLE_WIDGET_H
