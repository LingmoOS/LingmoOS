// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOPREVIEWPLUGIN_H
#define VIDEOPREVIEWPLUGIN_H

#include <previewplugin.h>

#include <QFuture>

namespace GrandSearch {
namespace video_preview {

class DecodeBridge : public QObject
{
    Q_OBJECT
public:
    DecodeBridge();
    ~DecodeBridge();
    static QVariantHash decode(QSharedPointer<DecodeBridge> self, const QString &file);
    static QPixmap scaleAndRound(const QImage &img, const QSize &size);
signals:
    void sigUpdateInfo(const QVariantHash &, bool needUpdate);
public:
    volatile bool decoding = false;
};

class VideoView;
class VideoPreviewPlugin : public QObject, public PreviewPlugin
{
    Q_OBJECT
public:
    explicit VideoPreviewPlugin(QObject *parent = nullptr);
    ~VideoPreviewPlugin() Q_DECL_OVERRIDE;
    void init(QObject *proxyInter) Q_DECL_OVERRIDE;
    bool previewItem(const ItemInfo &item) Q_DECL_OVERRIDE;
    ItemInfo item() const Q_DECL_OVERRIDE;
    QWidget *contentWidget() const Q_DECL_OVERRIDE;
    bool stopPreview() Q_DECL_OVERRIDE;
    QWidget *toolBarWidget() const Q_DECL_OVERRIDE;
    bool showToolBar() const Q_DECL_OVERRIDE;
    DetailInfoList getAttributeDetailInfo() const Q_DECL_OVERRIDE;
protected slots:
    void updateInfo(const QVariantHash &, bool needUpdate);
protected:
    ItemInfo m_item;
    DetailInfoList m_infos;
    VideoView *m_view = nullptr;
    QObject *m_proxy = nullptr;
    QSharedPointer<DecodeBridge> m_decode;
};

}}

#endif // VIDEOPREVIEWPLUGIN_H
