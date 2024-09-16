// Copyright (C) 2018 ~ 2025 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H


//#include "find_window_title.h"


#include <QFileInfo>
#include <QString>
#include <QVariant>
#include <QWidget>


const int RECTANGLE_PADDING = 24;
const int RECTANGLE_RADIUS = 8;
const int RECTANGLE_FONT_SIZE = 11;

class QPainter;

namespace Utils {

enum SizeUnit {
    B,      // 1024 ^ 0
    KB,     // 1024 ^ 1
    MB,     // 1024 ^ 2
    GB,     // 1024 ^ 3
    TB,     // 1024 ^ 4
    PB,     // 1024 ^ 5
    EB      // 1024 ^ 6
};

const char *const UnitSuffix[] = {"B", "K", "M", "G", "T", "P", "E"};
const char *const UnitSuffixExt[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB"};

inline QString formatUnit(QVariant size, SizeUnit base = B, int prec = 1, bool isSpeed = false)
{
    int u = base;
    if (!size.canConvert(QMetaType::Double)) return {};
    qreal v = size.toReal();

    while (v > 1024. && u <= EB) {
        v /= 1024;
        u++;
    }

    if (isSpeed) {
        return QString("%1%2%3").arg(v, 0, 'f', prec).arg(UnitSuffixExt[u]).arg("/s");
    } else {
        return QString("%1%2").arg(v, 0, 'f', prec).arg(UnitSuffix[u]);
    }
}

inline QString normalizeProcName(const QString &proc_name, const QByteArrayList &cmdline)
{
    if (cmdline.size() == 0 || proc_name.length() < 15) {
        return proc_name;
    }

    // windows apps case
    if (cmdline[0].length() > 3 &&
            isalpha(cmdline[0].at(0)) &&
            cmdline[0].at(1) == ':' &&
            (cmdline[0].at(2) == '/' || cmdline[0].at(2) == '\\')) {
        return QFileInfo(cmdline[0]).fileName();
    }

    for (auto cmd : cmdline) {
        QString name = {basename(cmd)};
        if (name.startsWith(proc_name.toLocal8Bit())) {
            return name;
        }
    }
    return proc_name;
}

int getStatusBarMaxWidth();
bool startWithHanzi(const QString &text);

QMap<QString, QString> getProcessDescriptions();
QMap<QString, QString> getDesktopfileMap();
QPixmap getDesktopFileIcon(std::string desktopFile, int iconSize = 24);
QSize getRenderSize(int fontSize, QString string);

QString getProcessCmdline(pid_t pid);
QString getProcessEnvironmentVariable(pid_t pid, QString environmentName);
QString getProcessNameFromCmdLine(const pid_t pid);
QString getQrcPath(QString imageName);
QString getFlatpakAppIcon(QString flatpakAppid);
QDir getFlatpakAppPath(QString flatpakAppid);
bool fileExists(QString path);
void drawLoadingRing(QPainter &painter, int centerX, int centerY, int radius, int penWidth,
                     int loadingAngle, int rotationAngle, QColor foregroundColor,
                     double foregroundOpacity, QColor backgroundColor, double backgroundOpacity,
                     double percent);
void drawRing(QPainter &painter, int centerX, int centerY, int radius, int penWidth,
              int loadingAngle, int rotationAngle, QColor color, double opacity);
void drawTooltipBackground(QPainter &painter, QRect rect, qreal opacity = 0.4);
void drawTooltipText(QPainter &painter, QString text, QString textColor, int textSize, QRectF rect);
void setFontSize(QPainter &painter, int textSize);
}  // namespace Utils

#endif // UTILS_H
