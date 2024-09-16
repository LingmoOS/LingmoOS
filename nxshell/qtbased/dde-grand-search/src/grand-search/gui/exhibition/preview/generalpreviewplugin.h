// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GENERALPREVIEWPLUGIN_H
#define GENERALPREVIEWPLUGIN_H

#include "../../datadefine.h"
#include "global/matcheditem.h"
#include "global/builtinsearch.h"
#include "previewplugin.h"

#include <QObject>
#include <QScopedPointer>

class QFrame;

namespace GrandSearch {

class GeneralPreviewPluginPrivate;
class GeneralPreviewPlugin : public QObject, public PreviewPlugin
{
    Q_OBJECT
public:
    explicit GeneralPreviewPlugin(QObject *parent = nullptr);
    ~GeneralPreviewPlugin() override;
    void init(QObject *proxyInter) Q_DECL_OVERRIDE;
    virtual bool previewItem(const ItemInfo &info) override;
    virtual ItemInfo item() const override;
    bool stopPreview() override;
    virtual QWidget *contentWidget() const override;
    virtual DetailInfoList getAttributeDetailInfo() const override;
    virtual QWidget *toolBarWidget() const override;
    bool showToolBar() const override;
private slots:
    void updateFolderSize(qint64 size);
private:
    QScopedPointer<GeneralPreviewPluginPrivate> d_p;
};

}

#endif // GENERALPREVIEWPLUGIN_H
