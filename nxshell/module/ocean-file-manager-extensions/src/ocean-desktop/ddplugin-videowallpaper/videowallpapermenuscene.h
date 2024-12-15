// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOWALLPAERMENUSCENE_H
#define VIDEOWALLPAERMENUSCENE_H

#include "dfm-base/interfaces/abstractmenuscene.h"
#include "dfm-base/interfaces/abstractscenecreator.h"

#include <QMap>

namespace ddplugin_videowallpaper {

namespace ActionID {
inline constexpr char kVideoWallpaper[] = "video-wallpaper";
}

class VideoWallpaerMenuCreator : public DFMBASE_NAMESPACE::AbstractSceneCreator
{
    Q_OBJECT
public:
    static QString name()
    {
        return "VideoWallpaperMenu";
    }
    DFMBASE_NAMESPACE::AbstractMenuScene *create() override;
};

class VideoWallpaperMenuScene : public DFMBASE_NAMESPACE::AbstractMenuScene
{
    Q_OBJECT
public:
    explicit VideoWallpaperMenuScene(QObject *parent = nullptr);
    QString name() const override;
    bool initialize(const QVariantHash &params) override;
    AbstractMenuScene *scene(QAction *action) const override;
    bool create(QMenu *parent) override;
    void updateState(QMenu *parent) override;
    bool triggered(QAction *action) override;
private:
    bool turnOn = false;
    bool onDesktop = false;
    bool isEmptyArea = false;
    QMap<QString, QAction *> predicateAction;   // id -- instance
    QMap<QString, QString> predicateName;   // id -- text
};
}

#endif // VIDEOWALLPAERMENUSCENE_H
