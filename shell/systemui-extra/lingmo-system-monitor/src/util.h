/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntulingmo.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <QString>
#include <QSet>
#include <QFile>
#include <QTextStream>
#include <QObject>
#include <QPainter>

#define MONITOR_TITLE_WIDGET_HEIGHT 88
#define TITLE_WIDGET_HEIGHT 39
#define DEVICE_MOUNT_PONINT_RECORD_FILE "/proc/mounts"
#define FontSize 12

using std::string;

std::string getDesktopFileAccordProcName(QString procName, QString cmdline);
std::string getDesktopFileAccordProcNameApp(QString procName, QString cmdline);
QPixmap getAppIconFromDesktopFile(std::string desktopFile, int iconSize = 24);
QString getDisplayNameAccordProcName(QString procName, std::string desktopFile);
QString getAppIconPathFromDesktopFile(std::string desktopFile);
std::string make_string(char *c_str);
QString formatProcessState(guint state);
QString getNiceLevel(int nice);
QString getNiceLevelWithPriority(int nice);
QString formatUnitSize(double v, const char** orders, int nb_orders);
QString formatByteCount(double v);
void setFontSize(QPainter &painter, int textSize);
QString formatDurationForDisplay(unsigned centiseconds);
QString getDeviceMountedPointPath(const QString &line);
QString getFileContent(const QString &filePath);
QString getElidedText(QFont font, QString str, int MaxWidth);
QString getMiddleElidedText(QFont font, QString str, int MaxWidth);
QSet<QString> getFileContentsLineByLine(const QString &filePath);

bool loadSvg(const QString &fileName, const int size, QPixmap& pixmap);

//图片反白
QPixmap drawSymbolicColoredPixmap(const QPixmap &source);

//图片反黑
QPixmap drawSymbolicBlackColoredPixmap(const QPixmap &source);

// 获取应用程序版本
QString getUsmVersion();