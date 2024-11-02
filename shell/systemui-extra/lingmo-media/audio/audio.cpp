/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "audio.h"
#include "ui_audio.h"

#include <QDebug>
#include <QLibraryInfo>
#include <QStandardPaths>
Audio::Audio() : mFirstLoad(true)
{
#ifndef QT_NO_TRANSLATION
    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *pTranslator = new QTranslator();
    if (pTranslator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        QApplication::installTranslator(pTranslator);
#endif
    QTranslator *translator = new QTranslator(this);
    translator->load("/usr/share/lingmo-media/translations/audio/" + QLocale::system().name());
    QApplication::installTranslator(translator);
    pluginName = tr("Audio");
    pluginType = SYSTEM;
}

Audio::~Audio()
{
}

QString Audio::plugini18nName()
{
    return pluginName;
}

int Audio::pluginTypes()
{
    return pluginType;
}

QWidget *Audio::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        widget = new UkmediaMainWidget;
    }
    return widget;
}

bool Audio::isEnable() const
{
    return true;
}

const QString Audio::name() const
{
    return QStringLiteral("Audio");
}

bool Audio::isShowOnHomePage() const
{
    return true;
}

QIcon Audio::icon() const
{
    return QIcon::fromTheme("audio-volume-high-symbolic");
}

QString Audio::translationPath() const
{
    return "/usr/share/lingmo-media/translations/audio/%1.ts";
}

void Audio::initSearchText()
{
    //~ contents_path /UkccPlugin/UkccPlugin
    tr("UkccPlugin");
    //~ contents_path /UkccPlugin/ukccplugin test
    tr("ukccplugin test");
}

