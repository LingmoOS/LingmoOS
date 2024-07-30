/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "emaildetailswidget.h"

#include <QLabel>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KPeople/PersonData>
#include <KPluginFactory>

using namespace KPeople;

QWidget *EmailFieldsPlugin::createDetailsWidget(const PersonData &person, QWidget *parent) const
{
    if (person.allEmails().isEmpty()) {
        return nullptr;
    }
    QWidget *widget = new QWidget(parent);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    const auto lstEmails = person.allEmails();
    for (const QString &email : lstEmails) {
        layout->addWidget(new QLabel(email));
    }
    return widget;
}

QString EmailFieldsPlugin::label() const
{
    return i18nc("E-mail field label", "E-mail");
}

int EmailFieldsPlugin::sortWeight() const
{
    return 50;
}
