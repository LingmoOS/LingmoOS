/*
    SPDX-FileCopyrightText: 2011-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef VARIABLE_ITEM_H
#define VARIABLE_ITEM_H

#include <QColor>
#include <QFont>
#include <QString>
#include <QStringList>

class VariableEditor;

// BEGIN class VariableItem
class VariableItem
{
public:
    explicit VariableItem(const QString &variable);
    virtual ~VariableItem() = default;

    QString variable() const;
    QString helpText() const;
    void setHelpText(const QString &text);

    bool isActive() const;
    void setActive(bool active);

    virtual void setValueByString(const QString &value) = 0;
    virtual QString valueAsString() const = 0;

    virtual VariableEditor *createEditor(QWidget *parent) = 0;

private:
    // not implemented: copy constructor and operator=
    VariableItem(const VariableItem &copy);
    VariableItem &operator=(const VariableItem &other);

    QString m_variable;
    QString m_helpText;
    bool m_active;
};
// END class VariableItem

//
// DERIVE A CLASS FOR EACH TYPE
//

// BEGIN class VariableIntItem
class VariableIntItem : public VariableItem
{
public:
    explicit VariableIntItem(const QString &variable, int value);

    int value() const;
    void setValue(int newValue);
    void setRange(int minValue, int maxValue);
    int minValue() const;
    int maxValue() const;

public:
    void setValueByString(const QString &value) override;
    QString valueAsString() const override;
    VariableEditor *createEditor(QWidget *parent) override;

private:
    int m_value;
    int m_minValue;
    int m_maxValue;
};
// END class VariableIntItem

// BEGIN class VariableStringListItem
class VariableStringListItem : public VariableItem
{
public:
    explicit VariableStringListItem(const QString &variable, const QStringList &slist, const QString &value);

    QStringList stringList() const;

    QString value() const;
    void setValue(const QString &newValue);

public:
    void setValueByString(const QString &value) override;
    QString valueAsString() const override;
    VariableEditor *createEditor(QWidget *parent) override;

private:
    QStringList m_list;
    QString m_value;
};
// END class VariableStringListItem

// BEGIN class VariableBoolItem
class VariableBoolItem : public VariableItem
{
public:
    explicit VariableBoolItem(const QString &variable, bool value);

    bool value() const;
    void setValue(bool enabled);

public:
    void setValueByString(const QString &value) override;
    QString valueAsString() const override;
    VariableEditor *createEditor(QWidget *parent) override;

private:
    bool m_value;
};
// END class VariableBoolItem

// BEGIN class VariableColorItem
class VariableColorItem : public VariableItem
{
public:
    explicit VariableColorItem(const QString &variable, const QColor &value);

    QColor value() const;
    void setValue(const QColor &color);

public:
    void setValueByString(const QString &value) override;
    QString valueAsString() const override;
    VariableEditor *createEditor(QWidget *parent) override;

private:
    QColor m_value;
};
// END class VariableColorItem

// BEGIN class VariableFontItem
class VariableFontItem : public VariableItem
{
public:
    explicit VariableFontItem(const QString &variable, const QFont &value);

    QFont value() const;
    void setValue(const QFont &value);

public:
    void setValueByString(const QString &value) override;
    QString valueAsString() const override;
    VariableEditor *createEditor(QWidget *parent) override;

private:
    QFont m_value;
};
// END class VariableFontItem

// BEGIN class VariableStringItem
class VariableStringItem : public VariableItem
{
public:
    explicit VariableStringItem(const QString &variable, const QString &value);

    QString value() const;
    void setValue(const QString &value);

public:
    void setValueByString(const QString &value) override; // Same as setValue() in this case, implemented for uniformity
    QString valueAsString() const override; // Same as value() in this case, implemented for uniformity
    VariableEditor *createEditor(QWidget *parent) override;

private:
    QString m_value;
};

// END class VariableStringItem

// BEGIN class VariableSpellCheckItem
class VariableSpellCheckItem : public VariableItem
{
public:
    explicit VariableSpellCheckItem(const QString &variable, const QString &value);

    QString value() const;
    void setValue(const QString &value);

public:
    void setValueByString(const QString &value) override; // Same as setValue() in this case, implemented for uniformity
    QString valueAsString() const override; // Same as value() in this case, implemented for uniformity
    VariableEditor *createEditor(QWidget *parent) override;

private:
    QString m_value;
};
// END class VariableSpellCheckItem

// BEGIN class VariableRemoveSpacesItem
class VariableRemoveSpacesItem : public VariableItem
{
public:
    explicit VariableRemoveSpacesItem(const QString &variable, int value);

    int value() const;
    void setValue(int value);

public:
    void setValueByString(const QString &value) override; // Same as setValue() in this case, implemented for uniformity
    QString valueAsString() const override; // Same as value() in this case, implemented for uniformity
    VariableEditor *createEditor(QWidget *parent) override;

private:
    int m_value;
};
// END class VariableRemoveSpacesItem

#endif
