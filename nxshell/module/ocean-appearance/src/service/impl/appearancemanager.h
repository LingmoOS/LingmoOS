// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPEARANCEMANAGER_H
#define APPEARANCEMANAGER_H

#include "fsnotify.h"
#include "modules/subthemes/subthemes.h"
#include "modules/fonts/fontsmanager.h"
#include "dbus/scaleFactors.h"
#include "wallpaperscheduler.h"

#include <QObject>
#include <QDebug>
#include <DConfig>
#include <QDBusInterface>
#include <QMap>
#include <QTimerEvent>
#include <QTimer>
#include <QGSettings>
#include <QJsonArray>

#include <dbus/appearanceproperty.h>

class AppearanceDBusProxy;
class Appearance1;
class CustomTheme;

using Dtk::Core::DConfig;

class AppearanceManager : public QObject
{
    Q_OBJECT
    struct coordinate{
        double latitude;
        double longitude;
    };

    struct fontConfigItem{
        QString Standard;
        QString Monospace;
    };

public:
    explicit AppearanceManager(AppearanceProperty *prop, QObject *parent = nullptr);
    ~AppearanceManager();

    static QString qtActiveColorToHexColor(const QString &activeColor);
    static QString hexColorToQtActiveColor(const QString &hexColor);

public:
    bool init();
    void deleteThermByType(const QString &ty, const QString &name);
    void setFontSize(double value);
    void setGlobalTheme(QString value);
    void setGtkTheme(QString value);
    void setIconTheme(QString value);
    void setCursorTheme(QString value);
    void setStandardFont(QString value);
    void setMonospaceFont(QString value);
    void setWindowRadius(int value);
    void setOpacity(double value);
    void setQtActiveColor(const QString &value);
    bool setWallpaperSlideShow(const QString &value);
    bool setWallpaperURls(const QString &value);
    void setDTKSizeMode(int value);
    void setQtScrollBarPolicy(int value);
    void setActiveColors(const QString &value);
    bool doSetFonts(double size);
    bool doUpdateFonts(double size);
    bool doSetGlobalTheme(QString value);
    bool doSetGtkTheme(QString value);
    bool doSetIconTheme(QString value);
    bool doSetCursorTheme(QString value);
    bool doSetStandardFont(QString value);
    bool doSetMonospaceFont(QString value);
    bool doSetBackground(QString value);
    bool doSetGreeterBackground(QString value);
    QString doGetWallpaperSlideShow(QString monitorName);
    void doSetDTKSizeMode(int value);
    void doSetQtScrollBarPolicy(int value);
    double getScaleFactor();
    ScaleFactors getScreenScaleFactors();
    bool setScaleFactor(double scale);
    bool setScreenScaleFactors(ScaleFactors scaleFactors);
    QString doList(QString type);
    QString doShow(const QString& type, const QStringList& names);
    void doResetSettingBykeys(QStringList keys);
    void doResetFonts();
    void doSetByType(const QString& type,const QString& value);
    QString doSetMonitorBackground(const QString& monitorName,const QString& imageGile);
    bool doSetWallpaperSlideShow(const QString &monitorName,const QString &wallpaperSlideShow);
    bool doSetWsLoop(const QString& monitorName,const QString& file);
    QString doThumbnail(const QString &type, const QString &name);
    void doSetCurrentWorkspaceBackground(const QString &uri);
    QString doGetCurrentWorkspaceBackground();
    void doSetCurrentWorkspaceBackgroundForMonitor(const QString &uri, const QString &strMonitorName);
    QString doGetCurrentWorkspaceBackgroundForMonitor(const QString &strMonitorName);
    void doSetWorkspaceBackgroundForMonitor(const int &index, const QString &strMonitorName, const QString &uri);
    QString doGetWorkspaceBackgroundForMonitor(const int &index,const QString &strMonitorName);

    inline QString getBackground() {return m_property->background; }
    inline double getFontSize() {return m_property->fontSize; }
    inline QString getGLobalTheme() {return m_property->globalTheme; }
    inline QString getGtkTheme() {return m_property->gtkTheme; }
    inline QString getIconTheme() {return m_property->iconTheme; }
    inline QString getCursorTheme() {return m_property->cursorTheme; }
    inline QString getMonospaceFont() {return m_property->monospaceFont; }
    inline double getOpacity() {return m_property->opacity; }
    inline QString getQtActiveColor() {return m_property->qtActiveColor; }
    inline QString getStandardFont() {return m_property->standardFont; }
    inline QString getWallpaperSlideShow() {return m_property->wallpaperSlideShow; }
    inline QString getWallpaperURls() {return m_property->wallpaperURls;}
    inline int getWindowRadius() {return m_property->windowRadius; }
    inline int getDTKSizeMode() {return m_property->dtkSizeMode; }
    QString getActiveColors();

    inline QString getGreetBg() {return m_greeterBg; }
    inline QMap<QString,QString>& getMonitor() {return m_monitorMap; }
    inline QSharedPointer<AppearanceDBusProxy> getDBusProxy() const { return m_dbusProxy; }
    int getWorkspaceCount();
    void timerEvent(QTimerEvent *event) override;

public Q_SLOTS:
    void handleWmWorkspaceCountChanged(int count);
    void handleWmWorkspaceSwithched(int from,int to);
    void handleSetScaleFactorStarted();
    void handleSetScaleFactorDone();
    void handleTimezoneChanged(QString timezone);
    void handleTimeUpdate();
    void handleNTPChanged();
    void handlethemeFileChange(QString theme);
    void handleXsettingDConfigChange(QString key);
    void handleSettingDConfigChange(QString key);
    void handleWrapBgDConfigChange(QString key);
    void handleGnomeBgDConfigChange(QString key);
    void handleDetectSysClockTimeOut();
    void handleUpdateToCustom(const QString &mode);
    void handleGlobalThemeChangeTimeOut();
    void updateMonitorMap();
    void handlePrepareForSleep(bool sleep);
    void onOpacityTriggerTimeout();

private:
    void initCoordinate();
    void initUserObj();
    void initCurrentBgs();
    void initWallpaperSlideshow();
    void initDtkSizeMode();
    void iso6709Parsing(QString city, QString coordinates);
    void doUpdateWallpaperURIs();
    void setPropertyWallpaperURIs(QMap<QString,QString> monitorWallpaperUris);
    void updateNewVersionData();
    void autoSetTheme(double latitude, double longitude);
    void resetThemeAutoTimer();
    void updateThemeAuto(bool enable);
    void enableDetectSysClock(bool enable);
    void updateWSPolicy(QString policy);
    void loadWSConfig();
    QDateTime getThemeAutoChangeTime(QDateTime date, double latitude, double longitude);

    void autoChangeBg(QString monitorSpace, QDateTime date);
    bool changeBgAfterLogin(QString monitorSpace);
    bool setDQtTheme(QStringList key,QStringList value);
    bool saveWSConfig(QString monitorSpace, QDateTime date);
    QString marshal(const QVector<QSharedPointer<Theme>>& themes);
    QString marshal(const QVector<Background>& backgrounds);
    QString marshal(const QStringList& strs);
    QString marshal(const QVector<QSharedPointer<FontsManager::Family>>& strs);
    int getCurrentDesktopIndex();
    void applyGlobalTheme(KeyFile &theme, const QString &themeName, const QString &defaultTheme, const QString &themePath);

    void updateCustomTheme(const QString &type, const QString &value);
    bool isBgInUse(const QString &file);
    QVector<Background> backgroundListVerify(const QVector<Background>& backgrounds);
    QString getWallpaperUri(const QString &index, const QString &monitorName);
    void initGlobalTheme();
    bool isSkipSetWallpaper(const QString &themePath);

Q_SIGNALS:
    void Changed(const QString &ty, const QString &value);
    void Refreshed(const QString &type);

private: // PROPERTIES
    AppearanceProperty *m_property;

private:
    DConfig                                          m_settingDconfig;
    QSharedPointer<QGSettings>                       m_xSetting;
    QSharedPointer<QGSettings>                       m_wrapBgSetting;
    QSharedPointer<QGSettings>                       m_gnomeBgSetting;
    QSharedPointer<AppearanceDBusProxy>              m_dbusProxy;
    QSharedPointer<Subthemes>                        m_subthemes;
    QSharedPointer<Backgrounds>                      m_backgrounds;
    QSharedPointer<FontsManager>                     m_fontsManager;
    QMap<QString,QString>                            m_monitorMap;
    QMap<QString,coordinate>                         m_coordinateMap;
    double                                           m_longitude;
    double                                           m_latitude;
    QStringList                                      m_desktopBgs;
    QString                                          m_greeterBg;
    int                                              m_timeUpdateTimeId;
    int                                              m_ntpTimeId;
    bool                                             m_locationValid;
    QString                                          m_curMonitorSpace;
    //QSharedPointer<CursorChangeHandler> cursorChangeHandler;
    QSharedPointer<Fsnotify>                         m_fsnotify;
    QString                                          m_gsQtActiveColor;
    QTimer                                           m_detectSysClockTimer;
    QTimer                                           m_themeAutoTimer;     // 自动切换深浅主题定时器
    qint64                                           m_detectSysClockStartTime;
    QString                                          m_zone;
    QMap<QString,QSharedPointer<WallpaperScheduler>> m_wsSchedulerMap;
    QMap<QString,QSharedPointer<WallpaperLoop>>      m_wsLoopMap;
    CustomTheme                                      *m_customTheme;
    bool                                             m_globalThemeUpdating;
    QString                                          m_currentGlobalTheme; // 当前主题，globalTheme+.light/.dark
    QJsonArray                                       m_wallpaperConfig; // store the config
    QTimer                                           *m_opacityTriggerTimer; // 定时器限制opacity的变化速率，防止卡顿
    double                                           m_tmpOpacity;
    bool                                             m_setDefaulting;
};

#endif // APPEARANCEMANAGER_H
