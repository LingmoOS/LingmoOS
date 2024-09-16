// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONSERVICE_H
#define COMMONSERVICE_H

#include "image-viewer_global.h"

#include <QObject>
#include <QStandardPaths>
#include <QDir>
#include <QMap>
#include <QMutex>
#include <QSet>

class LibCommonService : public QObject
{
    Q_OBJECT
public:
    static LibCommonService *instance();

    //设置图片展示类型，看图，相册
    void setImgViewerType(imageViewerSpace::ImgViewerType type);
    imageViewerSpace::ImgViewerType getImgViewerType();
    //设置缩略图保存路径
    void setImgSavePath(QString path);
    QString getImgSavePath();

    imageViewerSpace::ItemInfo getImgInfoByPath(QString path);
    //不发数据更新信号的保存信息
    //void setImgInfoByPat(QString path, imageViewerSpace::ItemInfo itemInfo);

    //重命名更新缓存
    void reName(const QString &oldPath, const QString &newPath);

public:
    QStringList m_listAllPath;//全部需要加载的
    QStringList m_noLoadingPath;//没有加载的地址
    QSet<QString> m_listLoaded;//已经加载的

signals:
    void sigRightMousePress();
public slots:
    //有新的图片加载上来,保存制作好的图片信息
    void slotSetImgInfoByPath(QString path, imageViewerSpace::ItemInfo itemInfo);
private:
    explicit LibCommonService(QObject *parent = nullptr);
    bool eventFilter(QObject *obj, QEvent *event);
private:
    static LibCommonService *m_commonService;
    QMutex m_mutex;
    imageViewerSpace::ImgViewerType m_imgViewerType = imageViewerSpace::ImgViewerTypeNull;
    QString       m_imgSavePath;
    QMap<QString, imageViewerSpace::ItemInfo> m_allInfoMap;//图片所有信息map


};

#endif // COMMONSERVICE_H
