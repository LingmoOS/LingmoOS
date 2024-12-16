// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCANNER_H
#define SCANNER_H

#include <QObject>
#include <QUrl>
#include <QDir>
#include <gio/gio.h>
#include <functional>

#include "../api/keyfile.h"
#include "../api/dfile.h"


class Scanner : public QObject
{
    Q_OBJECT
public:
    typedef std::function<bool (QString)> Fn;

    Scanner(QObject *parent = nullptr);
    ~Scanner();
    QString query(QString uri);
    QString queryThemeMime(QString file);
    QString doQueryFile(QString file);
    bool globalTheme(QString file);
    bool gtkTheme(QString file);
    bool iconTheme(QString file);
    bool cursorTheme(QString file);
    bool isGlobalTheme(QString uri);
    bool isGtkTheme(QString uri);
    bool isIconTheme(QString uri);
    bool isCursorTheme(QString uri);
    QVector<QString> listGlobalTheme(QString uri);
    QVector<QString> listGtkTheme(QString uri);
    QVector<QString> listIconTheme(QString uri);
    QVector<QString> listCursorTheme(QString uri);
    QVector<QString> listSubDir(QString path);
    QVector<QString> doListTheme(QString uri, QString ty,Fn fn);
    bool isHidden(QString file, QString ty);
};

#endif // SCANNER_H
