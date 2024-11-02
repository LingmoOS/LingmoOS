/*
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
 */
#ifndef WEBSEARCHPLUGIN_H
#define WEBSEARCHPLUGIN_H

#include <QObject>
#include <QObject>
#include <QThreadPool>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QAction>
#include "action-label.h"
#include "search-plugin-iface.h"
#include <gio/gdesktopappinfo.h>
#include <QGSettings>
#include "icon-loader.h"

namespace LingmoUISearch {

class LIBSEARCH_EXPORT WebSearchPlugin : public QObject, public SearchPluginIface
{
    Q_OBJECT

public:
    explicit WebSearchPlugin(QObject *parent = nullptr);
    PluginType pluginType() {return PluginType::SearchPlugin;}
    const QString name();
    const QString description();
    const QIcon icon() {return IconLoader::loadIconQt("folder");}
    void setEnable(bool enable) {m_enable = enable;}
    bool isEnable() {return m_enable;}
    QString getPluginName();
    void KeywordSearch(QString keyword,DataQueue<ResultInfo> *searchResult);
    void stopSearch();
    QList<SearchPluginIface::Actioninfo> getActioninfo(int type);
    void openAction(int actionkey, QString key, int type);
    QWidget *detailPage(const ResultInfo &ri);

private:
    void initDetailPage();
    QString m_keyWord;
    QString m_currentActionKey;
    QWidget *m_detailPage = nullptr;
    QVBoxLayout *m_detailLyt = nullptr;
    QLabel *m_iconLabel = nullptr;
    QFrame *m_actionFrame = nullptr;
    QVBoxLayout *m_actionFrameLyt = nullptr;
    ActionLabel *m_actionLabel1 = nullptr;

    QVBoxLayout * m_actionLyt = nullptr;

    QGSettings *m_settings = nullptr;
    QString m_webEngine;

    bool m_enable = true;
    QList<WebSearchPlugin::Actioninfo> m_actionInfo;

    QString getDefaultAppId(const char * contentType);
};
}
#endif // WEBSEARCHPLUGIN_H
