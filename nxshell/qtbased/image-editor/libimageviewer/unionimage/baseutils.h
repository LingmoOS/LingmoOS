// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEUTILS_H
#define BASEUTILS_H

#include <QObject>
#include <QTimer>
#include <QColor>
#include <QImage>

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

namespace Libutils {
namespace common {
const int TOP_TOOLBAR_THEIGHT = 40;
const int BOTTOM_TOOLBAR_HEIGHT = 22;

const int BORDER_RADIUS = 0;
const int BORDER_WIDTH = 1;
const int BORDER_WIDTH_SELECTED = 2;
const int THUMBNAIL_MAX_SCALE_SIZE = 192;

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
void        copyImageToClipboard(const QStringList &paths, const QImage &sourceImage = QImage());
void        showInFileManager(const QString &path);
int         stringWidth(const QFont &f, const QString &str);
int         stringHeight(const QFont &f, const QString &str);

QPixmap     renderSVG(const QString &filePath, const QSize &size);
QString     hash(const QString &str);
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
bool checkCommandExist(const QString &command);
//bool        isCommandExist(const QString &command);

bool        initCheckWaylandEnv();
bool        checkWayland();

bool        loadLibTransaltor();
}  // namespace base

}  // namespace utils

#endif // BASEUTILS_H
