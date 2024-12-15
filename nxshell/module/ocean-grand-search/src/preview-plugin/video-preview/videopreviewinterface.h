// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOPREVIEWINTERFACE_H
#define VIDEOPREVIEWINTERFACE_H

#include <previewplugininterface.h>

namespace GrandSearch {
namespace video_preview {

class VideoPreviewInterface : public QObject, public PreviewPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(GrandSearch::PreviewPluginInterface)
    Q_PLUGIN_METADATA(IID FilePreviewInterface_iid)
public:
    explicit VideoPreviewInterface(QObject *parent = nullptr);
    virtual PreviewPlugin *create(const QString &mimetype);
};

}}
#endif // VIDEOPREVIEWINTERFACE_H
