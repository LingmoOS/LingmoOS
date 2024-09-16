// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINPROXY_H
#define PLUGINPROXY_H

#include "previewwidget.h"
#include "previewplugin.h"

#include <QObject>

namespace GrandSearch {

class PluginProxy : public QObject
{
    Q_OBJECT
public:
    explicit PluginProxy(PreviewWidget *parent);

signals:

public slots:
    void updateDetailInfo(PreviewPlugin *plugin);
private:
    PreviewWidget *q = nullptr;
};

}

#endif // PLUGINPROXY_H
