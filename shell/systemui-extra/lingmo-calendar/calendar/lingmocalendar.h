/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Copyright: 2012-2013 Razor team
 * Authors:
 *   Kuzma Shapran <kuzma.shapran@gmail.com>
 *
 * Copyright: 2019 Tianjin LINGMO Information Technology Co., Ltd. *
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#include <QTimeZone>

#include <QDialog>
#include <QLabel>
#include <QGSettings>
#include <QMenu>
#include <QProcess>
#include <QTranslator>
#include <QtDBus/QtDBus>

#include "../panel/ilingmopanelplugin.h"
#include "lingmowebviewdialog.h"
#include "../panel/common/lingmogridlayout.h"
#include "../panel/common_fun/listengsettings.h"
#include "lunarcalendarwidget/frmlunarcalendarwidget.h"
class QTimer;
class CalendarActiveLabel;
class LingmoUICalendarWebView;

class IndicatorCalendar : public QWidget, public ILINGMOPanelPlugin
{
    Q_OBJECT
public:
    IndicatorCalendar(const ILINGMOPanelPluginStartupInfo &startupInfo);
    ~IndicatorCalendar();

    virtual QWidget *widget() { return mMainWidget; }
    virtual QString themeId() const { return QLatin1String("Calendar"); }
    bool isSeparate() const { return true; }
    void realign()override;
    void initializeCalendar();
    void setTimeShowStyle();
    /**
     * @brief modifyCalendarWidget 修改日历显示位置
     */
    void modifyCalendarWidget();
Q_SIGNALS:
    void deactivated();

private Q_SLOTS:
    void checkUpdateTime();
    void updateTimeText();
    void hidewebview();
    void CalendarWidgetShow();
    void ListenForManualSettingTime();
    void timeChange(QString time);

private:
    QWidget *mMainWidget;
    frmLunarCalendarWidget *w;
    LingmoUIWebviewDialog   *mWebViewDiag;
    bool mbActived;
    bool mbHasCreatedWebView;
    int font_size;
    CalendarActiveLabel *mContent;
    UKUi::GridLayout *mLayout;
    QString timeState;
    QTimer *mTimer;
    QTimer *mCheckTimer;
    int mUpdateInterval;

    int16_t mViewWidht;
    int16_t mViewHeight;

    QString mActiveTimeZone;

    QGSettings *gsettings;
    QGSettings *fgsettings;
    QString hourSystemMode;

    QString hourSystem_24_horzontal;
    QString hourSystem_24_vartical;
    QString hourSystem_12_horzontal;
    QString hourSystem_12_vartical;
    QString current_date;
    ILINGMOPanelPlugin * mPlugin;

    QProcess *mProcess;
    QTranslator *m_translator;

private:
    void translator();

};



#define SERVICE "org.lingmo.panel.calendar"
#define PATH "/calendarWidget"
#define INTERFACE "org.lingmo.panel.calendar"
class CalendarActiveLabel : public QLabel
{
Q_OBJECT

public:
    explicit CalendarActiveLabel(ILINGMOPanelPlugin *plugin,QWidget * = NULL);

    ILINGMOPanelPlugin * mPlugin;
    int16_t mViewWidht = 440;
    int16_t mViewHeight = 600 ;

    int changeHight = 0;
    void changeWidowpos();

protected:
    /**
     * @brief contextMenuEvent 右键菜单设置项
     * @param event
     */
    virtual void contextMenuEvent(QContextMenuEvent *event);
    void mousePressEvent(QMouseEvent *event);
private:
    frmLunarCalendarWidget *w;
    QDBusInterface *mInterface;


Q_SIGNALS:
    void pressTimeText();

private Q_SLOTS:
    /**
     * @brief setControlTime 右键菜单选项，在控制面板设置时间
     */
    void setControlTime();

};

class LINGMOCalendarPluginLibrary: public QObject, public ILINGMOPanelPluginLibrary
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "lingmo.org/Panel/PluginInterface/3.0")
    Q_INTERFACES(ILINGMOPanelPluginLibrary)
public:
    ILINGMOPanelPlugin *instance(const ILINGMOPanelPluginStartupInfo &startupInfo) const
    {
        return new IndicatorCalendar(startupInfo);
    }
};
