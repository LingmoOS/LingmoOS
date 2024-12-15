// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALCONTROL_H
#define GLOBALCONTROL_H

#include "imagedata/imageinfo.h"
#include "imagedata/imagesourcemodel.h"
#include "imagedata/pathviewproxymodel.h"
#include "types.h"

#include <QObject>
#include <QUrl>
#include <QBasicTimer>

class GlobalControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ImageSourceModel *globalModel READ globalModel CONSTANT)
    Q_PROPERTY(PathViewProxyModel *viewModel READ viewModel CONSTANT)
    Q_PROPERTY(QUrl currentSource READ currentSource WRITE setCurrentSource NOTIFY currentSourceChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int currentFrameIndex READ currentFrameIndex WRITE setCurrentFrameIndex NOTIFY currentFrameIndexChanged)
    Q_PROPERTY(int imageCount READ imageCount NOTIFY imageCountChanged)
    Q_PROPERTY(int currentRotation READ currentRotation WRITE setCurrentRotation NOTIFY currentRotationChanged)
    Q_PROPERTY(bool hasPreviousImage READ hasPreviousImage NOTIFY hasPreviousImageChanged)
    Q_PROPERTY(bool hasNextImage READ hasNextImage NOTIFY hasNextImageChanged)

public:
    explicit GlobalControl(QObject *parent = nullptr);
    ~GlobalControl() override;

    // 用于全局的图像源数据模型
    ImageSourceModel *globalModel() const;

    // 用于大图展示界面的数据
    PathViewProxyModel *viewModel() const;

    // 当前图像设置及(帧)索引变更信号
    void setCurrentSource(const QUrl &source);
    QUrl currentSource() const;
    Q_SIGNAL void currentSourceChanged();
    void setCurrentIndex(int index);
    int currentIndex() const;
    Q_SIGNAL void currentIndexChanged();
    void setCurrentFrameIndex(int frameIndex);
    int currentFrameIndex() const;
    Q_SIGNAL void currentFrameIndexChanged();
    int imageCount() const;
    Q_SIGNAL void imageCountChanged();

    // index 和 frameIndex 同时变更时必须一同设置
    Q_INVOKABLE void setIndexAndFrameIndex(int index, int frameIndex);

    // 图像旋转处理
    void setCurrentRotation(int angle);
    int currentRotation();
    Q_SIGNAL void changeRotationCacheBegin();
    Q_SIGNAL void currentRotationChanged();
    Q_SIGNAL void requestRotateCacheImage();

    // 图片切换操作
    bool hasPreviousImage() const;
    Q_SIGNAL void hasPreviousImageChanged();
    bool hasNextImage() const;
    Q_SIGNAL void hasNextImageChanged();
    Q_INVOKABLE bool previousImage();
    Q_INVOKABLE bool nextImage();
    Q_INVOKABLE bool firstImage();
    Q_INVOKABLE bool lastImage();
    Q_INVOKABLE void forceExit();

    // 图像文件变更操作
    Q_SLOT void setImageFiles(const QStringList &imageFiles, const QString &openFile);
    Q_SLOT void removeImage(const QUrl &removeImage);
    Q_SLOT void renameImage(const QUrl &oldName, const QUrl &newName);

    Q_SLOT void submitImageChangeImmediately();
    Q_SIGNAL void requestRotateImage(const QString &localPath, int rotation);

    // 判断当前设备是否支持多线程处理
    static bool enableMultiThread();

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    void checkSwitchEnable();

private:
    int curIndex = 0;
    int curFrameIndex = 0;
    ImageInfo currentImage;

    ImageSourceModel *sourceModel { nullptr };
    PathViewProxyModel *viewSourceModel { nullptr };
    bool hasPrevious = false;
    bool hasNext = false;

    int imageRotation = 0;    // 当前图片旋转角度
    QBasicTimer submitTimer;  // 图片变更提交定时器
};

#endif  // GLOBALCONTROL_H
