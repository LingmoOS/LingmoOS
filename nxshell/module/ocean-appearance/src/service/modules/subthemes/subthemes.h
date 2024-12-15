// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SUBTHEMES_
#define SUBTHEMES_

#include "scanner.h"
#include "theme.h"
#include "../api/themes.h"

#include <QObject>
#include <vector>
#include <DConfig>

class QTimer;
class QThread;
class AppearanceManager;

class Subthemes : public QObject
{
    Q_OBJECT
public:
    Subthemes(AppearanceManager *parent = nullptr);
    virtual ~Subthemes();

    void refreshGtkThemes();
    void refreshIconThemes();
    void refreshCursorThemes();
    void refreshGlobalThemes();
    QVector<QSharedPointer<Theme>> listGtkThemes();
    QVector<QSharedPointer<Theme>> listIconThemes();
    QVector<QSharedPointer<Theme>> listCursorThemes();
    QVector<QSharedPointer<Theme>> listGlobalThemes();
    bool deleteGtkTheme(const QString& name);
    bool deleteIconTheme(const QString& name);
    bool deleteCursorTheme(const QString& name);
    bool isGlobalTheme(QString id);
    bool isGtkTheme(QString id);
    bool isIconTheme(QString id);
    bool isCursorTheme(QString id);
    bool setGlobalTheme(QString id);
    bool setGtkTheme(QString id);
    bool setIconTheme(QString id);
    bool setCursorTheme(QString id);
    QString getGlobalThumbnail(QString id);
    QString getGtkThumbnail(QString id);
    QString getIconThumbnail(QString id);
    QString getCursorThumbnail(QString id);
    QVector<QSharedPointer<Theme>> getThemes(QVector<QString> files);
    bool isDeletable(QString file);
    bool isItemInList(QString item, QVector<QString> lists);
    QString getBasePath(QString filename);
    QMap<QString,QString>& getGtkThumbnailMap();

private Q_SLOTS:
    void init();

private:
    QSharedPointer<ThemesApi>          themeApi;
    QVector<QSharedPointer<Theme>>     gtkThemes;
    QVector<QSharedPointer<Theme>>     iconThemes;
    QVector<QSharedPointer<Theme>>     cursorThemes;
    QVector<QSharedPointer<Theme>>     globalThemes;
    QMap<QString,QString>              gtkThumbnailMap;
    int                                initStatus;
    QTimer                             *timer;
    QStringList                        thumbnailThemes;
};

#endif
