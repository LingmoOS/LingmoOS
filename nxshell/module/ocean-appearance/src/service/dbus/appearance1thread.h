// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPEARANCE1THREAD_H
#define APPEARANCE1THREAD_H

#include "scaleFactors.h"

#include <QMutex>
#include <QObject>

class AppearanceProperty;
class QDBusMessage;
class AppearanceManager;
class Appearance1Thread : public QObject
{
    Q_OBJECT
public:
    explicit Appearance1Thread();
    ~Appearance1Thread();

public: // PROPERTIES
    Q_PROPERTY(QString Background READ background)
    QString background() const;

    Q_PROPERTY(QString CursorTheme READ cursorTheme)
    QString cursorTheme() const;

    Q_PROPERTY(double FontSize READ fontSize WRITE setFontSize)
    double fontSize() const;

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

    Q_PROPERTY(QString QtActiveColor READ qtActiveColor WRITE setQtActiveColor)
    QString qtActiveColor() const;

    Q_PROPERTY(QString StandardFont READ standardFont)
    QString standardFont() const;

    Q_PROPERTY(QString WallpaperSlideShow READ wallpaperSlideShow WRITE setWallpaperSlideShow)
    QString wallpaperSlideShow() const;

    Q_PROPERTY(QString WallpaperURls READ wallpaperURls)
    QString wallpaperURls() const;

    Q_PROPERTY(int WindowRadius READ windowRadius WRITE setWindowRadius)
    int windowRadius() const;

    Q_PROPERTY(int DTKSizeMode READ dtkSizeMode WRITE setDTKSizeMode)
    int dtkSizeMode() const;

    Q_PROPERTY(int qtScrollBarPolicy READ qtScrollBarPolicy WRITE setQtScrollBarPolicy)
    int qtScrollBarPolicy() const;

public Q_SLOTS:
    // PROPERTIES
    void setFontSize(double value);
    void setOpacity(double value);
    void setQtActiveColor(const QString &value);
    void setWallpaperSlideShow(const QString &value);
    void setWindowRadius(int value);
    void setDTKSizeMode(int value);
    void setQtScrollBarPolicy(int value);
    // METHODS
    void Delete(const QString &ty, const QString &name, const QDBusMessage &message);
    QString GetCurrentWorkspaceBackground(const QDBusMessage &message);
    QString GetCurrentWorkspaceBackgroundForMonitor(const QString &strMonitorName, const QDBusMessage &message);
    double GetScaleFactor(const QDBusMessage &message);
    ScaleFactors GetScreenScaleFactors(const QDBusMessage &message);
    QString GetActiveColors(const QDBusMessage &message);
    QString GetWallpaperSlideShow(const QString &monitorName, const QDBusMessage &message);
    QString GetWorkspaceBackgroundForMonitor(const int &index, const QString &strMonitorName, const QDBusMessage &message);
    QString List(const QString &ty, const QDBusMessage &message);
    void Reset(const QDBusMessage &message);
    void Set(const QString &ty, const QString &value, const QDBusMessage &message);
    void SetCurrentWorkspaceBackground(const QString &uri, const QDBusMessage &message);
    void SetCurrentWorkspaceBackgroundForMonitor(const QString &uri, const QString &strMonitorName, const QDBusMessage &message);
    void SetMonitorBackground(const QString &monitorName, const QString &imageGile, const QDBusMessage &message);
    void SetScaleFactor(double scale, const QDBusMessage &message);
    void SetScreenScaleFactors(ScaleFactors scaleFactors, const QDBusMessage &message);
    void SetActiveColors(const QString &activeColors, const QDBusMessage &message);
    void SetWallpaperSlideShow(const QString &monitorName, const QString &slideShow, const QDBusMessage &message);
    void SetWorkspaceBackgroundForMonitor(const int &index, const QString &strMonitorName, const QString &uri, const QDBusMessage &message);
    QString Show(const QString &ty, const QStringList &names, const QDBusMessage &message);
    QString Thumbnail(const QString &ty, const QString &name, const QDBusMessage &message);

Q_SIGNALS: // SIGNALS
    void Changed(const QString &ty, const QString &value);
    void Refreshed(const QString &type);

private Q_SLOTS:
    void init();

private:
    QScopedPointer<AppearanceManager> appearanceManager;
    AppearanceProperty *property;
    QThread *thread;
    QMutex mutex;
};

#endif // APPEARANCE1THREAD_H
