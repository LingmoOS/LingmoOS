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
#ifndef SETTINGSSEARCHPLUGIN_H
#define SETTINGSSEARCHPLUGIN_H

#include <QObject>
#include <QThreadPool>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QAction>
#include <QDomDocument>
#include <QDBusInterface>

#include "action-label.h"
#include "separation-line.h"
#include "search-plugin-iface.h"
#include "icon-loader.h"

namespace LingmoUISearch {
class LIBSEARCH_EXPORT SettingsSearchPlugin : public QObject, public SearchPluginIface
{
    Q_OBJECT
    friend class SettingsSearch;
    friend class SettingsMatch;
public:
    SettingsSearchPlugin(QObject *parent = nullptr);
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
//    bool isPreviewEnable(QString key, int type);
//    QWidget *previewPage(QString key, int type, QWidget *parent);
    QWidget *detailPage(const ResultInfo &ri);

private:
    void initDetailPage();
    bool m_enable = true;
    QList<SettingsSearchPlugin::Actioninfo> m_actionInfo;
    QThreadPool m_pool;
    static size_t m_uniqueSymbolForSettings;
    static QMutex m_mutex;

    QString m_currentActionKey;
    QWidget *m_detailPage = nullptr;
    QVBoxLayout *m_detailLyt = nullptr;
    QLabel *m_iconLabel = nullptr;
    QFrame *m_nameFrame = nullptr;
    QHBoxLayout *m_nameFrameLyt = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_pluginLabel = nullptr;
    SeparationLine *m_line_1 = nullptr;
    QFrame *m_actionFrame = nullptr;
    QVBoxLayout *m_actionFrameLyt = nullptr;
    ActionLabel *m_actionLabel1 = nullptr;
    QVBoxLayout * m_actionLyt = nullptr;
};

class SettingsSearch :public QObject, public QRunnable {
    Q_OBJECT
public:
    SettingsSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, const QString& keyword, size_t uniqueSymbol);
    ~SettingsSearch() = default;
protected:
    void run() override;
private:
    QString m_keyword;
    size_t m_uniqueSymbol;
    DataQueue<SearchPluginIface::ResultInfo> *m_searchResult = nullptr;
};

class SettingsMatch :public QThread {
    Q_OBJECT
public:
    enum HandleType {
        Add,
        Delete
    };
    Q_DECLARE_FLAGS(HandleTypes, HandleType)

    static SettingsMatch *getInstance();
    void startSearch(QString &keyword, size_t uniqueSymbol, DataQueue<SearchPluginIface::ResultInfo> *searchResult);
public Q_SLOTS:
    void addItem(QVariantMap replyMap);
    void removeItem(QVariantMap replyMap);
protected:
    void run() override;
private:
    explicit SettingsMatch();
    ~SettingsMatch() = default;

    //parsing dbus of ukcc
    void initData();
    void parsingArgsOfDbus(QVariantMap replyMap, HandleTypes type);
    void handleData(const QVariantMap &itemInfo, HandleTypes type);
    void add2DataMap(const QString &key, const QString &enName, const QString &localName);

    //Parsing xml
    void initDataOfXml();
    bool matchCommonEnvironment(QDomNode childNode);
    void matchNodes(QDomNode node);

    //search
    bool matchDataMap(QString &key, QString &keyword);
    void createResultInfo(SearchPluginIface::ResultInfo &resultInfo, const QStringList &itemInfo, const QString &path);
    bool enqueueResultInfo(SearchPluginIface::ResultInfo &resultInfo, size_t uniqueSymbol, DataQueue<SearchPluginIface::ResultInfo> *searchResult);
    QMap<QString, QMap<QString, QStringList>> m_searchMap;
    QMap<QString, QStringList> m_dataMap;
    QDBusInterface *m_interface = nullptr;
};
}
#endif // SETTINGSSEARCHPLUGIN_H
