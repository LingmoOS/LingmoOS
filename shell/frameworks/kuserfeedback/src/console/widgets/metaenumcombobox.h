/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_METAENUMCOMBOBOX_H
#define KUSERFEEDBACK_CONSOLE_METAENUMCOMBOBOX_H

#include <QComboBox>

namespace KUserFeedback {
namespace Console {

class MetaEnumComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue USER true)
public:
    explicit MetaEnumComboBox(QWidget *parent = nullptr);
    ~MetaEnumComboBox() override;

    QVariant value() const;
    void setValue(const QVariant &value);

private:
    QVariant m_value;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_METAENUMCOMBOBOX_H
