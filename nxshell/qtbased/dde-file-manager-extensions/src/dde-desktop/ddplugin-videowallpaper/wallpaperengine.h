// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WALLPAPERENGINE_H
#define WALLPAPERENGINE_H

#include "ddplugin_videowallpaper_global.h"

#include <QObject>

namespace ddplugin_videowallpaper {

class WallpaperEnginePrivate;
class WallpaperEngine : public QObject
{
    Q_OBJECT
    friend class WallpaperEnginePrivate;
public:
    explicit WallpaperEngine(QObject *parent = nullptr);
    ~WallpaperEngine() override;
    bool init();
    void turnOn(bool build = true);
    void turnOff();
signals:

public slots:
    void refreshSource();
    void build();
    void onDetachWindows();
    void geometryChanged();
    void play();
    void show();
private slots:
    bool registerMenu();
    void checkResouce();
#ifndef USE_LIBDMR
    void catchImage(const QImage &img);
#endif
private:
    WallpaperEnginePrivate *d;
};

}

#endif // WALLPAPERENGINE_H
