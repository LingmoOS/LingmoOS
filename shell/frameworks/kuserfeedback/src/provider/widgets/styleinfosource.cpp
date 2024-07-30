/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "styleinfosource.h"

#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QVariant>

using namespace KUserFeedback;

StyleInfoSource::StyleInfoSource()
    : AbstractDataSource(QStringLiteral("style"))
{
}

QString StyleInfoSource::description() const
{
    return tr("The widget style used by the application, and information about the used color scheme.");
}

QVariant StyleInfoSource::data()
{
    QVariantMap m;
    if (qApp && qApp->style())
        m.insert(QStringLiteral("style"), qApp->style()->objectName()); // QStyleFactory sets the object name to the style name
    m.insert(QStringLiteral("dark"), qApp->palette().color(QPalette::Window).lightness() < 128);
    return m;
}

QString StyleInfoSource::name() const
{
    return tr("Application style");
}
