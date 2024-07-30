/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <notificationpopup.h>
#include <provider.h>
#include <surveyinfo.h>

#include <QApplication>
#include <QPushButton>
#include <QStandardPaths>
#include <QUuid>
#include <QVBoxLayout>

using namespace KUserFeedback;

int main(int argc, char **argv)
{
    QStandardPaths::setTestModeEnabled(true);

    QCoreApplication::setApplicationName(QStringLiteral("notificationpopuptest"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1984"));

    QApplication app(argc, argv);

    Provider provider;

    QWidget topLevel;
    auto layout = new QVBoxLayout(&topLevel);
    auto button = new QPushButton(QStringLiteral("Show Encouragement"));
    layout->addWidget(button);
    QObject::connect(button, &QPushButton::clicked, &provider, &Provider::showEncouragementMessage);
    button = new QPushButton(QStringLiteral("New Survey Available"));
    layout->addWidget(button);
    QObject::connect(button, &QPushButton::clicked, &provider, [&provider]() {
        SurveyInfo info;
        info.setUuid(QUuid(QStringLiteral("{9e529dfa-0213-413e-a1a8-8a9cea7d5a97}")));
        info.setUrl(QUrl(QStringLiteral("https://www.kde.org/")));
        Q_EMIT provider.surveyAvailable(info);
    });

    auto popup = new NotificationPopup(&topLevel);
    popup->setFeedbackProvider(&provider);

    topLevel.resize(1024, 786);
    topLevel.show();
    return app.exec();
}
