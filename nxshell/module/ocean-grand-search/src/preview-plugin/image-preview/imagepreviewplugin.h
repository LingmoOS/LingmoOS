// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPREVIEWPLUGIN_H
#define IMAGEPREVIEWPLUGIN_H

#include "previewplugin.h"

namespace GrandSearch {
namespace image_preview {

class ImageView;
class ImagePreviewPlugin : public QObject, public PreviewPlugin
{
    Q_OBJECT
public:
    explicit ImagePreviewPlugin(QObject *parent = nullptr);
    ~ImagePreviewPlugin() Q_DECL_OVERRIDE;
    void init(QObject *proxyInter) Q_DECL_OVERRIDE;
    bool previewItem(const ItemInfo &item) Q_DECL_OVERRIDE;
    ItemInfo item() const Q_DECL_OVERRIDE;
    QWidget *contentWidget() const Q_DECL_OVERRIDE;
    bool stopPreview() Q_DECL_OVERRIDE;
    DetailInfoList getAttributeDetailInfo() const Q_DECL_OVERRIDE;
    QWidget *toolBarWidget() const Q_DECL_OVERRIDE;
    bool showToolBar() const Q_DECL_OVERRIDE;
private:
    ItemInfo m_item;
    ImageView *m_imageView = nullptr;
    DetailInfoList m_detailInfos;
};

}}

#endif // IMAGEPREVIEWPLUGIN_H
