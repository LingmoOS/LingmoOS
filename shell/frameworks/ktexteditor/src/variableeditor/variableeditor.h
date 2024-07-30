/*
    SPDX-FileCopyrightText: 2011-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef VARIABLE_EDITOR_H
#define VARIABLE_EDITOR_H

#include <QWidget>

class KateHelpButton;

class VariableBoolItem;
class VariableColorItem;
class VariableFontItem;
class VariableItem;
class VariableStringListItem;
class VariableIntItem;
class VariableStringItem;
class VariableSpellCheckItem;
class VariableRemoveSpacesItem;

class KColorCombo;
class QFontComboBox;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;

namespace Sonnet
{
class DictionaryComboBox;
}

class VariableEditor : public QWidget
{
    Q_OBJECT

public:
    explicit VariableEditor(VariableItem *item, QWidget *parent = nullptr);

    VariableItem *item() const;

Q_SIGNALS:
    void valueChanged();

protected Q_SLOTS:
    void itemEnabled(bool enabled);
    void activateItem();

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    VariableItem *m_item;

    QCheckBox *m_checkBox;
    QLabel *m_variable;
    QLabel *m_helpText;
    KateHelpButton *m_btnHelp;
};

class VariableIntEditor : public VariableEditor
{
public:
    VariableIntEditor(VariableIntItem *item, QWidget *parent);

protected:
    void setItemValue(int newValue);

private:
    QSpinBox *m_spinBox;
};

class VariableBoolEditor : public VariableEditor
{
public:
    VariableBoolEditor(VariableBoolItem *item, QWidget *parent);

protected:
    void setItemValue(int enabled);

private:
    QComboBox *m_comboBox;
};

class VariableStringListEditor : public VariableEditor
{
public:
    VariableStringListEditor(VariableStringListItem *item, QWidget *parent);

protected:
    void setItemValue(const QString &newValue);

private:
    QComboBox *m_comboBox;
};

class VariableColorEditor : public VariableEditor
{
public:
    VariableColorEditor(VariableColorItem *item, QWidget *parent);

protected:
    void setItemValue(const QColor &newValue);

private:
    KColorCombo *m_comboBox;
};

class VariableFontEditor : public VariableEditor
{
public:
    VariableFontEditor(VariableFontItem *item, QWidget *parent);

protected:
    void setItemValue(const QFont &newValue);

private:
    QFontComboBox *m_comboBox;
};

class VariableStringEditor : public VariableEditor
{
public:
    VariableStringEditor(VariableStringItem *item, QWidget *parent);

protected:
    void setItemValue(const QString &newValue);

private:
    QLineEdit *m_lineEdit;
};

class VariableSpellCheckEditor : public VariableEditor
{
public:
    VariableSpellCheckEditor(VariableSpellCheckItem *item, QWidget *parent);

protected:
    void setItemValue(const QString &newValue);

private:
    Sonnet::DictionaryComboBox *m_dictionaryCombo;
};

class VariableRemoveSpacesEditor : public VariableEditor
{
public:
    VariableRemoveSpacesEditor(VariableRemoveSpacesItem *item, QWidget *parent);

protected:
    void setItemValue(int enabled);

private:
    QComboBox *m_comboBox;
};

#endif
