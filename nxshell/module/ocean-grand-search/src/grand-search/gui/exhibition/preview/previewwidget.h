// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include "previewpluginmanager.h"
#include <DWidget>

#include <QScopedPointer>

class QVBoxLayout;
class QSpacerItem;

namespace GrandSearch {

class GeneralToolBar;
class AiToolBar;
class DetailWidget;
class PluginProxy;
class PreviewWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
    friend class PluginProxy;
public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget() override;

    // 预览指定搜索项
    bool previewItem(const MatchedItem &item);

private:
    void initUi();
    void initConnect();
    void reLayout();
    void clearLayoutWidgets();

    void onOpenClicked();
    void onOpenpathClicked();
    void onCopypathClicked();
private:
    QVBoxLayout *m_vMainLayout = nullptr;               // 预览界面主体布局

    QSpacerItem* m_vSpaceItem = nullptr;       // 垂直弹簧，用于将工具栏置底显示

    QSharedPointer<PreviewPlugin> m_preview = nullptr;        // 当前正在预览的插件界面

    QSharedPointer<PreviewPlugin> m_generalPreview = nullptr; // 默认预览插件界面

    DetailWidget* m_detailInfoWidget = nullptr; // 通用属性详情部件
    GeneralToolBar *m_generalToolBar = nullptr;     // 通用工具栏部件
    AiToolBar *m_aiToolBar = nullptr;     // AI工具栏部件
    PluginProxy *m_proxy = nullptr; //用于预览插件回调预览框架的接口


private:
    MatchedItem m_item; //当前正在预览的匹配结果
    PreviewPluginManager m_pluginManager; //预览插件管理对象
};

}

#endif // PREVIEWWIDGET_H
