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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "readconfig.h"
#include <QDebug>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QStandardPaths>
#include <QMetaEnum>
#include "platformthemedebug.h"

ReadConfig::ReadConfig(const QString &cfg)
{

}

ReadConfig::~ReadConfig()
{

}

bool ReadConfig::load(const QString &cfg)
{
    if (!QFile::exists(cfg)) {
        qWarning() << "ReadConfig load file not exists!" << cfg;
        return false;
    }

    QFile file(cfg);
    if(!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ReadConfig load file Error!" << cfg;
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();
    QJsonParseError error;
    QJsonDocument jdoc = QJsonDocument::fromJson(jsonData, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning("QJsonDocument fromJson error!");
        return false;
    }
    m_cfgPath = cfg;
    qDebug() << "m_cfgPath..." << m_cfgPath;
    m_cfgJsonObj = jdoc.object();
    return true;
}

QVariant ReadConfig::getValue(const QString key_1, const QString key_2, const QString key_3, const QString key_4, const QString key_5) const
{
    QJsonObject obj;
    if (!key_1.isEmpty() && key_2.isEmpty()) {
        return getValue(key_1, m_cfgJsonObj);
    }

    if (!key_1.isEmpty() && !key_2.isEmpty()) {
        obj = getJsonObjectValue(key_1, m_cfgJsonObj);
    }

    if(!key_2.isEmpty()) {
        if(key_3.isEmpty())
            return getValue(key_2, obj);
        else
            obj = getJsonObjectValue(key_2, obj);
    }

    if (!key_3.isEmpty()) {
        if (key_4.isEmpty())
            return getValue(key_3, obj);
        else
            obj = getJsonObjectValue(key_3, obj);
    }

    if (!key_4.isEmpty()) {
        if(key_5.isEmpty())
            return getValue(key_4, obj);
        else
            obj = getJsonObjectValue(key_4, obj);
    }

    if(!key_5.isEmpty())
        return getValue(key_5, obj);
    return QVariant();
}

QVariant ReadConfig::getValue(const QString key, QJsonObject obj) const
{
    QVariant r;

    if (key.isEmpty() || obj.isEmpty() || !obj.contains(key)) {
        cWarning << "key or QJsonObject unvalue!" << key << obj.isEmpty() << obj.contains(key);
        return QVariant();
    }

    r = obj.value(key);
    if (!r.isValid()) {
        cWarning << "getValue key:" << key << "error!";
        return QVariant();
    }
    return r;
}

QJsonObject ReadConfig::getJsonObjectValue(const QString key, QJsonObject obj) const
{
    QJsonObject o;
    if (key.isEmpty() || obj.isEmpty() || !obj.contains(key)) {
        cWarning << "key or QJsonObject unvalue!";
        return o;
    }

    QJsonValue v = obj.value(key);
//  cDebug << "getJsonObjectValue:" << key << v;
    if (!v.isObject()) {
        cWarning << "getValue QJsonValue to QJsonObject error!";
        return o;
    }
    o = v.toObject();

    return o;
}

bool ReadConfig::getColorValue(QColor &c, const QString key_1, const QString key_2, const QString key_3, const QString key_4, const QString key_5) const
{
    QJsonObject obj = getValue(key_1, key_2, key_3, key_4, key_5).toJsonObject();
    if (obj.isEmpty()) {
        cDebug << "getvalue isempty!" << key_1 << key_2 << key_3 << key_4 << key_5;
        return false;
    }
    if (obj.value(Obj_Type) != Obj_Color_Type && obj.value(Obj_Type) != Obj_PaletteRole_Type) {
        cDebug << "get color value type error!" << obj.value(Obj_Type).toString() << key_1 << key_2 << key_3 << key_4 << key_5 << obj.value(Obj_Type);
        return false;
    }
    if(obj.value(Obj_Value).toString().isEmpty()){
        cDebug << "obj" << Obj_Value << "key isempty!";
        return false;
    }
    if(obj.value(Obj_Type) == Obj_PaletteRole_Type)
        c = stringToPaletteColor(obj.value(Obj_Value).toString());
    else
        c = stringToColor(obj.value(Obj_Value).toString());
    return true;
}

bool ReadConfig::getColorValue(QBrush &brush, const QString key_1, const QString key_2, const QString key_3, const QString key_4, const QString key_5) const
{
    QJsonObject obj = getValue(key_1, key_2, key_3, key_4, key_5).toJsonObject();
    if (obj.isEmpty()) {
        cDebug << "getvalue isempty!" << key_1 << key_2 << key_3 << key_4 << key_5;
        return false;
    }
    if (obj.value(Obj_Type) != Obj_Color_Type && obj.value(Obj_Type) != Obj_PaletteRole_Type) {
        cDebug << "get color value type error!" << obj.value(Obj_Type).toString()  << key_1 << key_2 << key_3 << key_4 << key_5 << obj.value(Obj_Type);
        return false;
    }
    if(obj.value(Obj_Value).toString().isEmpty()){
        cDebug << "obj" << Obj_Value << "key isempty!";
        return false;
    }
    if(obj.value(Obj_Type) == Obj_PaletteRole_Type)
        brush = stringToPaletteColor(obj.value(Obj_Value).toString());
    else
        brush = stringToColor(obj.value(Obj_Value).toString());
    return true;
}

bool ReadConfig::getGradientValue(QList<QColor> &cl, const QString key_1, const QString key_2, const QString key_3, const QString key_4, const QString key_5) const
{
    QJsonObject obj = getValue(key_1, key_2, key_3, key_4, key_5).toJsonObject();
    if (obj.isEmpty()) {
        cDebug << "getvalue isempty!" << key_1 << key_2 << key_3 << key_4 << key_5;
        return false;
    }
    if(obj.value(Obj_Value).toString().isEmpty()){
        cDebug << "obj" << Obj_Value << "key isempty!";
        return false;
    }
    if (obj.value(Obj_Type) != Obj_Gradient_Type) {
        cDebug << "get color value type error!" << obj.value(Obj_Type).toString()  << key_1 << key_2 << key_3 << key_4 << key_5 << obj.value(Obj_Type);
        return false;
    }
    stringToGradient(cl, obj.value(Obj_Value).toString());
    return true;

}

QString ReadConfig::getType(const QString key_1, const QString key_2, const QString key_3, const QString key_4, const QString key_5) const
{
    QJsonObject obj = getValue(key_1, key_2, key_3, key_4, key_5).toJsonObject();
    if (obj.isEmpty()) {
        cDebug << "getvalue isempty!" << key_1 << key_2 << key_3 << key_4 << key_5;
        return "";
    }
    return obj.value(Obj_Type).toString();
}

QColor ReadConfig::stringToColor(QString s) const
{
    QColor c = Qt::NoBrush;

    QStringList list = s.split(",");
    if (list.length() >= 3) {
        c.setRed(list[0].toInt());
        c.setGreen(list[1].toInt());
        c.setBlue(list[2].toInt());
        if (list.length() == 4) {
            QStringList alphaList = QString(list[3]).split("*");
            if (alphaList.length() == 2) {
                if (alphaList[0].toFloat() == 255.0)
                    c.setAlphaF(alphaList[1].toFloat());
                else if (alphaList[1].toFloat() == 255.0)
                    c.setAlphaF(alphaList[0].toFloat());
                else
                    c.setAlphaF(alphaList[0].toFloat() * alphaList[1].toFloat() / 255.0);

            } else if (alphaList.length() == 1) {
                if(alphaList[0].toFloat() <= 1.0)
                    c.setAlphaF(alphaList[0].toFloat());
                else
                    c.setAlphaF(alphaList[0].toFloat()/255.0);
            } else
                c.setAlphaF(1.0);
        }
    } else if (list.length() == 1 && list[0].startsWith("#")) {
        c = list[0];
    }

    return c;
}

QColor ReadConfig::stringToPaletteColor(QString s) const
{
    QColor c = Qt::NoBrush;
    QMetaEnum metaEnum = QMetaEnum::fromType<LINGMOPaletteRole::PaletteRole>();
    QByteArray ba=s.toLatin1();
    const char * c_scence=ba.data();
    //qDebug()<<c_scence;

    switch (metaEnum.keyToValue(c_scence)) {
    case LINGMOPaletteRole::WindowText_at:
        c = m_platte.color(QPalette::Active, QPalette::WindowText);
        break;
    case LINGMOPaletteRole::WindowText_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::WindowText);
        break;
    case LINGMOPaletteRole::WindowText_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::WindowText);
        break;
    case LINGMOPaletteRole::Button_at:
        c = m_platte.color(QPalette::Active, QPalette::Button);
        break;
    case LINGMOPaletteRole::Button_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::Button);
        break;
    case LINGMOPaletteRole::Button_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::Button);
        break;
    case LINGMOPaletteRole::Light_at:
        c = m_platte.color(QPalette::Active, QPalette::Light);
        break;
    case LINGMOPaletteRole::Light_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::Light);
        break;
    case LINGMOPaletteRole::Light_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::Light);
        break;
    case LINGMOPaletteRole::Midlight_at:
        c = m_platte.color(QPalette::Active, QPalette::Midlight);
        break;
    case LINGMOPaletteRole::Midlight_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::Midlight);
        break;
    case LINGMOPaletteRole::Midlight_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::Midlight);
        break;
    case LINGMOPaletteRole::Dark_at:
        c = m_platte.color(QPalette::Active, QPalette::Dark);
        break;
    case LINGMOPaletteRole::Dark_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::Dark);
        break;
    case LINGMOPaletteRole::Dark_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::Dark);
        break;
    case LINGMOPaletteRole::Mid_at:
        c = m_platte.color(QPalette::Active, QPalette::Mid);
        break;
    case LINGMOPaletteRole::Mid_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::Mid);
        break;
    case LINGMOPaletteRole::Mid_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::Mid);
        break;
    case LINGMOPaletteRole::Text_at:
        c = m_platte.color(QPalette::Active, QPalette::Text);
        break;
    case LINGMOPaletteRole::Text_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::Text);
        break;
    case LINGMOPaletteRole::Text_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::Text);
        break;
    case LINGMOPaletteRole::BrightText_at:
        c = m_platte.color(QPalette::Active, QPalette::BrightText);
        break;
    case LINGMOPaletteRole::BrightText_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::BrightText);
        break;
    case LINGMOPaletteRole::BrightText_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::BrightText);
        break;
    case LINGMOPaletteRole::ButtonText_at:
        c = m_platte.color(QPalette::Active, QPalette::ButtonText);
        break;
    case LINGMOPaletteRole::ButtonText_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::ButtonText);
        break;
    case LINGMOPaletteRole::ButtonText_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::ButtonText);
        break;
    case LINGMOPaletteRole::Base_at:
        c = m_platte.color(QPalette::Active, QPalette::Base);
        break;
    case LINGMOPaletteRole::Base_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::Base);
        break;
    case LINGMOPaletteRole::Base_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::Base);
        break;
    case LINGMOPaletteRole::Window_at:
        c = m_platte.color(QPalette::Active, QPalette::Window);
        break;
    case LINGMOPaletteRole::Window_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::Window);
        break;
    case LINGMOPaletteRole::Window_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::Window);
        break;
    case LINGMOPaletteRole::Shadow_at:
        c = m_platte.color(QPalette::Active, QPalette::Shadow);
        break;
    case LINGMOPaletteRole::Shadow_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::Shadow);
        break;
    case LINGMOPaletteRole::Shadow_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::Shadow);
        break;
    case LINGMOPaletteRole::HighLight_at:
        c = m_platte.color(QPalette::Active, QPalette::Highlight);
        break;
    case LINGMOPaletteRole::HighLight_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::Highlight);
        break;
    case LINGMOPaletteRole::HighLight_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::Highlight);
        break;
    case LINGMOPaletteRole::HighLightText_at:
        c = m_platte.color(QPalette::Active, QPalette::HighlightedText);
        break;
    case LINGMOPaletteRole::HighLightText_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::HighlightedText);
        break;
    case LINGMOPaletteRole::HighLightText_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::HighlightedText);
        break;
    case LINGMOPaletteRole::Link_at:
        c = m_platte.color(QPalette::Active, QPalette::Link);
        break;
    case LINGMOPaletteRole::Link_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::Link);
        break;
    case LINGMOPaletteRole::Link_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::Link);
        break;
    case LINGMOPaletteRole::LinkVisited_at:
        c = m_platte.color(QPalette::Active, QPalette::LinkVisited);
        break;
    case LINGMOPaletteRole::LinkVisited_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::LinkVisited);
        break;
    case LINGMOPaletteRole::LinkVisited_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::LinkVisited);
        break;
    case LINGMOPaletteRole::AlternateBase_at:
        c = m_platte.color(QPalette::Active, QPalette::AlternateBase);
        break;
    case LINGMOPaletteRole::AlternateBase_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::AlternateBase);
        break;
    case LINGMOPaletteRole::AlternateBase_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::AlternateBase);
        break;
    case LINGMOPaletteRole::NoRole_at:
        c = m_platte.color(QPalette::Active, QPalette::NoRole);
        break;
    case LINGMOPaletteRole::NoRole_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::NoRole);
        break;
    case LINGMOPaletteRole::NoRole_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::NoRole);
        break;
    case LINGMOPaletteRole::ToolTipBase_at:
        c = m_platte.color(QPalette::Active, QPalette::ToolTipBase);
        break;
    case LINGMOPaletteRole::ToolTipBase_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::ToolTipBase);
        break;
    case LINGMOPaletteRole::ToolTipBase_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::ToolTipBase);
        break;
    case LINGMOPaletteRole::ToolTipText_at:
        c = m_platte.color(QPalette::Active, QPalette::ToolTipText);
        break;
    case LINGMOPaletteRole::ToolTipText_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::ToolTipText);
        break;
    case LINGMOPaletteRole::ToolTipText_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::ToolTipText);
        break;
    case LINGMOPaletteRole::PlaceholderText_at:
        c = m_platte.color(QPalette::Active, QPalette::PlaceholderText);
        break;
    case LINGMOPaletteRole::PlaceholderText_iat:
        c = m_platte.color(QPalette::Inactive, QPalette::PlaceholderText);
        break;
    case LINGMOPaletteRole::PlaceholderText_dis:
        c = m_platte.color(QPalette::Disabled, QPalette::PlaceholderText);
        break;

    default:
        break;
    }

    return c;
}

void ReadConfig::stringToGradient(QList<QColor> &cl, QString s) const
{
    QStringList list = s.split("~");

    if(list.length() == 2)
        foreach (QString s, list) {
            cl.append(stringToColor(s));
        }
}

bool ReadConfig::setValue(QString key, QVariant value)
{
    QString cfg = m_cfgPath;
    if (!QFile::exists(cfg)) {
        cWarning << "ReadConfig setValue file not exists!";
        return false;
    }

    QFile file(cfg);
    if(!file.open(QIODevice::ReadWrite)) {
        cWarning << "ReadConfig setValue file Error!";
        return false;
    }

    QByteArray jsonData = file.readAll();
    QJsonParseError error;
    QJsonDocument jdoc = QJsonDocument::fromJson(jsonData, &error);
    if (error.error != QJsonParseError::NoError) {
        cWarning << "QJsonDocument fromJson error!";
        return false;
    }
    QJsonObject obj = jdoc.object();
    if(obj.contains(key)){
     obj[key] = value.toString();
    }
    QJsonDocument jdc;
    jdc.setObject(obj);
    file.write(jdc.toJson());
    file.close();
}

void ReadConfig::widgetPalette(const QPalette platte)
{
    m_platte = platte;
}


ReadThemeConfig::ReadThemeConfig(QString colorCfgPath, bool isDark):
    m_colorCfgPath(colorCfgPath),
    m_themeIsDark(isDark)
{
    if (m_colorCfgPath.isEmpty()) {
        QString path = ColorPath;
        m_colorCfgPath = path + DefaultConfigName;
    }
    m_readConfig = new ReadConfig(m_colorCfgPath);
    m_loadConfig = m_readConfig->load(m_colorCfgPath);
    if(!m_loadConfig){
        m_readConfig->deleteLater();
        m_readConfig = nullptr;
    }
    m_theme = m_themeIsDark ? LINGMODarkTheme : LINGMOLightTheme;

}

ReadThemeConfig::~ReadThemeConfig()
{
    if (m_readConfig) {
        m_readConfig->deleteLater();
        m_readConfig = nullptr;
    }
}

LINGMOColorTheme::PaletteColorCfg ReadThemeConfig::paletteColorCfg()
{
    if(!m_loadConfig)
        return m_paletteColorCfg;

    m_readConfig->getColorValue(m_paletteColorCfg.windowText_at      , m_theme, LINGMOPalette, c_windowText_at);
    m_readConfig->getColorValue(m_paletteColorCfg.windowText_iat     , m_theme, LINGMOPalette, c_windowText_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.windowText_dis     , m_theme, LINGMOPalette, c_windowText_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.button_at          , m_theme, LINGMOPalette, c_button_at);
    m_readConfig->getColorValue(m_paletteColorCfg.button_iat         , m_theme, LINGMOPalette, c_button_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.button_dis         , m_theme, LINGMOPalette, c_button_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.light_at           , m_theme, LINGMOPalette, c_light_at);
    m_readConfig->getColorValue(m_paletteColorCfg.light_iat          , m_theme, LINGMOPalette, c_light_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.light_dis          , m_theme, LINGMOPalette, c_light_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.midlight_at        , m_theme, LINGMOPalette, c_midlight_at);
    m_readConfig->getColorValue(m_paletteColorCfg.midlight_iat       , m_theme, LINGMOPalette, c_midlight_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.midlight_dis       , m_theme, LINGMOPalette, c_midlight_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.dark_at            , m_theme, LINGMOPalette, c_dark_at);
    m_readConfig->getColorValue(m_paletteColorCfg.dark_iat           , m_theme, LINGMOPalette, c_dark_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.dark_dis           , m_theme, LINGMOPalette, c_dark_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.mid_at             , m_theme, LINGMOPalette, c_mid_at);
    m_readConfig->getColorValue(m_paletteColorCfg.mid_iat            , m_theme, LINGMOPalette, c_mid_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.mid_dis            , m_theme, LINGMOPalette, c_mid_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.text_at            , m_theme, LINGMOPalette, c_text_at);
    m_readConfig->getColorValue(m_paletteColorCfg.text_iat           , m_theme, LINGMOPalette, c_text_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.text_dis           , m_theme, LINGMOPalette, c_text_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.brightText_at      , m_theme, LINGMOPalette, c_brightText_at);
    m_readConfig->getColorValue(m_paletteColorCfg.brightText_iat     , m_theme, LINGMOPalette, c_brightText_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.brightText_dis     , m_theme, LINGMOPalette, c_brightText_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.buttonText_at      , m_theme, LINGMOPalette, c_buttonText_at);
    m_readConfig->getColorValue(m_paletteColorCfg.buttonText_iat     , m_theme, LINGMOPalette, c_buttonText_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.buttonText_dis     , m_theme, LINGMOPalette, c_buttonText_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.base_at            , m_theme, LINGMOPalette, c_base_at);
    m_readConfig->getColorValue(m_paletteColorCfg.base_iat           , m_theme, LINGMOPalette, c_base_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.base_dis           , m_theme, LINGMOPalette, c_base_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.window_at          , m_theme, LINGMOPalette, c_window_at);
    m_readConfig->getColorValue(m_paletteColorCfg.window_iat         , m_theme, LINGMOPalette, c_window_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.window_dis         , m_theme, LINGMOPalette, c_window_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.shadow_at          , m_theme, LINGMOPalette, c_shadow_at);
    m_readConfig->getColorValue(m_paletteColorCfg.shadow_iat         , m_theme, LINGMOPalette, c_shadow_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.shadow_dis         , m_theme, LINGMOPalette, c_shadow_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.highLight_at       , m_theme, LINGMOPalette, c_highLight_at);
    m_readConfig->getColorValue(m_paletteColorCfg.highLight_iat      , m_theme, LINGMOPalette, c_highLight_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.highLight_dis      , m_theme, LINGMOPalette, c_highLight_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.highLightText_at   , m_theme, LINGMOPalette, c_highLightText_at);
    m_readConfig->getColorValue(m_paletteColorCfg.highLightText_iat  , m_theme, LINGMOPalette, c_highLightText_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.highLightText_dis  , m_theme, LINGMOPalette, c_highLightText_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.link_at            , m_theme, LINGMOPalette, c_link_at);
    m_readConfig->getColorValue(m_paletteColorCfg.link_iat           , m_theme, LINGMOPalette, c_link_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.link_dis           , m_theme, LINGMOPalette, c_link_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.linkVisited_at     , m_theme, LINGMOPalette, c_linkVisited_at);
    m_readConfig->getColorValue(m_paletteColorCfg.linkVisited_iat    , m_theme, LINGMOPalette, c_linkVisited_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.linkVisited_dis    , m_theme, LINGMOPalette, c_linkVisited_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.alternateBase_at   , m_theme, LINGMOPalette, c_alternateBase_at);
    m_readConfig->getColorValue(m_paletteColorCfg.alternateBase_iat  , m_theme, LINGMOPalette, c_alternateBase_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.alternateBase_dis  , m_theme, LINGMOPalette, c_alternateBase_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.noRole_at          , m_theme, LINGMOPalette, c_noRole_at);
    m_readConfig->getColorValue(m_paletteColorCfg.noRole_iat         , m_theme, LINGMOPalette, c_noRole_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.noRole_dis         , m_theme, LINGMOPalette, c_noRole_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.toolTipBase_at     , m_theme, LINGMOPalette, c_toolTipBase_at);
    m_readConfig->getColorValue(m_paletteColorCfg.toolTipBase_iat    , m_theme, LINGMOPalette, c_toolTipBase_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.toolTipBase_dis    , m_theme, LINGMOPalette, c_toolTipBase_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.toolTipText_at     , m_theme, LINGMOPalette, c_toolTipText_at);
    m_readConfig->getColorValue(m_paletteColorCfg.toolTipText_iat    , m_theme, LINGMOPalette, c_toolTipText_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.toolTipText_dis    , m_theme, LINGMOPalette, c_toolTipText_dis);
    m_readConfig->getColorValue(m_paletteColorCfg.placeholderText_at , m_theme, LINGMOPalette, c_placeholderText_at);
    m_readConfig->getColorValue(m_paletteColorCfg.placeholderText_iat, m_theme, LINGMOPalette, c_placeholderText_iat);
    m_readConfig->getColorValue(m_paletteColorCfg.placeholderText_dis, m_theme, LINGMOPalette, c_placeholderText_dis);
    return m_paletteColorCfg;
}

LINGMOColorTheme::ButtonColorCfg ReadThemeConfig::buttonColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_buttonColorCfg;

    m_readConfig->widgetPalette(palette);
    LINGMOColorTheme::ButtonColorCfg buttonCfg;
    m_readConfig->getColorValue(buttonCfg.defaultBrush, m_theme, LINGMOPushButton, property, C_Button_DefaultBrush);
    m_readConfig->getColorValue(buttonCfg.clickBrush  , m_theme, LINGMOPushButton, property, C_Button_ClickBrush);
    m_readConfig->getColorValue(buttonCfg.disableBrush, m_theme, LINGMOPushButton, property, C_Button_DisableBrush);

    buttonCfg.hoverType = m_readConfig->getType(m_theme, LINGMOPushButton, property, C_Button_HoverBrush);
    if(buttonCfg.hoverType == Obj_Gradient_Type){
        buttonCfg.hoverGradientList.clear();
        m_readConfig->getGradientValue(buttonCfg.hoverGradientList  , m_theme, LINGMOPushButton, property, C_Button_HoverBrush);
    }
    else{
        m_readConfig->getColorValue(buttonCfg.hoverBrush  , m_theme, LINGMOPushButton, property, C_Button_HoverBrush);
    }

    m_readConfig->getColorValue(buttonCfg.defaultPen     , m_theme, LINGMOPushButton, property, C_Button_DefaultPen     );
    m_readConfig->getColorValue(buttonCfg.hoverPen       , m_theme, LINGMOPushButton, property, C_Button_HoverPen       );
    m_readConfig->getColorValue(buttonCfg.clickPen       , m_theme, LINGMOPushButton, property, C_Button_ClickPen       );
    m_readConfig->getColorValue(buttonCfg.disablePen     , m_theme, LINGMOPushButton, property, C_Button_DisablePen     );
    m_readConfig->getColorValue(buttonCfg.focusPen       , m_theme, LINGMOPushButton, property, C_Button_FocusPen       );
    m_readConfig->getColorValue(buttonCfg.textDefaultColor, m_theme, LINGMOPushButton, property, C_Button_TextColor      );
    m_readConfig->getColorValue(buttonCfg.textHoverColor , m_theme, LINGMOPushButton, property, C_Button_TextHoverColor );
    m_readConfig->getColorValue(buttonCfg.textDisableColor , m_theme, LINGMOPushButton, property, C_Button_TextDisableColor );

    m_buttonColorCfg = buttonCfg;
    return m_buttonColorCfg;
}

LINGMOColorTheme::ToolButtonColorCfg ReadThemeConfig::toolButtonColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_toolButtonColorCfg;

    m_readConfig->widgetPalette(palette);
    LINGMOColorTheme::ToolButtonColorCfg tbcfg;
    m_readConfig->getColorValue(tbcfg.defaultBrush, m_theme, LINGMOToolButton, property, C_Button_DefaultBrush);
    m_readConfig->getColorValue(tbcfg.clickBrush  , m_theme, LINGMOToolButton, property, C_Button_ClickBrush);
    m_readConfig->getColorValue(tbcfg.disableBrush, m_theme, LINGMOToolButton, property, C_Button_DisableBrush);

    tbcfg.hoverType = m_readConfig->getType(m_theme, LINGMOToolButton, property, C_Button_HoverBrush);
    if(tbcfg.hoverType == Obj_Gradient_Type){
        tbcfg.hoverGradientList.clear();
        m_readConfig->getGradientValue(tbcfg.hoverGradientList  , m_theme, LINGMOToolButton, property, C_Button_HoverBrush);
    }
    else{
        m_readConfig->getColorValue(tbcfg.hoverBrush  , m_theme, LINGMOToolButton, property, C_Button_HoverBrush);
    }

    m_readConfig->getColorValue(tbcfg.defaultPen     , m_theme, LINGMOToolButton, property, C_Button_DefaultPen     );
    m_readConfig->getColorValue(tbcfg.hoverPen       , m_theme, LINGMOToolButton, property, C_Button_HoverPen       );
    m_readConfig->getColorValue(tbcfg.clickPen       , m_theme, LINGMOToolButton, property, C_Button_ClickPen       );
    m_readConfig->getColorValue(tbcfg.disablePen     , m_theme, LINGMOToolButton, property, C_Button_DisablePen     );
    m_readConfig->getColorValue(tbcfg.focusPen       , m_theme, LINGMOToolButton, property, C_Button_FocusPen       );
    m_readConfig->getColorValue(tbcfg.textDefaultColor, m_theme, LINGMOToolButton, property, C_Button_TextColor      );
    m_readConfig->getColorValue(tbcfg.textHoverColor , m_theme, LINGMOToolButton, property, C_Button_TextHoverColor );
    m_readConfig->getColorValue(tbcfg.textDisableColor , m_theme, LINGMOToolButton, property, C_Button_TextDisableColor );

    m_toolButtonColorCfg = tbcfg;
    return m_toolButtonColorCfg;
}

LINGMOColorTheme::CheckBoxColorCfg ReadThemeConfig::checkBoxColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_checkBoxColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_checkBoxColorCfg.defaultPen      , m_theme, LINGMOCheckBox, property, C_CheckBox_DefaultPen);
    m_readConfig->getColorValue(m_checkBoxColorCfg.hoverPen        , m_theme, LINGMOCheckBox, property, C_CheckBox_HoverPen);
    m_readConfig->getColorValue(m_checkBoxColorCfg.clickPen        , m_theme, LINGMOCheckBox, property, C_CheckBox_ClickPen);
    m_readConfig->getColorValue(m_checkBoxColorCfg.disablePen      , m_theme, LINGMOCheckBox, property, C_CheckBox_DisablePen);
    m_readConfig->getColorValue(m_checkBoxColorCfg.onDefaultPen    , m_theme, LINGMOCheckBox, property, C_CheckBox_OnDefaultPen);
    m_readConfig->getColorValue(m_checkBoxColorCfg.onHoverPen      , m_theme, LINGMOCheckBox, property, C_CheckBox_OnHoverPen);
    m_readConfig->getColorValue(m_checkBoxColorCfg.onClickPen      , m_theme, LINGMOCheckBox, property, C_CheckBox_OnClickPen);
    m_readConfig->getColorValue(m_checkBoxColorCfg.contentPen      , m_theme, LINGMOCheckBox, property, C_CheckBox_ContentPen);

    m_readConfig->getColorValue(m_checkBoxColorCfg.defaultBrush    , m_theme, LINGMOCheckBox, property, C_CheckBox_DefaultBrush);
    m_readConfig->getColorValue(m_checkBoxColorCfg.clickBrush      , m_theme, LINGMOCheckBox, property, C_CheckBox_ClickBrush);
    m_readConfig->getColorValue(m_checkBoxColorCfg.disableBrush    , m_theme, LINGMOCheckBox, property, C_CheckBox_DisableBrush);
    m_readConfig->getColorValue(m_checkBoxColorCfg.onDefaultBrush  , m_theme, LINGMOCheckBox, property, C_CheckBox_OnDefaultBrush);
    m_readConfig->getColorValue(m_checkBoxColorCfg.pathBrush       , m_theme, LINGMOCheckBox, property, C_CheckBox_PathBrush);
    m_readConfig->getColorValue(m_checkBoxColorCfg.pathDisableBrush, m_theme, LINGMOCheckBox, property, C_CheckBox_PathDisableBrush);

    m_checkBoxColorCfg.hoverType   = m_readConfig->getType(m_theme, LINGMOCheckBox, property, C_CheckBox_HoverBrush);
    m_checkBoxColorCfg.onHoverType = m_readConfig->getType(m_theme, LINGMOCheckBox, property, C_CheckBox_OnHoverBrush);
    m_checkBoxColorCfg.onClickType = m_readConfig->getType(m_theme, LINGMOCheckBox, property, C_CheckBox_OnClickBrush);

    if(m_checkBoxColorCfg.hoverType == Obj_Gradient_Type){
        m_checkBoxColorCfg.hoverGradientList.clear();
        m_readConfig->getGradientValue(m_checkBoxColorCfg.hoverGradientList  , m_theme, LINGMOCheckBox, property, C_CheckBox_HoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_checkBoxColorCfg.hoverBrush, m_theme, LINGMOCheckBox, property, C_CheckBox_HoverBrush);
    }
    if(m_checkBoxColorCfg.onHoverType == Obj_Gradient_Type){
        m_checkBoxColorCfg.onHoverGradientList.clear();
        m_readConfig->getGradientValue(m_checkBoxColorCfg.onHoverGradientList  , m_theme, LINGMOCheckBox, property, C_CheckBox_OnHoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_checkBoxColorCfg.onHoverBrush, m_theme, LINGMOCheckBox, property, C_CheckBox_OnHoverBrush);
    }
    if(m_checkBoxColorCfg.onClickType == Obj_Gradient_Type){
        m_checkBoxColorCfg.onClickGradientList.clear();
        m_readConfig->getGradientValue(m_checkBoxColorCfg.onClickGradientList  , m_theme, LINGMOCheckBox, property, C_CheckBox_OnClickBrush);
    }
    else{
        m_readConfig->getColorValue(m_checkBoxColorCfg.onClickBrush, m_theme, LINGMOCheckBox, property, C_CheckBox_OnClickBrush);
    }

    return m_checkBoxColorCfg;
}

LINGMOColorTheme::RadioButtonColorCfg ReadThemeConfig::radioButtonColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_radioButtonColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_radioButtonColorCfg.defaultPen            , m_theme, LINGMORadioButton, property, C_RadioButton_DefaultPen);
    m_readConfig->getColorValue(m_radioButtonColorCfg.hoverPen              , m_theme, LINGMORadioButton, property, C_RadioButton_HoverPen);
    m_readConfig->getColorValue(m_radioButtonColorCfg.clickPen              , m_theme, LINGMORadioButton, property, C_RadioButton_ClickPen);
    m_readConfig->getColorValue(m_radioButtonColorCfg.disablePen            , m_theme, LINGMORadioButton, property, C_RadioButton_DisablePen);
    m_readConfig->getColorValue(m_radioButtonColorCfg.onDefaultPen          , m_theme, LINGMORadioButton, property, C_RadioButton_OnDefaultPen);
    m_readConfig->getColorValue(m_radioButtonColorCfg.onHoverPen            , m_theme, LINGMORadioButton, property, C_RadioButton_OnHoverPen);
    m_readConfig->getColorValue(m_radioButtonColorCfg.onClickPen            , m_theme, LINGMORadioButton, property, C_RadioButton_OnClickPen);

    m_readConfig->getColorValue(m_radioButtonColorCfg.defaultBrush          , m_theme, LINGMORadioButton, property, C_RadioButton_DefaultBrush);
    m_readConfig->getColorValue(m_radioButtonColorCfg.hoverBrush            , m_theme, LINGMORadioButton, property, C_RadioButton_HoverBrush);
    m_readConfig->getColorValue(m_radioButtonColorCfg.clickBrush            , m_theme, LINGMORadioButton, property, C_RadioButton_ClickBrush);
    m_readConfig->getColorValue(m_radioButtonColorCfg.disableBrush          , m_theme, LINGMORadioButton, property, C_RadioButton_DisableBrush);
    m_readConfig->getColorValue(m_radioButtonColorCfg.onDefaultBrush        , m_theme, LINGMORadioButton, property, C_RadioButton_OnDefaultBrush);
    m_readConfig->getColorValue(m_radioButtonColorCfg.onClickBrush          , m_theme, LINGMORadioButton, property, C_RadioButton_OnClickBrush);
    m_readConfig->getColorValue(m_radioButtonColorCfg.childrenOnDefaultBrush, m_theme, LINGMORadioButton, property, C_RadioButton_ChildrenOnDefaultBrush);
    m_readConfig->getColorValue(m_radioButtonColorCfg.childrenOnHoverBrush  , m_theme, LINGMORadioButton, property, C_RadioButton_ChildrenOnHoverBrush);
    m_readConfig->getColorValue(m_radioButtonColorCfg.childrenOnClickBrush  , m_theme, LINGMORadioButton, property, C_RadioButton_ChildrenOnClickBrush);
    m_readConfig->getColorValue(m_radioButtonColorCfg.childrenOnDisableBrush, m_theme, LINGMORadioButton, property, C_RadioButton_ChildrenOnDisableBrush);

    m_radioButtonColorCfg.onHoverType = m_readConfig->getType(m_theme, LINGMORadioButton, property, C_RadioButton_OnHoverBrush);
    if(m_radioButtonColorCfg.onHoverType == Obj_Gradient_Type){
        m_radioButtonColorCfg.onHoverGradientList.clear();
        m_readConfig->getGradientValue(m_radioButtonColorCfg.onHoverGradientList  , m_theme, LINGMORadioButton, property, C_RadioButton_OnHoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_radioButtonColorCfg.onHoverBrush, m_theme, LINGMORadioButton, property, C_RadioButton_OnHoverBrush);
    }

    return m_radioButtonColorCfg;
}

LINGMOColorTheme::LineEditColorCfg ReadThemeConfig::lineEditColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_lineEditColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_lineEditColorCfg.defaultPen  , m_theme, LINGMOLineEdit, property, C_LineEdit_DefaultPen);
    m_readConfig->getColorValue(m_lineEditColorCfg.hoverPen    , m_theme, LINGMOLineEdit, property, C_LineEdit_HoverPen);
    m_readConfig->getColorValue(m_lineEditColorCfg.focusPen    , m_theme, LINGMOLineEdit, property, C_LineEdit_FocusPen);
    m_readConfig->getColorValue(m_lineEditColorCfg.disablePen  , m_theme, LINGMOLineEdit, property, C_LineEdit_DisablePen);

    m_readConfig->getColorValue(m_lineEditColorCfg.defaultBrush, m_theme, LINGMOLineEdit, property, C_LineEdit_DefaultBrush);
    m_readConfig->getColorValue(m_lineEditColorCfg.focusBrush  , m_theme, LINGMOLineEdit, property, C_LineEdit_FocusBrush);
    m_readConfig->getColorValue(m_lineEditColorCfg.disableBrush, m_theme, LINGMOLineEdit, property, C_LineEdit_DisableBrush);

    m_lineEditColorCfg.hoverType = m_readConfig->getType(m_theme, LINGMOLineEdit, property, C_LineEdit_HoverBrush);
    if(m_lineEditColorCfg.hoverType == Obj_Gradient_Type){
        m_lineEditColorCfg.hoverGradientList.clear();
        m_readConfig->getGradientValue(m_lineEditColorCfg.hoverGradientList  , m_theme, LINGMOLineEdit, property, C_LineEdit_HoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_lineEditColorCfg.hoverBrush, m_theme, LINGMOLineEdit, property, C_LineEdit_HoverBrush);
    }

    return m_lineEditColorCfg;
}

LINGMOColorTheme::ComboBoxColorCfg ReadThemeConfig::comboBoxColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_comboBoxColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_comboBoxColorCfg.defaultPen  , m_theme, LINGMOComboBox, property, C_ComboBox_DefaultPen);
    m_readConfig->getColorValue(m_comboBoxColorCfg.hoverPen    , m_theme, LINGMOComboBox, property, C_ComboBox_HoverPen);
    m_readConfig->getColorValue(m_comboBoxColorCfg.onPen       , m_theme, LINGMOComboBox, property, C_ComboBox_OnPen);
    m_readConfig->getColorValue(m_comboBoxColorCfg.editPen     , m_theme, LINGMOComboBox, property, C_ComboBox_EditPen);
    m_readConfig->getColorValue(m_comboBoxColorCfg.disablePen  , m_theme, LINGMOComboBox, property, C_ComboBox_DisablePen);
    m_readConfig->getColorValue(m_comboBoxColorCfg.focusPen    , m_theme, LINGMOComboBox, property, C_ComboBox_FocusPen);

    m_readConfig->getColorValue(m_comboBoxColorCfg.defaultBrush, m_theme, LINGMOComboBox, property, C_ComboBox_DefaultBrush);
    m_readConfig->getColorValue(m_comboBoxColorCfg.hoverBrush  , m_theme, LINGMOComboBox, property, C_ComboBox_HoverBrush);
    m_readConfig->getColorValue(m_comboBoxColorCfg.onBrush     , m_theme, LINGMOComboBox, property, C_ComboBox_OnBrush);
    m_readConfig->getColorValue(m_comboBoxColorCfg.editBrush   , m_theme, LINGMOComboBox, property, C_ComboBox_EditBrush);
    m_readConfig->getColorValue(m_comboBoxColorCfg.disableBrush, m_theme, LINGMOComboBox, property, C_ComboBox_DisableBrush);
    return m_comboBoxColorCfg;
}

LINGMOColorTheme::SpinBoxColorCfg ReadThemeConfig::spinBoxColorCfg(const QPalette palette, QString property, QString layout)
{
    if(!m_loadConfig)
        return m_spinBoxColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_spinBoxColorCfg.defaultPen         , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DefaultPen);
    m_readConfig->getColorValue(m_spinBoxColorCfg.hoverPen           , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_HoverPen);
    m_readConfig->getColorValue(m_spinBoxColorCfg.focusPen           , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_FocusPen);
    m_readConfig->getColorValue(m_spinBoxColorCfg.disablePen         , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DisablePen);

    m_readConfig->getColorValue(m_spinBoxColorCfg.defaultBrush       , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DefaultBrush);
    m_readConfig->getColorValue(m_spinBoxColorCfg.focusBrush           , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_FocusBrush);
    m_readConfig->getColorValue(m_spinBoxColorCfg.disableBrush         , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DisableBrush);
    m_readConfig->getColorValue(m_spinBoxColorCfg.upClickBrush           , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpClickBrush       );
    m_readConfig->getColorValue(m_spinBoxColorCfg.upDefaultBrush         , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpDefaultBrush     );
    m_readConfig->getColorValue(m_spinBoxColorCfg.downClickBrush     , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownClickBrush     );
    m_readConfig->getColorValue(m_spinBoxColorCfg.downDefaultBrush   , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownDefaultBrush   );

    m_spinBoxColorCfg.hoverType               = m_readConfig->getType(m_theme, LINGMOSpinBox, property, layout, C_SpinBox_HoverBrush);
    m_spinBoxColorCfg.upHoverBrushType        = m_readConfig->getType(m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpHoverBrush);
    m_spinBoxColorCfg.upFocusHoverBrushType   = m_readConfig->getType(m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpFocusHoverBrush);
    m_spinBoxColorCfg.downHoverBrushType      = m_readConfig->getType(m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownHoverBrush);
    m_spinBoxColorCfg.downFocusHoverBrushType = m_readConfig->getType(m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownFocusHoverBrush);

    if(m_spinBoxColorCfg.hoverType == Obj_Gradient_Type){
        m_spinBoxColorCfg.hoverGradientList.clear();
        m_readConfig->getGradientValue(m_spinBoxColorCfg.hoverGradientList  , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_HoverBrush);
    }
    else {
        m_readConfig->getColorValue(m_spinBoxColorCfg.hoverBrush, m_theme, LINGMOSpinBox, property, layout, C_SpinBox_HoverBrush);
    }

    if(m_spinBoxColorCfg.upHoverBrushType == Obj_Gradient_Type){
        m_spinBoxColorCfg.upHoverBrushGradientList.clear();
        m_readConfig->getGradientValue(m_spinBoxColorCfg.upHoverBrushGradientList  , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpHoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_spinBoxColorCfg.upHoverBrush, m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpHoverBrush);
    }

    if(m_spinBoxColorCfg.upFocusHoverBrushType == Obj_Gradient_Type){
        m_spinBoxColorCfg.upFocusHoverBrushGradientList.clear();
        m_readConfig->getGradientValue(m_spinBoxColorCfg.upFocusHoverBrushGradientList  , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpFocusHoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_spinBoxColorCfg.upFocusHoverBrush, m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpFocusHoverBrush);
    }

    if(m_spinBoxColorCfg.downHoverBrushType == Obj_Gradient_Type){
        m_spinBoxColorCfg.downHoverBrushGradientList.clear();
        m_readConfig->getGradientValue(m_spinBoxColorCfg.downHoverBrushGradientList  , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownHoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_spinBoxColorCfg.downHoverBrush, m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownHoverBrush);
    }

    if(m_spinBoxColorCfg.downFocusHoverBrushType == Obj_Gradient_Type){
        m_spinBoxColorCfg.downFocusHoverBrushGradientList.clear();
        m_readConfig->getGradientValue(m_spinBoxColorCfg.downFocusHoverBrushGradientList  , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownFocusHoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_spinBoxColorCfg.downFocusHoverBrush, m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownFocusHoverBrush);
    }

    m_readConfig->getColorValue(m_spinBoxColorCfg.upDefaultPen      , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpDefaultPen );
    m_readConfig->getColorValue(m_spinBoxColorCfg.upHoverPen        , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpHoverPen   );
    m_readConfig->getColorValue(m_spinBoxColorCfg.upDisablePen      , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpDisablePen );
    m_readConfig->getColorValue(m_spinBoxColorCfg.upClickPen        , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpClickPen   );
    m_readConfig->getColorValue(m_spinBoxColorCfg.upFocusPen        , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_UpFocusPen   );

    m_readConfig->getColorValue(m_spinBoxColorCfg.downDefaultPen    , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownDefaultPen );
    m_readConfig->getColorValue(m_spinBoxColorCfg.downHoverPen      , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownHoverPen   );
    m_readConfig->getColorValue(m_spinBoxColorCfg.downDisablePen    , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownDisablePen );
    m_readConfig->getColorValue(m_spinBoxColorCfg.downClickPen      , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownClickPen   );
    m_readConfig->getColorValue(m_spinBoxColorCfg.downFocusPen      , m_theme, LINGMOSpinBox, property, layout, C_SpinBox_DownFocusPen   );

    return m_spinBoxColorCfg;
}

LINGMOColorTheme::TableColorCfg ReadThemeConfig::tableColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_tableColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_tableColorCfg.defaultBrush  ,m_theme, LINGMOTable, property, C_Table_DefaultBrush);
    m_readConfig->getColorValue(m_tableColorCfg.selectBrush   ,m_theme, LINGMOTable, property, C_Table_SelectBrush);
    m_readConfig->getColorValue(m_tableColorCfg.disableBrush  ,m_theme, LINGMOTable, property, C_Table_DisableBrush);

    m_readConfig->getColorValue(m_tableColorCfg.hoverPen      ,m_theme, LINGMOTable, property, C_Table_HoverPen);
    m_readConfig->getColorValue(m_tableColorCfg.selectPen     ,m_theme, LINGMOTable, property, C_Table_SelectPen);
    m_readConfig->getColorValue(m_tableColorCfg.textHoverPen  ,m_theme, LINGMOTable, property, C_Table_TextHoverPen);
    m_readConfig->getColorValue(m_tableColorCfg.textSelectPen ,m_theme, LINGMOTable, property, C_Table_TextSelectPen);
    m_readConfig->getColorValue(m_tableColorCfg.textDisablePen,m_theme, LINGMOTable, property, C_Table_TextDisablePen);
    m_readConfig->getColorValue(m_tableColorCfg.focusPen      ,m_theme, LINGMOTable, property, C_Table_FocusPen);

    m_tableColorCfg.hoverType          = m_readConfig->getType(m_theme, LINGMOTable, property, C_Table_HoverBrush);
    if(m_tableColorCfg.hoverType == Obj_Gradient_Type){
        m_tableColorCfg.hoverGradientList.clear();
        m_readConfig->getGradientValue(m_tableColorCfg.hoverGradientList  , m_theme, LINGMOTable, property, C_Table_HoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_tableColorCfg.hoverBrush, m_theme, LINGMOTable, property, C_Table_HoverBrush);
    }
    return m_tableColorCfg;
}

LINGMOColorTheme::ListViewColorCfg ReadThemeConfig::listViewColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_listViewColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_listViewColorCfg.defaultBrush , m_theme, LINGMOListView, property, C_ListView_DefaultBrush);
    m_readConfig->getColorValue(m_listViewColorCfg.selectBrush  , m_theme, LINGMOListView, property, C_ListView_SelectBrush);
    m_readConfig->getColorValue(m_listViewColorCfg.disableBrush , m_theme, LINGMOListView, property, C_ListView_DisableBrush);

    m_readConfig->getColorValue(m_listViewColorCfg.textHoverPen , m_theme, LINGMOListView, property, C_ListView_TextHoverPen);
    m_readConfig->getColorValue(m_listViewColorCfg.textSelectPen, m_theme, LINGMOListView, property, C_ListView_TextSelectPen);
    m_readConfig->getColorValue(m_listViewColorCfg.textDisablePen,m_theme, LINGMOListView, property, C_ListView_TextDisablePen);

    m_readConfig->getColorValue(m_listViewColorCfg.focusPen     , m_theme, LINGMOListView, property, C_ListView_FocusPen);

    m_listViewColorCfg.hoverType          = m_readConfig->getType(m_theme, LINGMOListView, property, C_ListView_HoverBrush);
    if(m_listViewColorCfg.hoverType == Obj_Gradient_Type){
        m_listViewColorCfg.hoverGradientList.clear();
        m_readConfig->getGradientValue(m_listViewColorCfg.hoverGradientList  , m_theme, LINGMOListView, property, C_ListView_HoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_listViewColorCfg.hoverBrush, m_theme, LINGMOListView, property, C_ListView_HoverBrush);
    }

    m_readConfig->getColorValue(m_listViewColorCfg.hoverPen , m_theme, LINGMOListView, property, C_ListView_HoverPen);
    m_readConfig->getColorValue(m_listViewColorCfg.selectPen, m_theme, LINGMOListView, property, C_ListView_SelectPen);

    return m_listViewColorCfg;
}

LINGMOColorTheme::TreeViewColorCfg ReadThemeConfig::treeViewColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_treeViewColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_treeViewColorCfg.defaultBrush      , m_theme, LINGMOTreeView, property, C_TreeView_DefaultBrush);
    m_readConfig->getColorValue(m_treeViewColorCfg.selectBrush       , m_theme, LINGMOTreeView, property, C_TreeView_SelectBrush);
    m_readConfig->getColorValue(m_treeViewColorCfg.disableBrush      , m_theme, LINGMOTreeView, property, C_TreeView_DisableBrush);
    m_readConfig->getColorValue(m_treeViewColorCfg.branchDefaultBrush, m_theme, LINGMOTreeView, property, C_TreeView_BranchDefaultBrush);
    m_readConfig->getColorValue(m_treeViewColorCfg.branchSelectBrush , m_theme, LINGMOTreeView, property, C_TreeView_BranchSelectBrush);
    m_readConfig->getColorValue(m_treeViewColorCfg.branchDisableBrush, m_theme, LINGMOTreeView, property, C_TreeView_BranchDisableBrush);
    m_readConfig->getColorValue(m_treeViewColorCfg.focusPen          , m_theme, LINGMOTreeView, property, C_TreeView_FocusPen);

    m_readConfig->getColorValue(m_treeViewColorCfg.textHoverPen      , m_theme, LINGMOTreeView, property, C_TreeView_TextHoverPen);
    m_readConfig->getColorValue(m_treeViewColorCfg.textSelectPen     , m_theme, LINGMOTreeView, property, C_TreeView_TextSelectPen);
    m_readConfig->getColorValue(m_treeViewColorCfg.textDisablePen    , m_theme, LINGMOTreeView, property, C_TreeView_TextDisablePen);


    m_treeViewColorCfg.hoverType          = m_readConfig->getType(m_theme, LINGMOTreeView, property, C_TreeView_HoverBrush);
    if(m_treeViewColorCfg.hoverType == Obj_Gradient_Type){
        m_treeViewColorCfg.hoverGradientList.clear();
        m_readConfig->getGradientValue(m_treeViewColorCfg.hoverGradientList  , m_theme, LINGMOTreeView, property, C_TreeView_HoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_treeViewColorCfg.hoverBrush, m_theme, LINGMOTreeView, property, C_TreeView_HoverBrush);
    }
    m_treeViewColorCfg.branchHoverType          = m_readConfig->getType(m_theme, LINGMOTreeView, property, C_TreeView_BranchHoverBrush);
    if(m_treeViewColorCfg.branchHoverType == Obj_Gradient_Type){
        m_treeViewColorCfg.branchHoverGradientList.clear();
        m_readConfig->getGradientValue(m_treeViewColorCfg.branchHoverGradientList  , m_theme, LINGMOTreeView, property, C_TreeView_BranchHoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_treeViewColorCfg.branchHoverBrush, m_theme, LINGMOTreeView, property, C_TreeView_BranchHoverBrush);
    }
    m_readConfig->getColorValue(m_treeViewColorCfg.hoverPen , m_theme, LINGMOTreeView, property, C_TreeView_HoverPen);
    m_readConfig->getColorValue(m_treeViewColorCfg.selectPen, m_theme, LINGMOTreeView, property, C_TreeView_SelectPen);
    m_readConfig->getColorValue(m_treeViewColorCfg.branchhoverPen , m_theme, LINGMOTreeView, property, C_TreeView_BranchHoverPen);
    m_readConfig->getColorValue(m_treeViewColorCfg.branchselectPen, m_theme, LINGMOTreeView, property, C_TreeView_BranchSelectPen);

    return m_treeViewColorCfg;
}

LINGMOColorTheme::SliderBarColorCfg ReadThemeConfig::sliderBarColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_sliderBarColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_sliderBarColorCfg.handleDefaultBrush       , m_theme, LINGMOSliderBar, property, C_SliderBar_HandleDefaultBrush       );
    m_readConfig->getColorValue(m_sliderBarColorCfg.handleClickBrush         , m_theme, LINGMOSliderBar, property, C_SliderBar_HandleClickBrush         );
    m_readConfig->getColorValue(m_sliderBarColorCfg.handleDisableBrush       , m_theme, LINGMOSliderBar, property, C_SliderBar_HandleDisableBrush       );
    m_readConfig->getColorValue(m_sliderBarColorCfg.grooveValueDefaultBrush  , m_theme, LINGMOSliderBar, property, C_SliderBar_GrooveValueDefaultBrush  );
    m_readConfig->getColorValue(m_sliderBarColorCfg.grooveValueHoverBrush    , m_theme, LINGMOSliderBar, property, C_SliderBar_GrooveValueHoverBrush    );
    m_readConfig->getColorValue(m_sliderBarColorCfg.grooveValueDisableBrush  , m_theme, LINGMOSliderBar, property, C_SliderBar_GrooveValueDisableBrush  );
    m_readConfig->getColorValue(m_sliderBarColorCfg.grooveUnvalueDefaultBrush, m_theme, LINGMOSliderBar, property, C_SliderBar_GrooveUnvalueDefaultBrush);
    m_readConfig->getColorValue(m_sliderBarColorCfg.grooveUnvalueHoverBrush  , m_theme, LINGMOSliderBar, property, C_SliderBar_GrooveUnvalueHoverBrush  );
    m_readConfig->getColorValue(m_sliderBarColorCfg.grooveUnvalueDisableBrush, m_theme, LINGMOSliderBar, property, C_SliderBar_GrooveUnvalueDisableBrush);
    m_readConfig->getColorValue(m_sliderBarColorCfg.handleDefaultPen, m_theme, LINGMOSliderBar, property, C_SliderBar_HandleDefaultPen);
    m_readConfig->getColorValue(m_sliderBarColorCfg.handleHoverPen  , m_theme, LINGMOSliderBar, property, C_SliderBar_HandleHoverPen  );
    m_readConfig->getColorValue(m_sliderBarColorCfg.handleClickPen  , m_theme, LINGMOSliderBar, property, C_SliderBar_HandleClickPen  );
    m_readConfig->getColorValue(m_sliderBarColorCfg.handleDisablePen, m_theme, LINGMOSliderBar, property, C_SliderBar_HandleDisablePen);
    m_readConfig->getColorValue(m_sliderBarColorCfg.sliderGroovePen , m_theme, LINGMOSliderBar, property, C_SliderBar_SliderGroovePen );
    m_readConfig->getColorValue(m_sliderBarColorCfg.focusPen        , m_theme, LINGMOSliderBar, property, C_SliderBar_FocusPen);


    m_sliderBarColorCfg.handleHoverType          = m_readConfig->getType(m_theme, LINGMOSliderBar, property, C_SliderBar_HandleHoverBrush);
    if(m_sliderBarColorCfg.handleHoverType == Obj_Gradient_Type){
        m_sliderBarColorCfg.handleHoverGradientList.clear();
        m_readConfig->getGradientValue(m_sliderBarColorCfg.handleHoverGradientList  , m_theme, LINGMOSliderBar, property, C_SliderBar_HandleHoverBrush);
    }
    else {
        m_readConfig->getColorValue(m_sliderBarColorCfg.handleHoverBrush, m_theme, LINGMOSliderBar, property, C_SliderBar_HandleHoverBrush);
    }

    return m_sliderBarColorCfg;
}

LINGMOColorTheme::ProgressBarColorCfg ReadThemeConfig::progressBarColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_progressBarColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_progressBarColorCfg.contentStartColor , m_theme, LINGMOProgressBar, property, C_ProgressBar_ContentStartColor );
    m_readConfig->getColorValue(m_progressBarColorCfg.contentEndColor   , m_theme, LINGMOProgressBar, property, C_ProgressBar_ContentEndColor   );
    m_readConfig->getColorValue(m_progressBarColorCfg.contentBrush      , m_theme, LINGMOProgressBar, property, C_ProgressBar_ContentBrush      );
    m_readConfig->getColorValue(m_progressBarColorCfg.grooveDefaultBrush, m_theme, LINGMOProgressBar, property, C_ProgressBar_GrooveDefaultBrush);
    m_readConfig->getColorValue(m_progressBarColorCfg.grooveDisableBrush, m_theme, LINGMOProgressBar, property, C_ProgressBar_GrooveDisableBrush);

    m_readConfig->getColorValue(m_progressBarColorCfg.contentPen      , m_theme, LINGMOProgressBar, property, C_ProgressBar_ContentPen      );
    m_readConfig->getColorValue(m_progressBarColorCfg.grooveDefaultPen, m_theme, LINGMOProgressBar, property, C_ProgressBar_GrooveDefaultPen);
    m_readConfig->getColorValue(m_progressBarColorCfg.grooveDisablePen, m_theme, LINGMOProgressBar, property, C_ProgressBar_GrooveDisablePen);

    return m_progressBarColorCfg;
}

LINGMOColorTheme::ToolTipColorCfg ReadThemeConfig::toolTipColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_toolTipColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_toolTipColorCfg.backgroundBrush, m_theme, LINGMOToolTip, property, C_ToolTip_BackgroundBrush);
    m_readConfig->getColorValue(m_toolTipColorCfg.backgroundPen, m_theme, LINGMOToolTip, property, C_ToolTip_BackgroundPen);
    return m_toolTipColorCfg;
}

LINGMOColorTheme::ScrollBarColorCfg ReadThemeConfig::scrollBarColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_scrollBarColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_scrollBarColorCfg.grooveDefaultBrush , m_theme, LINGMOScrollBar, property, C_ScrollBar_GrooveDefaultBrush   );
    m_readConfig->getColorValue(m_scrollBarColorCfg.grooveInactiveBrush, m_theme, LINGMOScrollBar, property, C_ScrollBar_GrooveInactiveBrush  );
    m_readConfig->getColorValue(m_scrollBarColorCfg.sliderDefaultBrush , m_theme, LINGMOScrollBar, property, C_ScrollBar_SliderDefaultBrush   );
    m_readConfig->getColorValue(m_scrollBarColorCfg.sliderClickBrush   , m_theme, LINGMOScrollBar, property, C_ScrollBar_SliderClickBrush     );
    m_readConfig->getColorValue(m_scrollBarColorCfg.sliderDisableBrush , m_theme, LINGMOScrollBar, property, C_ScrollBar_SliderDisableBrush   );

    m_scrollBarColorCfg.sliderHoverType          = m_readConfig->getType(m_theme, LINGMOScrollBar, property, C_ScrollBar_SliderHoverBrush);
    if(m_scrollBarColorCfg.sliderHoverType == Obj_Gradient_Type){
        m_scrollBarColorCfg.sliderHoverGradientList.clear();
        m_readConfig->getGradientValue(m_scrollBarColorCfg.sliderHoverGradientList  , m_theme, LINGMOScrollBar, property, C_ScrollBar_SliderHoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_scrollBarColorCfg.sliderHoverBrush, m_theme, LINGMOScrollBar, property, C_ScrollBar_SliderHoverBrush);
    }
    return m_scrollBarColorCfg;
}

LINGMOColorTheme::TabWidgetColorCfg ReadThemeConfig::tabWidgetColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_tabWidgetColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_tabWidgetColorCfg.widgetBackgroundBrush, m_theme, LINGMOTabWidget, property, C_TabWidget_WidgetBackgroundBrush);
    m_readConfig->getColorValue(m_tabWidgetColorCfg.tabBarBackgroundBrush, m_theme, LINGMOTabWidget, property, C_TabWidget_TabBarBackgroundBrush);
    m_readConfig->getColorValue(m_tabWidgetColorCfg.tabBarDefaultBrush   , m_theme, LINGMOTabWidget, property, C_TabWidget_TabBarDefaultBrush   );
    m_readConfig->getColorValue(m_tabWidgetColorCfg.tabBarSelectBrush    , m_theme, LINGMOTabWidget, property, C_TabWidget_TabBarSelectBrush    );
    m_readConfig->getColorValue(m_tabWidgetColorCfg.tabBarClickBrush     , m_theme, LINGMOTabWidget, property, C_TabWidget_TabBarClickBrush     );
    m_readConfig->getColorValue(m_tabWidgetColorCfg.tabBarHoverPen       , m_theme, LINGMOTabWidget, property, C_TabWidget_TabBarHoverPen       );
    m_readConfig->getColorValue(m_tabWidgetColorCfg.tabBarClickPen       , m_theme, LINGMOTabWidget, property, C_TabWidget_TabBarClickPen       );
    m_readConfig->getColorValue(m_tabWidgetColorCfg.tabBarFocusPen       , m_theme, LINGMOTabWidget, property, C_TabWidget_TabBarFocusPen       );

    m_tabWidgetColorCfg.tabBarHoverType          = m_readConfig->getType(m_theme, LINGMOTabWidget, property, C_TabWidget_TabBarHoverBrush);
    if(m_tabWidgetColorCfg.tabBarHoverType == Obj_Gradient_Type){
        m_tabWidgetColorCfg.tabBarHoverGradientList.clear();
        m_readConfig->getGradientValue(m_tabWidgetColorCfg.tabBarHoverGradientList  , m_theme, LINGMOTabWidget, property, C_TabWidget_TabBarHoverBrush);
    }
    else{
        m_readConfig->getColorValue(m_tabWidgetColorCfg.tabBarHoverBrush, m_theme, LINGMOTabWidget, property, C_TabWidget_TabBarHoverBrush);
    }

    return m_tabWidgetColorCfg;
}

LINGMOColorTheme::MenuColorCfg ReadThemeConfig::menuColorCfg(const QPalette palette, QString property)
{
    if(!m_loadConfig)
        return m_menuColorCfg;

    m_readConfig->widgetPalette(palette);
    m_readConfig->getColorValue(m_menuColorCfg.menuBackgroundBrush, m_theme, LINGMOMenu, property, C_Menu_MenuBackgroundBrush);
    m_readConfig->getColorValue(m_menuColorCfg.menuBackgroundPen, m_theme, LINGMOMenu, property, C_Menu_MenuBackgroundPen);
    m_readConfig->getColorValue(m_menuColorCfg.menuItemSelectBrush, m_theme, LINGMOMenu, property, C_Menu_MenuItemSelectBrush);
    m_readConfig->getColorValue(m_menuColorCfg.menuItemFocusPen   , m_theme, LINGMOMenu, property, C_Menu_MenuItemFocusPen );
    m_readConfig->getColorValue(m_menuColorCfg.menuTextHoverPen   , m_theme, LINGMOMenu, property, C_Menu_MenuTextHoverPen   );

    return m_menuColorCfg;
}


bool ReadThemeConfig::setRadius(QString key, int r)
{
    if(!m_loadConfig)
        return false;
    m_readConfig->setValue(key, r);
}

void ReadThemeConfig::getRadius(QString key, int &r)
{
    if(!m_loadConfig)
        return;
    QJsonObject obj = m_readConfig->getValue(LINGMO_Radius, key).toJsonObject();
    if (obj.isEmpty()) {
        cWarning << "getRadius isempty!" << key;
        return;
    }
    if (obj.value(Obj_Type) != Obj_Int_Type) {
        cWarning << "get radius value type error!" << obj.value(Obj_Type).toString() << key;
        return;
    }
    if(obj.value(Obj_Value).toString().isEmpty()){
        cWarning << "obj" << Obj_Value << "key isempty!";
        return;
    }
    r = QString(obj.value(Obj_Value).toString()).toInt();
    return;
}

LINGMORadiusInformation::LINGMORadiusStruct ReadThemeConfig::getRadiusStruct(QString key)
{
    if(!m_loadConfig)
        return m_radiusStruct;

    getRadius(Max_Radius, m_radiusStruct.maxRadius);
    getRadius(Normal_Radius, m_radiusStruct.normalRadius);
    getRadius(Min_Radius, m_radiusStruct.minRadius);
    return m_radiusStruct;
}

QString ReadThemeConfig::getColorValueAdjustRules()
{
    if(!m_loadConfig)
        return "";
    if(m_readConfig->getValue(ColorAdjustRules).toString().isEmpty())
        return "";
    return m_readConfig->getValue(ColorAdjustRules).toString();
}


/*
RadiusConfig::RadiusConfig(QString cfgPath):
    m_cfgPath(cfgPath)
{
    if (m_cfgPath.isEmpty()) {
        QString path = UsrRadiusPath;
        m_cfgPath = path + DefaultConfigName;
    }
    m_readConfig = new ReadConfig(m_cfgPath);
    m_loadConfig = m_readConfig->load(m_cfgPath);
    cDebug << "m_cfgPath:" << m_cfgPath << "m_loadConfig:" << m_loadConfig;

    if(!m_loadConfig){
        m_readConfig->deleteLater();
        m_readConfig = nullptr;
    }
}

RadiusConfig::~RadiusConfig()
{

}

bool RadiusConfig::setRadius(QString key, int r)
{
    if(!m_loadConfig)
        return false;
    m_readConfig->setValue(key, r);
}

void RadiusConfig::getRadius(QString key, int &r)
{
    if(!m_loadConfig)
        return;
    QJsonObject obj = m_readConfig->getValue(LINGMO_Radius, key).toJsonObject();
    if (obj.isEmpty()) {
        cDebug << "getRadius isempty!" << key;
        return;
    }
    if (obj.value(Obj_Type) != Obj_Int_Type) {
        cDebug << "get radius value type error!" << obj.value(Obj_Type).toString() << key;
        return;
    }
    if(obj.value(Obj_Value).toString().isEmpty()){
        cDebug << "obj" << Obj_Value << "key isempty!";
        return;
    }
    r = QString(obj.value(Obj_Value).toString()).toInt();
    return;
}
*/
