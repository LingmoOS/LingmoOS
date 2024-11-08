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

#ifndef READCONFIG_H
#define READCONFIG_H

#include <QObject>
#include <QSettings>
#include <QColor>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPalette>
#include "themeinformation.h"


class ReadConfig : public QObject
{
    Q_OBJECT

public:
    ReadConfig(const QString &cfg);
    ~ReadConfig();

    bool load(const QString &cfg);

    QVariant getValue(const QString key_1, const QString key_2 = "", const QString key_3 = "", const QString key_4 = "", const QString key_5 = "") const;

    QVariant getValue(const QString key, QJsonObject obj) const;

    QJsonObject getJsonObjectValue(const QString key, QJsonObject obj) const;

    bool getColorValue(QColor &color, const QString key_1, const QString key_2 = "", const QString key_3 = "", const QString key_4 = "", const QString key_5 = "") const;

    bool getColorValue(QBrush &brush, const QString key_1, const QString key_2 = "", const QString key_3 = "", const QString key_4 = "", const QString key_5 = "") const;

    bool getGradientValue(QList<QColor> &cl, const QString key_1, const QString key_2 = "", const QString key_3 = "", const QString key_4 = "", const QString key_5 = "") const;

    QString getType(const QString key_1, const QString key_2 = "", const QString key_3 = "", const QString key_4 = "", const QString key_5 = "") const;

    QColor stringToColor(QString s) const;

    QColor stringToPaletteColor(QString s) const;

    void stringToGradient(QList<QColor> &cl, QString s) const;

    bool setValue(QString key, QVariant value);

    void widgetPalette(const QPalette platte);
private:
    QJsonObject m_cfgJsonObj;
    QString m_cfgPath;
    QPalette m_platte;
};

class ReadThemeConfig : public QObject
{
    Q_OBJECT
public:
    ReadThemeConfig(QString colorCfgPath, bool isDark = true);
    ~ReadThemeConfig();

    LINGMOColorTheme::PaletteColorCfg paletteColorCfg();

    LINGMOColorTheme::ButtonColorCfg buttonColorCfg(const QPalette palette, QString property = C_Button_DefaultPushButton);

    LINGMOColorTheme::ToolButtonColorCfg toolButtonColorCfg(const QPalette palette, QString property = C_Button_DefaultToolButton);

    LINGMOColorTheme::CheckBoxColorCfg checkBoxColorCfg(const QPalette palette, QString property = C_CheckBox_Default);

    LINGMOColorTheme::RadioButtonColorCfg radioButtonColorCfg(const QPalette palette, QString property = C_RadioButton_Default);

    LINGMOColorTheme::LineEditColorCfg lineEditColorCfg(const QPalette palette, QString property = C_LineEdit_Default);

    LINGMOColorTheme::ComboBoxColorCfg comboBoxColorCfg(const QPalette palette, QString property = C_ComboBox_Default);

    LINGMOColorTheme::SpinBoxColorCfg spinBoxColorCfg(const QPalette palette, QString property = C_SpinBox_Default, QString layout = C_SpinBox_DefaultLayout);

    LINGMOColorTheme::TableColorCfg tableColorCfg(const QPalette palette, QString property = C_Table_Default);

    LINGMOColorTheme::ListViewColorCfg listViewColorCfg(const QPalette palette, QString property = C_ListView_Default);

    LINGMOColorTheme::TreeViewColorCfg treeViewColorCfg(const QPalette palette, QString property = C_TreeView_Default);

    LINGMOColorTheme::SliderBarColorCfg sliderBarColorCfg(const QPalette palette, QString property = C_SliderBar_Default);

    LINGMOColorTheme::ProgressBarColorCfg progressBarColorCfg(const QPalette palette, QString property = C_ProgressBar_Default);

    LINGMOColorTheme::ToolTipColorCfg toolTipColorCfg(const QPalette palette, QString property = C_ToolTip_Default);

    LINGMOColorTheme::ScrollBarColorCfg scrollBarColorCfg(const QPalette palette, QString property = C_ScrollBar_Default);

    LINGMOColorTheme::TabWidgetColorCfg tabWidgetColorCfg(const QPalette palette, QString property = C_TabWidget_Default);

    LINGMOColorTheme::MenuColorCfg menuColorCfg(const QPalette palette, QString property = C_Menu_Default);

    bool setRadius(QString key, int r);

    void getRadius(QString key, int &r);

    LINGMORadiusInformation::LINGMORadiusStruct getRadiusStruct(QString key);

    QString getColorValueAdjustRules();

private:
    QString m_colorCfgPath;
    bool m_themeIsDark = false;
    LINGMOColorTheme::PaletteColorCfg         m_paletteColorCfg     ;
    LINGMOColorTheme::ButtonColorCfg          m_buttonColorCfg      ;
    LINGMOColorTheme::ToolButtonColorCfg      m_toolButtonColorCfg  ;
    LINGMOColorTheme::CheckBoxColorCfg        m_checkBoxColorCfg    ;
    LINGMOColorTheme::RadioButtonColorCfg     m_radioButtonColorCfg ;
    LINGMOColorTheme::LineEditColorCfg        m_lineEditColorCfg    ;
    LINGMOColorTheme::ComboBoxColorCfg        m_comboBoxColorCfg    ;
    LINGMOColorTheme::SpinBoxColorCfg         m_spinBoxColorCfg     ;
    LINGMOColorTheme::TableColorCfg           m_tableColorCfg       ;
    LINGMOColorTheme::ListViewColorCfg        m_listViewColorCfg    ;
    LINGMOColorTheme::TreeViewColorCfg        m_treeViewColorCfg    ;
    LINGMOColorTheme::SliderBarColorCfg       m_sliderBarColorCfg   ;
    LINGMOColorTheme::ProgressBarColorCfg     m_progressBarColorCfg ;
    LINGMOColorTheme::ToolTipColorCfg         m_toolTipColorCfg     ;
    LINGMOColorTheme::ScrollBarColorCfg       m_scrollBarColorCfg   ;
    LINGMOColorTheme::TabWidgetColorCfg       m_tabWidgetColorCfg ;
    LINGMOColorTheme::MenuColorCfg            m_menuColorCfg        ;

    LINGMORadiusInformation::LINGMORadiusStruct m_radiusStruct;


    ReadConfig                              *m_readConfig = nullptr;

    QString m_theme = "";
    bool m_loadConfig = false;
};

/*
class RadiusConfig : public QObject{
    Q_OBJECT
public:
    RadiusConfig(QString cfgPath);
    ~RadiusConfig();

    bool setRadius(QString key, int r);
    void getRadius(QString key, int &r);

private:
    QString m_cfgPath = "";
    ReadConfig *m_readConfig = nullptr;
    bool m_loadConfig = false;
};
*/
#endif // READCONFIG_H
