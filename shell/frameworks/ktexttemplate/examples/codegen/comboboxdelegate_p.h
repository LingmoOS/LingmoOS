/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef COMBOBOX_DELEGATE_P_H
#define COMBOBOX_DELEGATE_P_H

#include "comboboxdelegate.h"

#include <QComboBox>
#include <QItemEditorCreatorBase>
#include <QItemEditorFactory>

class ComboBoxEditorCreator : public QItemEditorCreatorBase
{
public:
    explicit ComboBoxEditorCreator(const QStringList &data, ComboBoxDelegate::Type type);
    virtual ~ComboBoxEditorCreator();

    /* reimp */ QWidget *createWidget(QWidget *parent) const;

    /* reimp */ QByteArray valuePropertyName() const;

private:
    QStringList m_data;
    ComboBoxDelegate::Type m_type;
};

class ViewComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(QString choice READ choice WRITE setChoice)
public:
    explicit ViewComboBox(QWidget *parent = 0);

    QString choice() const;
    void setChoice(const QString &choice);
};

#endif
