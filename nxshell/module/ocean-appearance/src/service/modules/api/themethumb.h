// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THEMETHUMB_H
#define THEMETHUMB_H

#include <QDir>
#include <QGSettings>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QProcess>

#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

const int gtkVersion = 0;
const int iconVersion = 1;
const int cursorVersion = 1;
QString getScaleDir();
QString getTypeDir(QString type0, int version);
void init(double scalFactor0);
void removeUnusedScaleDirs();
void removeAllTypesOldVersionDirs();
void removeOldVersionDirs(QString scaleDir, QString type0, int version);
void removeUnusedDirs(QString pattern, QString userDir);
bool checkScaleFactor();
double getScaleFactor();
QString getCursor(QString id, QString descFile);
bool genCursor(QString descFile, int width, int height, double scaleFactor, QString out);
bool genGtk(QString descFile, int width, int height, double scaleFactor, QString dest);
QImage CompositeImages(QVector<QImage *>, int width, int height, int inconSize, int padding);
QImage *loadXCursor(QString fileName, int size);
QList<QIcon> getIcons(QString theme, int size);
QVector<QImage *> getCursors(QString dir, int size);
QString getGlobal(QString id, QString descFile,QString gtkTheme);
QString getGtk(QString id, QString descFile);
QString getIcon(QString id, QString descFile);
bool shouldGenerateNew(QString descFile, QString out);
bool shouldGenerateNewCursor(QString descFile, QString out);
QString prepareOutputPath(QString type0, QString id, int version);

void UpdateScaleFactor(double scaleFactor);

#endif // THEMETHUMB_H
