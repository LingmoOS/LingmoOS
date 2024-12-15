// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEUTILS_H
#define BASEUTILS_H

#include <QObject>
#include <QTimer>
#include <QColor>

#define V23_FILEMANAGER_DAEMON_SERVICE      "org.lingmo.filemanager.server"
#define V23_FILEMANAGER_DAEMON_PATH         "/org/lingmo/filemanager/server/DeviceManager"
#define V23_FILEMANAGER_DAEMON_INTERFACE    "org.lingmo.filemanager.server.DeviceManager"

#define V25_FILEMANAGER_DAEMON_SERVICE      "org.lingmo.Filemanager.Daemon"
#define V25_FILEMANAGER_DAEMON_PATH         "/org/lingmo/Filemanager/Daemon/DeviceManager"
#define V25_FILEMANAGER_DAEMON_INTERFACE    "org.lingmo.Filemanager.Daemon.DeviceManager"

/*!
   \brief 拷贝自 ocean-file-manager 代码，看图中仅使用 kRemovable 访问可能被卸载的设备
 */
enum DeviceQueryOption {
    kNoCondition = 0,
    kMountable = 1,
    kMounted = 1 << 1,
    kRemovable = 1 << 2,
    kNotIgnored = 1 << 3,
    kNotMounted = 1 << 4,
    kOptical = 1 << 5,
    kSystem = 1 << 6,
    kLoop = 1 << 7,
};

#if QT_VERSION >= 0x050500
#define TIMER_SINGLESHOT(Time, Code, captured...){ \
        QTimer::singleShot(Time, [captured] {Code});\
    }
#else
#define TIMER_SINGLESHOT(Time, Code, captured...){ \
        QTimer *timer = new QTimer;\
        timer->setSingleShot(true);\
        QObject::connect(timer, &QTimer::timeout, [timer, captured] {\
                                                                     timer->deleteLater();\
                                                                     Code\
                                                                    });\
        timer->start(Time);\
    }

#endif

#define VIEW_CONTEXT_MENU "View"
#define FULLSCREEN_CONTEXT_MENU "Fullscreen"
#define PRINT_CONTEXT_MENU "Print"
#define SLIDESHOW_CONTEXT_MENU "Slide show"
#define PRINT_CONTEXT_MENU "Print"
#define EXPORT_CONTEXT_MENU "Export"
#define COPYTOCLIPBOARD_CONTEXT_MENU "Copy"
#define DELETE_CONTEXT_MENU "Delete"
#define THROWTOTRASH_CONTEXT_MENU "Move to trash"
#define REMOVEFROMALBUM_CONTEXT_MENU "Delete from album"
#define UNFAVORITE_CONTEXT_MENU "Unfavorite"
#define FAVORITE_CONTEXT_MENU "Favorite"
#define ROTATECLOCKWISE_CONTEXT_MENU "Rotate clockwise"
#define ROTATECOUNTERCLOCKWISE_CONTEXT_MENU "Rotate counterclockwise"
#define SETASWALLPAPER_CONTEXT_MENU "Set as wallpaper"
#define DISPLAYINFILEMANAGER_CONTEXT_MENU "Display in file manager"
#define ImageInfo_CONTEXT_MENU "Image info"
#define VideoInfo_CONTEXT_MENU "Video info"
#define BUTTON_RECOVERY "Recovery"
#define SHOW_SHORTCUT_PREVIEW "Show shortcut preview"

#define VAULT_DECRYPT_DIR_NAME          "vault_unlocked"
#define VAULT_BASE_PATH (QDir::homePath() + QString("/.local/share/applications"))  //! 获取保险箱创建的目录地址

namespace Libutils {
namespace common {
const int TOP_TOOLBAR_THEIGHT = 40;
const int BOTTOM_TOOLBAR_HEIGHT = 22;

const int BORDER_RADIUS = 8;
const int SHADOW_BORDER_RADIUS = 14;
const int BORDER_WIDTH = 0;
const int BORDER_WIDTH_SELECTED = 7;
const int THUMBNAIL_MAX_SCALE_SIZE = 192;
const int ALBUM_NAME_MAX_LENGTH = 255;

//const QColor DARK_BACKGROUND_COLOR = QColor("#202020");
//const QColor LIGHT_BACKGROUND_COLOR = QColor("#FFFFFF");
const QColor DARK_BACKGROUND_COLOR = QColor("#252525");
const QColor LIGHT_BACKGROUND_COLOR = QColor("#F8F8F8");

const QColor LIGHT_CHECKER_COLOR = QColor("#FFFFFF");
const QColor DARK_CHECKER_COLOR = QColor("#CCCCCC");

const QColor DARK_BORDER_COLOR = QColor(255, 255, 255, 26);
const QColor LIGHT_BORDER_COLOR = QColor(0, 0, 0, 15);

const QColor DARK_TITLE_COLOR = QColor("#FFFFFF");
const QColor LIGHT_TITLE_COLOR = QColor(48, 48, 48);
//由于qrc路径变更,代码中使用也得变更
const QString DARK_DEFAULT_THUMBNAIL = ":/dark/images/default_thumbnail.png";
const QString LIGHT_DEFAULT_THUMBNAIL = ":/light/images/default_thumbnail.png";

const QColor BORDER_COLOR_SELECTED = QColor("#01bdff");
const QColor SELECTED_RECT_COLOR = QColor(44, 167, 248, 26);
const QColor TOP_LINE2_COLOR_DARK = QColor(255, 255, 255, 13);
const QColor TOP_LINE2_COLOR_LIGHT = QColor(255, 255, 255, 153);
const QColor TITLE_SELECTED_COLOR = QColor("#2ca7f8");

//快捷键
const QString ENTER_SHORTCUT = "Enter";
const QString RETURN_SHORTCUT = "Return";
const QString F11_SHORTCUT = "F11";
const QString F5_SHORTCUT = "F5";
const QString CTRLC_SHORTCUT = "Ctrl+C";
const QString DELETE_SHORTCUT = "Delete";
const QString SHIFTDEL_SHORTCUT = "Shift+Del";
const QString CTRLK_SHORTCUT = "Ctrl+K";
const QString CTRLSHIFTK_SHORTCUT = "Ctrl+Shift+K";
const QString CTRLR_SHORTCUT = "Ctrl+R";
const QString CTRLSHIFTR_SHORTCUT = "Ctrl+Shift+R";
//const QString CTRLF8_SHORTCUT = "Ctrl+F8";
const QString CTRLF9_SHORTCUT = "Ctrl+F9";
//const QString CTRLD_SHORTCUT = "Ctrl+D";
const QString ALTD_SHORTCUT = "Alt+D";
//const QString ALTRETURN_SHORTCUT = "Alt+Return";
const QString CTRLI_SHORTCUT = "Ctrl+I";
const QString CTRLSHIFTN_SHORTCUT = "Ctrl+Shift+N";
const QString F2_SHORTCUT = "F2";
const QString CTRLO_SHORTCUT = "Ctrl+O";
const QString CTRLQ_SHORTCUT = "Ctrl+Q";
const QString CTRLUP_SHORTCUT = "Ctrl+=";
const QString CTRLDOWN_SHORTCUT = "Ctrl+-";
const QString CTRLSHIFTSLASH_SHORTCUT = "Ctrl+Shift+/";
const QString CTRLE_SHORTCUT = "Ctrl+E";
const QString RECTRLUP_SHORTCUT = "Ctrl++";
const QString CTRLF_SHORTCUT = "Ctrl+F";
const QString SENTENCE_SHORTCUT = ".";
}
namespace timeline {
const QColor DARK_SEPERATOR_COLOR = QColor(255, 255, 255, 20);
const QColor LIGHT_SEPERATOR_COLOR = QColor(0, 0, 0, 20);
}
namespace album {
const QColor DARK_DATELABEL_COLOR = QColor(255, 255, 255, 153);
const QColor LIGHT_DATELABEL_COLOR = QColor(48, 48, 48, 255);
//由于qrc路径变更,代码中使用也得变更
const QString DARK_CREATEALBUM_NORMALPIC = ":/dark/images/"
                                           "create_album_normal.png";
const QString DARK_CREATEALBUM_HOVERPIC = ":/dark/images/"
                                          "create_album_hover.png";
const QString DARK_CREATEALBUM_PRESSPIC = ":/dark/images/"
                                          "create_album_press.png";
const QString LIGHT_CREATEALBUM_NORMALPIC = ":/light/images/"
                                            "create_album_normal.png";
const QString LIGHT_CREATEALBUM_HOVERPIC = ":/light/images/"
                                           "create_album_hover.png";
const QString LIGHT_CREATEALBUM_PRESSPIC = ":/light/images/"
                                           "create_album_press.png";

const QString DARK_ADDPIC = ":/dark/images/album_add.svg";
const QString LIGHT_ADDPIC = ":/light/images/album_add.svg";

const QString DARK_ALBUM_BG_NORMALPIC = ":/dark/images/"
                                        "album_bg_normal.png";
const QString DARK_ALBUM_BG_PRESSPIC = ":/dark/images/"
                                       "album_bg_press.png";

const QString LIGHT_ALBUM_BG_NORMALPIC = ":/light/images/"
                                         "album_bg_normal.svg";
const QString LIGHT_ALBUM_BG_HOVERPIC = ":/light/images/"
                                        "album_bg_hover.svg";
const QString LIGHT_ALBUM_BG_PRESSPIC = ":/light/images/"
                                        "album_bg_press.svg";
}
namespace view {
const QString DARK_DEFAULT_THUMBNAIL =
    ":/dark/images/empty_defaultThumbnail.png";
const QString LIGHT_DEFAULT_THUMBNAIL =
    ":/light/images/empty_defaultThumbnail.png";
const QString DARK_LOADINGICON =
    ":/dark/images/dark_loading.gif";
const QString LIGHT_LOADINGICON =
    ":/light/images/light_loading.gif";
const QString DARK_DAMAGEICON =
    ":/resources/images/other/picture damaged_dark.svg";
const QString LIGHT_DAMAGEICON =
    ":/resources/images/other/picture damaged_light.svg";
namespace naviwindow {
const QString DARK_BG_IMG = ":/dark/images/naviwindow_bg.svg";
const QColor DARK_BG_COLOR = QColor(0, 0, 0, 100);
const QColor DARK_MR_BG_COLOR = QColor(0, 0, 0, 150);
const QColor DARK_MR_BORDER_Color = QColor(255, 255, 255, 80);
const QColor DARK_IMG_R_BORDER_COLOR = QColor(255, 255, 255, 50);

const QString LIGHT_BG_IMG = ":/light/images/naviwindow_bg.svg";
const QColor LIGHT_BG_COLOR = QColor(255, 255, 255, 104);
const QColor LIGHT_MR_BG_COLOR = QColor(0, 0, 0, 101);
const QColor LIGHT_MR_BORDER_Color = QColor(255, 255, 255, 80);
const QColor LIGHT_IMG_R_BORDER_COLOR = QColor(255, 255, 255, 50);
}
}
namespace widgets {

}
namespace base {
void        copyOneImageToClipboard(const QString &path);
void        copyImageToClipboard(const QStringList &paths);
void        showInFileManager(const QString &path);
int         stringWidth(const QFont &f, const QString &str);
int         stringHeight(const QFont &f, const QString &str);

QPixmap     renderSVG(const QString &filePath, const QSize &size);
bool        checkMimeUrls(const QList<QUrl> &urls);
QString     hash(const QString &str);
QString     hashByString(const QString &str);
QString     hashByData(const QString &str);
QString     mkMutiDir(const QString &path);
//根据源文件路径生产缩略图路径
QString     filePathToThumbnailPath(const QString &filePath, QString dataHash = "");
//QString     wrapStr(const QString &str, const QFont &font, int maxWidth);
QString     SpliteText(const QString &text, const QFont &font, int nLabelSize, bool bReturn = false);
//QString     sizeToHuman(const qlonglong bytes);
QString     timeToString(const QDateTime &time, bool normalFormat = false);
QDateTime   stringToDateTime(const QString &time);
QString     getFileContent(const QString &file);
//QString     symFilePath(const QString &path);
//bool        writeTextFile(QString filePath, QString content);

bool        trashFile(const QString &file);
//bool        trashFiles(const QStringList &files);

bool        onMountDevice(const QString &path);
bool        mountDeviceExist(const QString &path);
//bool        isCommandExist(const QString &command);

//生成lingmo-album-delete下的文件路径，hash：原始路径hash，fileName：文件名
QString getDeleteFullPath(const QString &hash, const QString &fileName);

std::pair<QDateTime, bool>          analyzeDateTime(const QVariant &data);

//同步文件拷贝，用于区分QFile::copy的异步拷贝
bool syncCopy(const QString &srcFileName, const QString &dstFileName);
//判断图片是否支持设置壁纸
bool isSupportWallpaper(const QString &path);
}  // namespace base

}  // namespace utils

#endif // BASEUTILS_H
