// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PERFORMANCEMONITOR_H
#define PERFORMANCEMONITOR_H

#include <QObject>

class PerformanceMonitor : public QObject
{
    Q_OBJECT
public:
    explicit PerformanceMonitor(QObject *parent = nullptr);

    static void initializeAppStart();
    static void initializeAppFinish();

    static void loadFontStart();
    static void loadFontFinish();

    static qint64 calcInitAndLoadTime();

    static void installFontStart();
    static void installFontFinish(int fontCount);

    static void exportFontStart();
    static void exportFontFinish(int fontCount);

    static void deleteFontStart();
    static void deleteFontFinish(int fontCount);

    static void activeFontStart();
    static void activeFontFinish(bool isDeActive, int fontCount);

    static void favoriteFontStart();
    static void favoriteFontFinish(bool isFavorite, int fontCount);

private:
    Q_DISABLE_COPY(PerformanceMonitor)

    static qint64 initializeAppStartMs;
    static qint64 initializeAppFinishMs;
    static qint64 loadFontStartMs;
    static qint64 loadFontFinishMs;
    static qint64 installFontStartMs;
    static qint64 installFontFinishMs;
    static qint64 exportFontStartMs;
    static qint64 exportFontFinishMs;
    static qint64 deleteFontStartMs;
    static qint64 deleteFontFinishMs;
    static qint64 favoriteFontStartMs;
    static qint64 favoriteFontFinishMs;
    static qint64 activeFontStartMs;
    static qint64 activeFontFinishMs;

};

#endif // PERFORMANCEMONITOR_H
