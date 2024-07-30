/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "screeninfosource.h"

#include <QGuiApplication>
#include <QScreen>
#include <QVariant>

using namespace KUserFeedback;

ScreenInfoSource::ScreenInfoSource() :
    AbstractDataSource(QStringLiteral("screens"), Provider::DetailedSystemInformation)
{
}

QString ScreenInfoSource::description() const
{
    return tr("Size and resolution of all connected screens.");
}

QVariant ScreenInfoSource::data()
{
    QVariantList l;
    foreach (auto screen, QGuiApplication::screens()) {
        QVariantMap m;
        m.insert(QStringLiteral("width"), screen->size().width());
        m.insert(QStringLiteral("height"), screen->size().height());
        m.insert(QStringLiteral("dpi"), qRound(screen->physicalDotsPerInch()));
        m.insert(QStringLiteral("devicePixelRatio"), screen->devicePixelRatio());
        l.push_back(m);
    }
    return l;
}

QString ScreenInfoSource::name() const
{
    return tr("Screen parameters");
}
