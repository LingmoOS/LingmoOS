/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "orwell.h"
#include "ui_orwell.h"

#include <KUserFeedback/FeedbackConfigDialog>
#include <KUserFeedback/NotificationPopup>
#include <KUserFeedback/StyleInfoSource>
#include <KUserFeedback/ApplicationVersionSource>
#include <KUserFeedback/CompilerInfoSource>
#include <KUserFeedback/CpuInfoSource>
#include <KUserFeedback/LocaleInfoSource>
#include <KUserFeedback/OpenGLInfoSource>
#include <KUserFeedback/PlatformInfoSource>
#include <KUserFeedback/PropertyRatioSource>
#include <KUserFeedback/QPAInfoSource>
#include <KUserFeedback/QtVersionSource>
#include <KUserFeedback/ScreenInfoSource>
#include <KUserFeedback/StartCountSource>
#include <KUserFeedback/SurveyInfo>
#include <KUserFeedback/UsageTimeSource>

#include <QApplication>
#include <QDesktopServices>
#include <QSettings>

static std::unique_ptr<KUserFeedback::Provider> provider; // TODO make this nicer

Orwell::Orwell(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::Orwell)
{
    ui->setupUi(this);
    loadStats();

    connect(ui->version, &QLineEdit::textChanged, this, [this]() {
        QCoreApplication::setApplicationVersion(ui->version->text());
    });

    connect(ui->submitButton, &QPushButton::clicked, provider.get(), &KUserFeedback::Provider::submit);
    connect(ui->overrideButton, &QPushButton::clicked, this, [this] (){
        writeStats();
        QMetaObject::invokeMethod(provider.get(), "load");
        loadStats();
    });

    connect(ui->surveyInterval, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [](int value) {
        provider->setSurveyInterval(value);
    });

    ui->actionContribute->setVisible(provider->isEnabled());
    connect(ui->actionContribute, &QAction::triggered, this, [this]() {
        KUserFeedback::FeedbackConfigDialog dlg(this);
        dlg.setFeedbackProvider(provider.get());
        dlg.exec();
    });

    connect(ui->actionQuit, &QAction::triggered, qApp, &QCoreApplication::quit);

    auto propertyMonitorSource = new KUserFeedback::PropertyRatioSource(ui->dial, "value", QStringLiteral("dialRatio"));
    propertyMonitorSource->setDescription(QStringLiteral("The dial position."));
    propertyMonitorSource->addValueMapping(0, QStringLiteral("off"));
    propertyMonitorSource->addValueMapping(11, QStringLiteral("max"));
    propertyMonitorSource->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);
    provider->addDataSource(propertyMonitorSource);
    auto notifyPopup = new KUserFeedback::NotificationPopup(this);
    notifyPopup->setFeedbackProvider(provider.get());
}

Orwell::~Orwell() = default;

void Orwell::loadStats()
{
    ui->version->setText(QCoreApplication::applicationVersion());

    QSettings settings(QStringLiteral("KDE"), QStringLiteral("UserFeedback.org.kde.orwell"));
    ui->startCount->setValue(settings.value(QStringLiteral("UserFeedback/ApplicationStartCount")).toInt());
    ui->runtime->setValue(settings.value(QStringLiteral("UserFeedback/ApplicationTime")).toInt());
    ui->surveys->setText(settings.value(QStringLiteral("UserFeedback/CompletedSurveys")).toStringList().join(QStringLiteral(", ")));
    ui->surveyInterval->setValue(provider->surveyInterval());
}

void Orwell::writeStats()
{
    QSettings settings(QStringLiteral("KDE"), QStringLiteral("UserFeedback.org.kde.orwell"));
    settings.setValue(QStringLiteral("UserFeedback/ApplicationStartCount"), ui->startCount->value());
    settings.setValue(QStringLiteral("UserFeedback/ApplicationTime"), ui->runtime->value());
    settings.setValue(QStringLiteral("UserFeedback/CompletedSurveys"), ui->surveys->text().split(QStringLiteral(", ")));
}


int main(int argc, char** argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("orwell"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1984"));

    QApplication app(argc, argv);

    provider.reset(new KUserFeedback::Provider);
    provider->setFeedbackServer(QUrl(QStringLiteral("https://feedback.volkerkrause.eu")));
    provider->setSubmissionInterval(1);
    provider->setApplicationStartsUntilEncouragement(5);
    provider->setEncouragementDelay(10);
    provider->addDataSource(new KUserFeedback::ApplicationVersionSource);
    provider->addDataSource(new KUserFeedback::CompilerInfoSource);
    provider->addDataSource(new KUserFeedback::CpuInfoSource);
    provider->addDataSource(new KUserFeedback::LocaleInfoSource);
    provider->addDataSource(new KUserFeedback::OpenGLInfoSource);
    provider->addDataSource(new KUserFeedback::PlatformInfoSource);
    provider->addDataSource(new KUserFeedback::QPAInfoSource);
    provider->addDataSource(new KUserFeedback::QtVersionSource);
    provider->addDataSource(new KUserFeedback::ScreenInfoSource);
    provider->addDataSource(new KUserFeedback::StartCountSource);
    provider->addDataSource(new KUserFeedback::UsageTimeSource);
    provider->addDataSource(new KUserFeedback::StyleInfoSource);

    Orwell mainWindow;
    mainWindow.show();

    return app.exec();
}
