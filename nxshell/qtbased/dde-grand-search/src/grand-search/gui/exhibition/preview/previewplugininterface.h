// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWPLUGINIFACE_H
#define PREVIEWPLUGINIFACE_H

#include <QObject>

namespace GrandSearch {
#define FilePreviewInterface_iid "com.deepin.grandsearch.FilePreviewInterface.iid"

class PreviewPlugin;
class PreviewPluginInterface
{
public:
    virtual PreviewPlugin *create(const QString &mimetype) = 0;
};

}

Q_DECLARE_INTERFACE(GrandSearch::PreviewPluginInterface, FilePreviewInterface_iid)
#endif // PREVIEWPLUGINIFACE_H
