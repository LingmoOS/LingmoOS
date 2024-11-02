/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <QString>
#include <QDebug>

#include "gsettings.hpp"
#include "gsettings_test.hpp"

GsettingTest::GsettingTest()
{
    this->systemGsettingsTest();
    this->commonGsettingsTest();
    this->signalGsettingsTest();
}

GsettingTest::~GsettingTest() {}

void GsettingTest::systemGsettingsTest(void)
{
    QString systemFontSize = kdk::kabase::Gsettings::getSystemFontSize().toString();
    double systemTransparency = kdk::kabase::Gsettings::getSystemTransparency().toDouble();
    QString systemItem = kdk::kabase::Gsettings::getSystemTheme().toString();

    qInfo() << "system font size : " << systemFontSize;
    qInfo() << "system transparency : " << systemTransparency;
    qInfo() << "system item : " << systemItem;

    return;
}

void GsettingTest::commonGsettingsTest(void)
{
    if (kdk::kabase::Gsettings::registerObject(QString("A"), QByteArray("org.lingmo.style"))) {
        qCritical() << "register gsettings fail !";
        return;
    }

    QString fontSize = kdk::kabase::Gsettings::get(QString("A"), QString("systemFontSize")).toString();
    qInfo() << "get system font size : " << fontSize;

    if (kdk::kabase::Gsettings::set(QString("A"), QString("systemFontSize"), QVariant(14))) {
        qCritical() << "set system font size fail !";
        return;
    }

    if (kdk::kabase::Gsettings::reset(QString("A"), QString("systemFontSize"))) {
        qCritical() << "reset system font size fail !";
        return;
    }

    QStringList keyList = kdk::kabase::Gsettings::keys(QString("A"));
    qInfo() << "key list : " << keyList;

    if (kdk::kabase::Gsettings::cancelRegister(QString("A"))) {
        qCritical() << "cancel register fail !";
        return;
    }

    return;
}

void GsettingTest::signalGsettingsTest(void)
{
    this->connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemFontSizeChange, this, [=]() {
        qInfo() << kdk::kabase::Gsettings::getSystemFontSize();
    });

    this->connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemThemeChange, this, [=]() {
        qInfo() << kdk::kabase::Gsettings::getSystemTheme();
    });

    this->connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemTransparencyChange, this, [=]() {
        qInfo() << kdk::kabase::Gsettings::getSystemTransparency();
    });

    return;
}
