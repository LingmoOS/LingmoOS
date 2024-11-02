/*
 * Qt5-LINGMO's Library
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "lingmo-style-plugin.h"
#include "qt5-config-style-lingmo/lingmo-config-style.h"

#include "settings/black-list.h"
#include "settings/lingmo-style-settings.h"
#include "effects/highlight-effect.h"
#include "settings/application-style-settings.h"

#include <QApplication>
#include <QTimer>

#include <QDebug>
#include <QStyleFactory>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnection>

LINGMOStylePlugin::LINGMOStylePlugin(QObject *parent) :
    QStylePlugin(parent)
{
    qDebug() << "LINGMOStylePlugin.........";
    if (LINGMOStyleSettings::isSchemaInstalled("org.lingmo.style")) {
        auto settings = LINGMOStyleSettings::globalInstance();
        connect(settings, &LINGMOStyleSettings::changed, this, [=](const QString &key) {
            if (key == "styleName" || key == "widgetThemeName" || key == "themeColor" ||
                    key == "style-name" || key == "widget-theme-name" || key == "theme-color") {
                if (qApp->property("skipLINGMOTheme").isValid()&&qApp->property("skipLINGMOTheme").canConvert<bool>())
                    return;

                if (blackList().contains(qAppName()) || qAppName() == "lingmo-software-center.py")
                    return;

                //We should not swich a application theme which use internal style.
                if (QApplication::style()->inherits("InternalStyle"))
                    return;

//                auto appStyleSettings = ApplicationStyleSettings::getInstance();
//                if (appStyleSettings->currentStyleStretagy() != ApplicationStyleSettings::Default)
//                    return;

                qDebug() << "LINGMOStyleSettings change...." << key;
                auto styleName = settings->get("styleName").toString();
                auto widgetThemeName = settings->get("widgetThemeName").toString();

                if (widgetThemeName != "") {
                    if (widgetThemeName == "classical") {
                        styleName ="lingmo-config";
                    } else {
                        styleName = "lingmo-config";
                    }
                    qApp->setStyle(new LINGMOConfigStyle(styleName));

//                        if(key == "widgetThemeName" || key == "widget-theme-name"){
//                            qDebug() << "qApp style....." << qApp->style();
//                            auto configStyle = qobject_cast<LINGMOConfigStyle *>(qApp->style());
//                            if(configStyle){
//                                int max, normal, min;
//                                max = configStyle->property("windowRadius").isValid() ? configStyle->property("windowRadius").toInt() : settings->get("windowRadius").toInt();
//                                normal = configStyle->property("normalRadius").isValid() ? configStyle->property("normalRadius").toInt() : settings->get("normalRadius").toInt();
//                                min = configStyle->property("minRadius").isValid() ? configStyle->property("minRadius").toInt() : settings->get("minRadius").toInt();
//                                settings->trySet("max-radius", max);
//                                settings->trySet("normal-radius", normal);
//                                settings->trySet("min-radius", min);
//                            }
//                        }

                    return;
                }

                //other style
                for (auto keys : QStyleFactory::keys()) {
                    if (styleName.toLower() == keys.toLower()) {
                        qApp->setStyle(new QProxyStyle(styleName));
                        return;
                    }
                }

                //default style fusion
                qApp->setStyle(new QProxyStyle(styleName));
                return;
            }

            if (key == "systemPalette") {
                onSystemPaletteChanged();
            }

            if (key == "useSystemPalette") {
                onSystemPaletteChanged();
            }

            /*
            if(key == "window-radius" || key == "windowRadius"){
                auto radius = settings->get("windowRadius").toInt();
                foreach (QWidget *widget, qApp->allWidgets()) {
                    widget->setProperty("windowRadius", radius);
                    widget->repaint();
                }
            }
            if(key == "normal-radius" || key == "normalRadius"){
                auto radius = settings->get("normalRadius").toInt();
                //qDebug() << "normalRadius.........." << radius;
                foreach (QWidget *widget, qApp->allWidgets()) {
                    widget->setProperty("normalRadius", radius);
                    widget->repaint();
                }
            }
            if(key == "min-radius" || key == "minRadius"){
                auto radius = settings->get("minRadius").toInt();
                foreach (QWidget *widget, qApp->allWidgets()) {
                    widget->setProperty("minRadius", radius);
                    widget->repaint();
                }
            }
            */
        });
    }

    QDBusInterface *interFace = new QDBusInterface("com.lingmo.statusmanager.interface",
                                                   "/",
                                                   "com.lingmo.statusmanager.interface",
                                                    QDBusConnection::sessionBus());
    if(interFace->isValid()){
        connect(interFace, SIGNAL(mode_change_signal(bool)), this,  SLOT(tableModeChanged(bool)));
    }
}

QStyle *LINGMOStylePlugin::create(const QString &key)
{
    if (blackList().contains(qAppName()))
        return new Style;

    if (qApp->property("skipLINGMOTheme").isValid()&&qApp->property("skipLINGMOTheme").canConvert<bool>()){
        bool isSkipLINGMOTheme = qApp->property("skipLINGMOTheme").toBool();
        if (isSkipLINGMOTheme)
            return new Style;
    }

    auto settings = LINGMOStyleSettings::globalInstance();
    auto widgetThemeName = settings->get("widgetThemeName").toString();
    auto styleName = settings->get("styleName").toString();

    if (widgetThemeName != "") {
        if (widgetThemeName == "classical") {
            return new LINGMOConfigStyle(styleName);
        } else {
            return new LINGMOConfigStyle(styleName);
        }

    }
    return new LINGMOConfigStyle(widgetThemeName);
}

const QStringList LINGMOStylePlugin::blackList()
{
    return blackAppList();
}

void LINGMOStylePlugin::onSystemPaletteChanged()
{
    bool useSystemPalette = LINGMOStyleSettings::globalInstance()->get("useSystemPalette").toBool();
    if (useSystemPalette) {
        auto data = LINGMOStyleSettings::globalInstance()->get("systemPalette");
        if (data.isNull())
            return;
        auto palette = qvariant_cast<QPalette>(data);
        QApplication::setPalette(palette);
    } else {
        auto palette = QApplication::style()->standardPalette();
        QApplication::setPalette(palette);
    }
}

void LINGMOStylePlugin::tableModeChanged(bool isTableMode)
{
    if (blackList().contains(qAppName()))
        return;

    //We should not swich a application theme which use internal style.
    if (QApplication::style()->inherits("InternalStyle"))
        return;

//    auto appStyleSettings = ApplicationStyleSettings::getInstance();
//    if (appStyleSettings->currentStyleStretagy() != ApplicationStyleSettings::Default)
//        return;

    auto settings = LINGMOStyleSettings::globalInstance();
    auto styleName = settings->get("styleName").toString();

    if (styleName == "lingmo-default" || styleName == "lingmo-dark" || styleName == "lingmo-white"
            || styleName == "lingmo-black" || styleName == "lingmo-light" || styleName == "lingmo" || styleName == "lingmo-config")

    {
        if (qApp->property("customStyleName").isValid()) {
            if (qApp->property("customStyleName").toString() == "lingmo-light") {
                styleName = "lingmo-light";
            } else if (qApp->property("customStyleName").toString() == "lingmo-dark") {
                styleName = "lingmo-dark";
            }
        }

        foreach (QWidget *widget, qApp->allWidgets()) {
            widget->updateGeometry();
        }
        qApp->setProperty("isInitial", false);
        auto settings = LINGMOStyleSettings::globalInstance();
        auto widgetThemeName = settings->get("widgetThemeName").toString();
        auto styleName = settings->get("styleName").toString();

        if (widgetThemeName != "") {
            if (widgetThemeName == "classical") {
                qApp->setStyle(new LINGMOConfigStyle(styleName));
            } else {
                qApp->setStyle(new LINGMOConfigStyle(styleName));
            }

        }
        else
            qApp->setStyle(new LINGMOConfigStyle(styleName));

    }
    /*
    foreach (QObject *obj, qApp->children()) {
        if(qobject_cast<const QVBoxLayout*>(obj))//obj->inherits("QVBoxLayout"))
        {
            auto *vlayout = qobject_cast<QVBoxLayout *>(obj);
            qDebug() << "vlayout..." << vlayout->spacing();

            //layout->update();
        }
        else if(qobject_cast<QHBoxLayout *>(obj))
        {
            auto *hlayout = qobject_cast<QHBoxLayout *>(obj);
            qDebug() << "hlayout..." << hlayout->spacing();

            //layout->update();
        }
    }
    foreach (QWidget *widget, qApp->topLevelWidgets()) {
        qDebug() << "widget size...." << widget->size() << widget->minimumSizeHint() << widget->minimumSize();
        widget->adjustSize();
        if(widget->layout() != nullptr)
        {
            qDebug() << "layoutSpace...." << widget->layout()->spacing();
        }
//        if(widget->size() != widget->minimumSizeHint())
//            widget->setGeometry(widget->x(), widget->y(), widget->minimumSizeHint().width(), widget->minimumSizeHint().height());
    }
*/
    return;
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(lingmo-config-style, LINGMOStylePlugin)
#endif // QT_VERSION < 0x050000
