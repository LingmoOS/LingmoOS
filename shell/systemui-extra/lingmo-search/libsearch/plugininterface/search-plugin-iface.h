/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#ifndef SEARCHPLUGINIFACE_H
#define SEARCHPLUGINIFACE_H
#define SearchPluginIface_iid "org.lingmo.lingmo-search.plugin-iface.SearchPluginInterface"
/**
 * changelog：1.3.0 增加resourceType
 * changelog:1.2.0 增加toolTip
 */
#define SEARCH_PLUGIN_IFACE_VERSION "1.3.0"

#include <QString>
#include <QIcon>
#include <QList>
#include <QMutex>
#include <QtPlugin>
#include "plugin-iface.h"
#include "data-queue.h"

namespace LingmoUISearch {
class SearchPluginIface : public PluginInterface
{
public:
    enum InvokableAction
    {
        None                     = 1u << 0,
        HideUI                   = 1u << 1
    };
    Q_DECLARE_FLAGS(InvokableActions, InvokableAction)

    struct DescriptionInfo
    {
        QString key;
        QString value;
    };
    struct Actioninfo
    {
        int actionkey;
        QString displayName;
    };
    /**
     * @brief The ResultInfo struct
     */
    struct ResultInfo
    {
        QIcon icon; //要显示的图标
        QString name; //要显示的名称
        QString toolTip; //要先显示的tooltip
        QVector<DescriptionInfo> description; //详情页中使用的描述
        QString actionKey; //执行action时的key
        QString resourceType; //资源类型，可定义，例如“file, app”
        int type;
        explicit ResultInfo(const QIcon &iconToSet = QIcon(),
                   const QString &nameToSet = QString(),
                   const QString &toolTipToSet = QString(),
                   const QString &resourceTypeToSet = QString(),
                   const QVector<DescriptionInfo> &descriptionToSet = QVector<DescriptionInfo>(),
                   const QString &actionKeyToSet = QString(),
                   const int &typeToSet = 0) {
            icon = iconToSet;
            name = nameToSet;
            toolTip = toolTipToSet;
            resourceType = resourceTypeToSet;
            description = descriptionToSet;
            actionKey = actionKeyToSet;
            type = typeToSet;
        }
        ~ResultInfo() {
            description.clear();
            description.squeeze();
        }
    };

    virtual ~SearchPluginIface() {}
    virtual QString getPluginName() = 0;
    virtual void KeywordSearch(QString keyword,DataQueue<ResultInfo> *searchResult) = 0;
    virtual void stopSearch() = 0;
    virtual QList<Actioninfo> getActioninfo(int type) = 0;
    virtual void openAction(int actionkey, QString key, int type) = 0;
//    virtual bool isPreviewEnable(QString key, int type) = 0;
//    virtual QWidget *previewPage(QString key, int type, QWidget *parent = nullptr) = 0;
    virtual QWidget *detailPage(const ResultInfo &ri) = 0;

    void invokeActions(InvokableActions actions);
};
}
Q_DECLARE_INTERFACE(LingmoUISearch::SearchPluginIface, SearchPluginIface_iid)

#endif // SEARCHPLUGINIFACE_H
