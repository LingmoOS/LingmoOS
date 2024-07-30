/*
 *   SPDX-FileCopyrightText: 2007-2009 Shawn Starr <shawn.starr@rogers.com>
 *   SPDX-FileCopyrightText: 2012 Luís Gabriel Lima <lampih@gmail.com>
 *   SPDX-FileCopyrightText: 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *   SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef WEATHERAPPLET_H
#define WEATHERAPPLET_H

#include <Plasma5Support/DataEngineConsumer>
#include <Plasma/Applet>

class WeatherApplet : public Plasma::Applet, public Plasma5Support::DataEngineConsumer
{
    Q_OBJECT
    // used for making this information available to the config pages
    Q_PROPERTY(bool needsToBeSquare MEMBER m_needsToBeSquare NOTIFY needsToBeSquareChanged FINAL)

    Q_PROPERTY(QVariantMap providers MEMBER m_providers NOTIFY providersChanged FINAL)

public:
    WeatherApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~WeatherApplet() override;

Q_SIGNALS:
    void needsToBeSquareChanged();
    void providersChanged();

private:
    void setDefaultUnits();

private:
    QVariantMap m_providers;
    bool m_needsToBeSquare = false;
};

#endif
