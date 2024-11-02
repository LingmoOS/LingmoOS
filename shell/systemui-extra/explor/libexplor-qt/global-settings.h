/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QMutex>
#include <QDBusInterface>

#include "explor-core_global.h"
#include <gio/gio.h>

//文件管理器版本
#define PEONY_VERSION               "explorVersion"

//顶部菜单 - Top menu
#define RESIDENT_IN_BACKEND         "resident"
#define SHOW_HIDDEN_PREFERENCE      "showHiddenFile"
#define ALLOW_FILE_OP_PARALLEL      "allow-file-op-parallel"
#define FORBID_THUMBNAIL_IN_VIEW    "doNotThumbnail"
#define SHOW_IN_NEW_WINDOW          "showInNewWindow"
#define DISABLED_EXTENSIONS         "disabledExtensions"

//视图 - View
#define DEFAULT_VIEW_ID             "defaultViewId"
#define DEFAULT_VIEW_ZOOM_LEVEL     "defaultViewZoomLevel"
#define DEFAULT_DETAIL              "detail"
#define LABLE_ALIGNMENT             "labelAlignment"

//排序类型 - Sort type
// fix 112308, sort type option does no effect issue.
#define SORT_COLUMN                 "sortType"
#define SORT_TYPE                   "sortType"

//排序顺序 - Sort order
#define SORT_ORDER                  "sortOrder"

//排序偏好 - Sort preference
#define SORT_FOLDER_FIRST           "folderFirst"
#define SORT_CHINESE_FIRST          "chineseFirst"
#define USE_GLOBAL_DEFAULT_SORTING "useGlobalDefaultSorting"
#define SHOW_CREATE_TIME            "showCreateTime"

//窗口设置 - Window setting
#define DEFAULT_WINDOW_WIDTH        "defaultWindowWidth"
#define DEFAULT_WINDOW_HEIGHT       "defaultWindowHeight"
#define DEFAULT_WINDOW_SIZE         "default-window-size" //deprecated

//侧边栏 - Sidebar
#define DEFAULT_SIDEBAR_WIDTH       "defaultSidebarWidth"

//删除提示框 - tooltip
#define SHOW_TRASH_DIALOG           "showTrashDialog"

//状态栏 - Zoom Slider
#define ZOOM_SLIDER_VISIBLE         "zoomSliderVisible"

//多选模式 - Multiple selection
#define MULTI_SELECT                "multiSelectCheckboxEnabled"

//桌面配置 - Desktop setting
#define LAST_DESKTOP_SORT_ORDER     "lastDesktopSortOrder"
#define TEMPLATES_DIR               "templatesDir"
#define DEFAULT_DESKTOP_ZOOM_LEVEL  "defaultDesktopZoomLevel"
#define DEFAULT_GRID_SIZE           "default-grid-size"

//收藏的服务器IP - favorite ip
#define REMOTE_SERVER_REMOTE_IP     "remote-server/favorite-ip"

//gsettings
#define SIDEBAR_BG_OPACITY          "sidebar-bg-opacity" // deprecated
#define TABLET_MODE                 "tablet-mode"

#define FONT_SETTINGS               "org.lingmo.style"

//difference between Community version and Commercial version
#define COMMERCIAL_VERSION          true

//其它
#define INIT_FOR_FIRST_TIME         "initForFirstTime"  /* org.lingmo.explor.settings 首次初始化标志 */
#define SHOW_FILE_EXTENSION         "showFileExtension" /* 显示文件扩展名 */
#define SEND_URIS_OF_COPY_DSPS      "sendUrisOfCopyDsps"  /* send src and dest uris of copy dsps files to wps,默认值为false */
#define DOC_IS_OCCUPIED_BY_WPS      "docIsOccupiedByWps" /* 是否开启wps占用文件监控功能 */

#define LINGMO_SEARCH_SCHEMAS          "org.lingmo.search.settings"
#define SEARCH_METHOD_KEY            "fileIndexEnable"

//Control the display of desktop standard icons
#define DISPLAY_STANDARD_ICONS       "displayStandardIcons"
#define HOME_ICON_VISIBLE            "homeIconVisible"
#define TRASH_ICON_VISIBLE           "trashIconVisible"
#define COMPUTER_ICON_VISIBLE        "computerIconVisible"

//control the mobile device trash file issue, if be true can trash mobile files
#define TRASH_MOBILE_FILES            "trashMobileFiles"

// control center
#define LINGMO_CONTROL_CENTER_PANEL_PLUGIN            "org.lingmo.control-center.panel.plugins"                 // schema
#define LINGMO_CONTROL_CENTER_PANEL_PLUGIN_TIME       "org.lingmo.control-center.panel.plugins.time"            // time format key, value is '12' or '24'
#define LINGMO_CONTROL_CENTER_PANEL_PLUGIN_DATE       "org.lingmo.control-center.panel.plugins.date"            // date format key, value is cn or en

// guestos machine
#define IS_GUESTOS_MACHINE           "isGuestOSMachine"

//gsettings
#define PERSONAL_EFFECT_SCHEMA       "org.lingmo.control-center.personalise"
#define PERSONAL_EFFECT_ENABLE       "effect"
#define PERSONAL_EFFECT_TRANSPARENCY "transparency"

#define SHOW_NETWORK                "showNetwork"

#define SHOW_RELATIVE_DATE          "showRelativeDate"

//intel 个性化设置，透明度...
//schema
#define LINGMO_CONTROL_CENTER_PERSONALISE             "org.lingmo.control-center.personalise"
//keys
#define PERSONALISE_EFFECT                          "effect"
#define PERSONALISE_TRANSPARENCY                    "transparency"

#define SETTINGS_DAEMON_SCHEMA_XRANDR        "org.lingmo.SettingsDaemon.plugins.xrandr"
#define DUAL_SCREEN_MODE                     "xrandrMirrorMode"
#define DUAL_SCREEN_EXPAND_MODE              "expand"
#define DUAL_SCREEN_MIRROR_MODE              "mirror"

//dbus
#define DBUS_STATUS_MANAGER_IF      "com.lingmo.statusmanager.interface"

#ifdef LINGMO_SDK_DATE
#define SDK_DATE_SERVER_PATH "/com/lingmo/lingmosdk/Date"
#define SDK_DATE_SERVER_SERVICE "com.lingmo.lingmosdk.DateServer"
#define SDK_DATE_SERVER_INTERFACE "com.lingmo.lingmosdk.DateInterface"
#endif

class QGSettings;

namespace Peony {

/*!
 * \brief The GlobalSettings class
 * \details
 * this class provide a global settings for explor-qt.
 * it most be used to save and load preferences of explor-qt, such as default view,
 * sort type, etc.
 *
 * you can also save another kind of datas using by extensions. such as enable properties.
 * this class instance is shared in both explor-qt and its plugins.
 */
class PEONYCORESHARED_EXPORT GlobalSettings : public QObject
{
    Q_OBJECT
public:
    static GlobalSettings *getInstance();
    const QVariant getValue(const QString &key);
    bool isExist(const QString &key);
    bool initDateFormatDBus();
    QString getProjectName();

    bool getShowCreateTime() const;

Q_SIGNALS:
    void valueChanged(const QString &key);
    void updateHiddenFile(const QString &fileName);
    void signal_updateRemoteServer(const QString& server, bool add);
    void updateShortDataFormat(const QString &fileName);
    void updateLongDataFormat(const QString &fileName);

public Q_SLOTS:
    void setValue(const QString &key, const QVariant &value);
    void reset(const QString &key);
    void resetAll();
    void setTimeFormat(const QString &value);
    void setDateFormat(const QString &value);
    void sendShortDataFormat(const QString &format);
    void sendLongDataFormat(const QString &format);
    QString getSystemTimeFormat();
    QString transToSystemTimeFormat(guint64 mtime, bool longFormat=false);

    /*!
     * \brief 通过GSetting保存设置
     * \param key
     * \param value
     */
    void setGSettingValue(const QString &key, const QVariant &value);

    /*!
     * \brief forceSync
     * \param key
     * \details
     * use force sync to ensure the settings key-value and cached map's data
     * keep same. this may be used in out progress, such as explor-qt-desktop.
     */
    void forceSync(const QString &key = nullptr);
    void slot_updateRemoteServer(const QString& server, bool add);

    bool isGuestOSMachine();

private:
    explicit GlobalSettings(QObject *parent = nullptr);
    ~GlobalSettings();

    void getLingmoUIStyle();
    void getMachineMode();
    void getDualScreenMode();

    QSettings*                  m_settings;
    QMap<QString, QVariant>     m_cache;

    QGSettings*                 m_gsettings = nullptr;
    QGSettings*                 m_control_center_plugin = nullptr;
    QGSettings*                 m_explor_gsettings  = nullptr;
    QGSettings*                 m_explorGSettings  = nullptr; //fixme: 代码冲突遗留，后续仅保留一个explor gsettings实例
    QGSettings *m_gsettings_tablet_mode = nullptr;
    QGSettings *m_gsettings_dual_screen_mode = nullptr;
    QMutex                      m_mutex;

    QString                     m_date_format = "";
    QString                     m_time_format = "";
    QString                     m_system_time_format  = "";

    QDBusInterface*             mDbusDateServer = nullptr;
    bool m_showCreateTime = false;
    bool m_showRelativeTime = false;
};

}

#endif // GLOBALSETTINGS_H
