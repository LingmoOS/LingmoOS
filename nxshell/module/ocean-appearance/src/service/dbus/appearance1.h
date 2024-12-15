// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPEARANCE1_H
#define APPEARANCE1_H

#include "scaleFactors.h"
#include <qdbuscontext.h>

#include <QObject>

class QDBusMessage;
class Appearance1Thread;
class Appearance1 : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    Appearance1(QObject *parent = nullptr);
    ~Appearance1();

public: // PROPERTIES
    Q_PROPERTY(QString Background READ background)
    QString background() const;

    Q_PROPERTY(QString CursorTheme READ cursorTheme)
    QString cursorTheme() const;

    Q_PROPERTY(double FontSize READ fontSize WRITE setFontSize)
    double fontSize() const;
    void setFontSize(double value);

    Q_PROPERTY(QString GlobalTheme READ globalTheme)
    QString globalTheme() const;

    Q_PROPERTY(QString GtkTheme READ gtkTheme)
    QString gtkTheme() const;

    Q_PROPERTY(QString IconTheme READ iconTheme)
    QString iconTheme() const;

    Q_PROPERTY(QString MonospaceFont READ monospaceFont)
    QString monospaceFont() const;

    Q_PROPERTY(double Opacity READ opacity WRITE setOpacity)
    double opacity() const;
    void setOpacity(double value);

    Q_PROPERTY(QString QtActiveColor READ qtActiveColor WRITE setQtActiveColor)
    QString qtActiveColor() const;
    void setQtActiveColor(const QString &value);

    Q_PROPERTY(QString StandardFont READ standardFont)
    QString standardFont() const;

    Q_PROPERTY(QString WallpaperSlideShow READ wallpaperSlideShow WRITE setWallpaperSlideShow)
    QString wallpaperSlideShow() const;
    void setWallpaperSlideShow(const QString &value);

    Q_PROPERTY(QString WallpaperURls READ wallpaperURls)
    QString wallpaperURls() const;

    Q_PROPERTY(int WindowRadius READ windowRadius WRITE setWindowRadius)
    int windowRadius() const;
    void setWindowRadius(int value);

    Q_PROPERTY(int DTKSizeMode READ dtkSizeMode WRITE setDTKSizeMode)
    int dtkSizeMode() const;
    void setDTKSizeMode(int value);

    Q_PROPERTY(int QtScrollBarPolicy READ qtScrollBarPolicy WRITE setQtScrollBarPolicy)
    int qtScrollBarPolicy() const;
    void setQtScrollBarPolicy(int value);

public Q_SLOTS: // METHODS
    void Delete(const QString &ty, const QString &name);
    QString GetCurrentWorkspaceBackground();
    QString GetCurrentWorkspaceBackgroundForMonitor(const QString &strMonitorName);
    double GetScaleFactor();
    ScaleFactors GetScreenScaleFactors();
    QString GetActiveColors();
    QString GetWallpaperSlideShow(const QString &monitorName);
    QString GetWorkspaceBackgroundForMonitor(const int &index, const QString &strMonitorName);
    QString List(const QString &ty);
    void Reset();
    void Set(const QString &ty, const QString &value);
    void SetCurrentWorkspaceBackground(const QString &uri);
    void SetCurrentWorkspaceBackgroundForMonitor(const QString &uri, const QString &strMonitorName);
    void SetMonitorBackground(const QString &monitorName, const QString &imageGile);
    void SetScaleFactor(double scale);
    void SetScreenScaleFactors(ScaleFactors scaleFactors);
    void SetActiveColors(const QString &activeColors);
    void SetWallpaperSlideShow(const QString &monitorName, const QString &slideShow);
    void SetWorkspaceBackgroundForMonitor(const int &index, const QString &strMonitorName, const QString &uri);
    QString Show(const QString &ty, const QStringList &names);
    QString Thumbnail(const QString &ty, const QString &name);
Q_SIGNALS: // SIGNALS
    void Changed(const QString &ty, const QString &value);
    void Refreshed(const QString &type);

public:
    QScopedPointer<Appearance1Thread> appearance1Thread;
};

#endif
