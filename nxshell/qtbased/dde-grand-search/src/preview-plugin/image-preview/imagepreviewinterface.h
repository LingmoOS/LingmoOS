// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPREVIEWINTERFACE_H
#define IMAGEPREVIEWINTERFACE_H

#include "previewplugininterface.h"

namespace GrandSearch {
namespace image_preview {

class ImagePreviewInterface : public QObject, public PreviewPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(GrandSearch::PreviewPluginInterface)
    Q_PLUGIN_METADATA(IID FilePreviewInterface_iid)
public:
    explicit ImagePreviewInterface(QObject *parent = nullptr);
    virtual PreviewPlugin *create(const QString &mimetype);
};

}}

#endif // IMAGEPREVIEWINTERFACE_H
