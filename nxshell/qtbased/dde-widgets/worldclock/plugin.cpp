// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"
#include "common/helper.hpp"
#include "accessible/accessible.h"
#include "clockpanel.h"
#include "settingsview.h"
#include "timezonemodel.h"

namespace dwclock {
QString WorldClockWidgetPlugin::title() const
{
    return tr("World Clock");
}

QString WorldClockWidgetPlugin::description() const
{
    return tr("Display clocks at different geographical locations");
}

QString WorldClockWidgetPlugin::aboutDescription() const
{
    return tr("Viewing the time and time difference of multiple cities around the world");
}

IWidget *WorldClockWidgetPlugin::createWidget()
{
    return new WorldClockWidget();
}

QWidget *WorldClockWidget::view()
{
    return m_viewManager->clockPanel(handler()->roundedCornerRadius());
}

bool WorldClockWidget::initialize(const QStringList &arguments) {
    Q_UNUSED(arguments);
    static bool hasLoaded = false;
    if (!hasLoaded)
        hasLoaded = BuildinWidgetsHelper::instance()->loadTranslator("dde-widgets-worldclock_");

    m_viewManager = new ViewManager();

    QObject::connect(m_viewManager->model(), &TimezoneModel::timezonesChanged, m_viewManager, [this]() {
        const QStringList &timezones = m_viewManager->model()->timezones();
        handler()->setValue("locations", timezones);
    });

    return true;
}

void WorldClockWidget::delayInitialize()
{
    QAccessible::installFactory(accessibleFactory);
}

void WorldClockWidget::typeChanged(const IWidget::Type type)
{
    auto clockPanel = m_viewManager->clockPanel(handler()->roundedCornerRadius());
    clockPanel->setFixedSize(handler()->size());

    clockPanel->setSmallType(type == IWidget::Small);

    const QStringList &defaultLocations = TimezoneModel::defaultLocations();
    auto data = handler()->value("locations", defaultLocations).toStringList();

    m_viewManager->updateModel(data);
}

bool WorldClockWidget::enableSettings()
{
    return true;
}

void WorldClockWidget::settings()
{
    auto settingsView = m_viewManager->settingsView();
    auto pw = m_viewManager->clockPanel(handler()->roundedCornerRadius());

    settingsView->move(pw->mapToGlobal(pw->geometry().bottomLeft()));
    if (QDialog::Accepted == settingsView->exec()) {
        m_viewManager->updateModel(settingsView->model()->timezones());
    }
    settingsView->deleteLater();
}

void WorldClockWidget::hideWidgets()
{
    m_viewManager->settingsView()->close();
}

QIcon WorldClockWidgetPlugin::logo() const
{
    return QIcon::fromTheme("dcc_nav_datetime");
}

QStringList WorldClockWidgetPlugin::contributors() const
{
    return {BuildinWidgetsHelper::instance()->contributor()};
}
}
