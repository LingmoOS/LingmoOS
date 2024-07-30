/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "metaenumcombobox.h"

#include <QDebug>
#include <QMetaEnum>
#include <QMetaType>

using namespace KUserFeedback::Console;

MetaEnumComboBox::MetaEnumComboBox(QWidget* parent) :
    QComboBox(parent)
{
}

MetaEnumComboBox::~MetaEnumComboBox()
{
}

QVariant MetaEnumComboBox::value() const
{
    auto value = m_value;
    *(static_cast<int*>(value.data())) = currentData().toInt();
    return value;
}

void MetaEnumComboBox::setValue(const QVariant& value)
{
    clear();
    m_value = value;

    const auto mo = QMetaType::metaObjectForType(value.userType());
    if (!mo)
        return;

    const QByteArray typeName = value.typeName();
    const auto idx = typeName.lastIndexOf("::");
    if (idx <= 0)
        return;

    const auto enumName = typeName.mid(idx + 2);
    const auto enumIdx = mo->indexOfEnumerator(enumName.constData());
    if (enumIdx < 0)
        return;

    const auto me = mo->enumerator(enumIdx);
    for (int i = 0; i < me.keyCount(); ++i) {
        addItem(QLatin1String(me.key(i)), me.value(i));
        if (me.value(i) == value.toInt())
            setCurrentIndex(i);
    }
}

#include "moc_metaenumcombobox.cpp"
