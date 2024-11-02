/*
* Copyright (C) 2023, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "calendarbuttontext.h"
#include <lingmo/common.h>
#include <QtDBus/QtDBus>

#define  LINGMO_CONTROL_CENTER_SERVER     "org.freedesktop.Accounts"
#define  LINGMO_CONTROL_CENTER_PATH       "/org/freedesktop/Accounts/User1000"
#define  LINGMO_CONTROL_CENTER_INTERFACE  "org.freedesktop.Accounts.User"
#define  PROPERTIES_NAME                "FormatsLocale"

#define SYSTEM_FONT_SET  "org.lingmo.style"
#define SYSTEM_FONT_SIZE "systemFontSize"
#define SYSTEM_FONT      "systemFont"
#define SYSTEM_MIN_FONT      10

#define PANEL_SETTINGS      "org.lingmo.panel.settings"
#define PANEL_POSITION_KEY  "panelposition"
#define ICON_SIZE_KEY       "iconsize"
#define PANEL_SIZE_KEY      "panelsize"

#define CALENDAR_BUTTON_WIDTH  120
#define PANEL_SIZE_LARGE  92
#define PANEL_SIZE_MEDIUM 70
#define PANEL_SIZE_SMALL  46
#define PANEL_SIZE_KEY    "panelsize"

#define  EN_FORMATS                        "en_US"

#define INSIDE_SPAC 6


CalendarButtonText::CalendarButtonText(ILINGMOPanelPlugin *plugin, QPushButton *parent):
    m_plugin(plugin),
    m_parent(parent)
{
    const QByteArray fontId(SYSTEM_FONT_SET);
    if (QGSettings::isSchemaInstalled(fontId)) {
        m_fontGsettings = new QGSettings(fontId);
    }
    const QByteArray panelId(PANEL_SETTINGS);
    if (QGSettings::isSchemaInstalled(panelId)) {
        m_panelGsettings = new QGSettings(panelId);
    }
}

CalendarButtonText::~CalendarButtonText()
{
    if(m_fontGsettings) {
        m_fontGsettings->deleteLater();
    }
    if(m_panelGsettings) {
        m_panelGsettings->deleteLater();
    }
}

/*
 * 更新时间文字显示
 * 横向任务栏显示格式(中文)：
 *      24小时制：
 *              hh:mm ddd
 *              yyyy/MM/dd
 *      12小时制：
 *              AM hh:mm ddd
 *              yyyy/MM/dd
 *
 * 纵向任务栏显示格式:
 *      24小时制：
 *              hh:mm        或   hh:mm
 *               ddd               ddd
 *              yyyy/MM/dd        MM/dd
 *      12小时制：
 *               AM                  AM
 *              hh:mm               hh:mm
 *               ddd          或     ddd
 *              yyyy/MM/dd          MM/dd
*/
QString CalendarButtonText::getBtnText()
{
    QString timeStr = SDK2CPP(kdk_system_nowtime)();
    QString weekStr = SDK2CPP(kdk_system_longweek)();
    QString dateStr = SDK2CPP(kdk_system_get_shortformat_date)();

    if(QLocale::system().name() == EN_FORMATS) {
        weekStr = SDK2CPP(kdk_system_shortweek)();
    }

    QString btnTextStr;
    if(m_plugin->panel()->isHorizontal()) {
        btnTextStr = timeStr + " " + weekStr + "\n" + dateStr;
    } else {
        if (timeStr.contains(" ")) {
            QStringList list = timeStr.split(" ");
            timeStr = list.takeFirst() + "\n" + list.takeLast();
        } else {
            auto iter = std::find_if(timeStr.begin(), timeStr.end(), [](auto c){
                return c >= "0" && c <= "9";
            });
            int numAt = timeStr.indexOf(*iter);
            if (numAt > 0) {
                QString ampmStr;
                ampmStr = timeStr.left(numAt);
                timeStr = ampmStr + "\n" + timeStr.mid(numAt);
            }
        }

        if(getFormatsLocale().contains(EN_FORMATS)) {
            if (dateStr.contains("/")) {
                dateStr = dateStr.section("/", 0, 1);
            } else if (dateStr.contains("-")) {
                dateStr = dateStr.section("-", 0, 1);
            } else if (dateStr.contains(".")) {
                dateStr = dateStr.section(".", 0, 1);
            }
        } else {
            if (dateStr.contains("/")) {
                dateStr = dateStr.section("/", 1);
            } else if (dateStr.contains("-")) {
                dateStr = dateStr.section("-", 1);
            } else if (dateStr.contains(".")) {
                dateStr = dateStr.section(".", 1);
            }
        }

        btnTextStr = timeStr + "\n" + weekStr + "\n" + dateStr;
    }
    updateFontSize(btnTextStr);
    return btnTextStr;
}

void CalendarButtonText::updateFontSize(QString btnText)
{
    if (m_panelGsettings == nullptr) {
        qWarning()<<Q_FUNC_INFO<<__LINE__<<"gsettings init error!";
        return;
    }
    QStringList textList = btnText.split("\n", QString::SkipEmptyParts);

    int maxLength;
    if(m_plugin->panel()->isHorizontal()) {
        if(m_panelGsettings->get(PANEL_SIZE_KEY).toInt() < PANEL_SIZE_MEDIUM) {
            if (m_fontGsettings != nullptr) {
                QString systemFont = m_fontGsettings->get(SYSTEM_FONT).toString();
                m_parent->setFont(QFont(systemFont, SYSTEM_MIN_FONT));
            }
            return;
        }
        maxLength = CALENDAR_BUTTON_WIDTH;
        setOptimalFont(textList, maxLength);
    } else {
        int m_panelSize = m_panelGsettings->get(PANEL_SIZE_KEY).toInt();
        if(m_panelSize >= PANEL_SIZE_SMALL && m_panelSize < PANEL_SIZE_MEDIUM) {
            maxLength = PANEL_SIZE_SMALL;
        } else if(m_panelSize >= PANEL_SIZE_MEDIUM && m_panelSize < PANEL_SIZE_LARGE) {
            maxLength = PANEL_SIZE_MEDIUM;
        } else if(m_panelSize == PANEL_SIZE_LARGE) {
            maxLength = PANEL_SIZE_LARGE;
        } else {
            maxLength = PANEL_SIZE_SMALL;
        }
        setOptimalFont(textList, maxLength);
    }
}

//判断最长字体的是否可以显示下，选取最优字体大小显示
void CalendarButtonText::setOptimalFont(QStringList textList, int btnMaxLength)
{
    if (m_fontGsettings == nullptr) {
        qWarning()<<Q_FUNC_INFO<<__LINE__<<"gsettings init error!";
        return;
    }
    QString systemFont = m_fontGsettings->get(SYSTEM_FONT).toString();
    double fontSizeCurrent = m_fontGsettings->get(SYSTEM_FONT_SIZE).toDouble();
    QList<double> systemFonts;
    systemFonts<<10<<12<<13.5<<15;

    //获取最长的行
    QFontMetrics fontMetrics(QFont(systemFont, fontSizeCurrent));
    int fontWidth = fontMetrics.width(textList.first());
    int maxIndex = 0;
    for(int i=1; i<textList.size(); i++) {
        int width = fontMetrics.width(textList.at(i));
        if(width > fontWidth) {
            fontWidth = width;
            maxIndex = i;
        }
    }

    //判断最长字体的是否可以显示下，选取最优字体
    int maxFontWidth = fontMetrics.width(textList.at(maxIndex));
    if(maxFontWidth > btnMaxLength-INSIDE_SPAC) {
        int index = systemFonts.indexOf(fontSizeCurrent);
        if(index>0) {
            for(int i=index-1; i>=0; i--) {
                QFontMetrics fontMetrics(QFont(systemFont, systemFonts.at(i)));
                int width = fontMetrics.width(textList.at(maxIndex));
                if(width <= btnMaxLength-INSIDE_SPAC) {
                    m_parent->setFont(QFont(systemFont, systemFonts.at(i)));
                    break;
                }
            }
        } else {
            //容错处理，如果系统 org.lingmo.style system-font-size 设置错误，也可保证日期正常显示不遮挡
            m_parent->setFont(QFont(systemFont, systemFonts.first()));
        }
    } else {
        m_parent->setFont(QFont(systemFont, fontSizeCurrent));
    }
}

QString CalendarButtonText::getFormatsLocale()
{
    QDBusInterface interface(LINGMO_CONTROL_CENTER_SERVER,
                             LINGMO_CONTROL_CENTER_PATH,
                             LINGMO_CONTROL_CENTER_INTERFACE,
                             QDBusConnection::systemBus());
    QVariant ret = interface.property(PROPERTIES_NAME);
    return ret.toString();
}
